/*
 *  snd.c
 *
 *  Created by Kevin Wojniak on 3/3/11.
 *  Copyright 2011 Kevin Wojniak. All rights reserved.
 *
 */

#include "snd.h"
#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <AudioToolbox/AudioToolbox.h>

struct snd_mod_ref_t {
	uint16_t mod_number;
	int32_t mod_init;
} __attribute__((__packed__));
typedef struct snd_mod_ref_t snd_mod_ref_t;

struct snd_command_t {
	uint16_t cmd;
	int16_t param1;
	int32_t param2;
} __attribute__((__packed__));
typedef struct snd_command_t snd_command_t;

struct snd_list_resource_t {
	int16_t format;
	int16_t num_modifiers;
	snd_mod_ref_t modifier_part[1];
	int16_t num_commands;
	snd_command_t command_part[1];
} __attribute__((__packed__));
typedef struct snd_list_resource_t snd_list_resource_t;

struct snd_header_t {
	uint32_t sample_ptr;
	uint32_t length;
	uint32_t sample_rate;
	uint32_t loop_start;
	uint32_t loop_end;
	uint8_t encode;
	uint8_t base_frequency;
} __attribute__((__packed__));
typedef struct snd_header_t snd_header_t;


struct snd_t {
	AudioQueueRef queue;
	AudioQueueBufferRef buffer;
};


void snd_audio_queue_output_callback(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
{
	OSStatus status = AudioQueueStop(inAQ, true);
	if (status != noErr) {
		printf("AudioQueueStop: %ld\n", (long)status);
	}
}


snd_t* snd_new(const char *data, size_t size)
{
	if (size < sizeof(snd_list_resource_t) || !data) {
		return NULL;
	}
	
	snd_list_resource_t *handle = (snd_list_resource_t *)data;
	handle->format = ntohs(handle->format);
	if (handle->format != 1) {
		// Format is 1 for general format, 2 for newer HyperCard sounds
		return NULL;
	}
	
	handle->num_modifiers = ntohs(handle->num_modifiers);
	handle->num_commands = ntohs(handle->num_commands);
	if (handle->num_modifiers < 1) {
		return NULL;
	}
	
	
	uint32_t header_offset = sizeof(snd_list_resource_t) + ((handle->num_modifiers - 1) * sizeof(snd_mod_ref_t)) + ((handle->num_commands - 1) * sizeof(snd_command_t));
	snd_header_t *header = (snd_header_t *)(data + header_offset);
	header->length = ntohl(header->length);
	if (header->length != (size - (header_offset + sizeof(snd_header_t)))) {
		return NULL;
	}
	
	header->sample_rate = ntohl(header->sample_rate);
	
	if (header->encode != 0x00) {
		// 0x00 for standard, 0xFF for extended, 0xFE for compressed
		return NULL;
	}
	
	//printf("DURATION: %fs\n", (float)header->length / (float)(header->sample_rate >> 16));
	
	snd_t *snd = calloc(1, sizeof(snd_t));
	
	AudioStreamBasicDescription stream = {0};
	stream.mSampleRate = header->sample_rate >> 16;
	stream.mFormatID = kAudioFormatLinearPCM;
	stream.mFormatFlags = 0;
	stream.mBytesPerPacket = 1;
	stream.mFramesPerPacket = 1;
	stream.mBytesPerFrame = 1;
	stream.mChannelsPerFrame = 1;
	stream.mBitsPerChannel = 8;
	OSStatus status = AudioQueueNewOutput(&stream, snd_audio_queue_output_callback, NULL, NULL, NULL, 0, &snd->queue);
	if (status != noErr) {
		printf("AudioQueueNewOutput: %ld\n", (long)status);
		free(snd);
		return NULL;
	}
	
	status = AudioQueueAllocateBuffer(snd->queue, header->length, &snd->buffer);
	if (status != noErr) {
		printf("AudioQueueAllocateBuffer: %ld\n", (long)status);
		free(snd);
		AudioQueueDispose(snd->queue, true);
		return NULL;
	}
	
	snd->buffer->mAudioDataByteSize = header->length;
	memcpy(snd->buffer->mAudioData, data + header_offset + sizeof(snd_header_t), header->length);

	return snd;
}


int snd_play(snd_t *snd)
{
	if (!snd || !snd->queue || !snd->buffer) {
		return -1;
	}
	
	OSStatus status = AudioQueueEnqueueBuffer(snd->queue, snd->buffer, 0, NULL);
	if (status != noErr) {
		printf("AudioQueueEnqueueBuffer: %ld\n", (long)status);
		return -1;
	}
	
	status = AudioQueueStart(snd->queue, NULL);
	if (status != noErr) {
		printf("AudioQueueStart: %ld\n", (long)status);
		return -1;
	}
	
	return 0;
}

#if 0
int snd_convert(snd_t *snd)
{
	NSString *pp = @"/Volumes/Macintosh HD/Users/kainjow/Desktop/ext.aif";
	ExtAudioFileRef audioFile;
	OSStatus status;
	AudioStreamBasicDescription stream = {0};
	stream.mSampleRate = 22000;
	stream.mFormatID = kAudioFormatLinearPCM;
	stream.mFormatFlags = 0;
	stream.mBytesPerPacket = 1;
	stream.mFramesPerPacket = 1;
	stream.mBytesPerFrame = 1;
	stream.mChannelsPerFrame = 1;
	stream.mBitsPerChannel = 8;
	status = ExtAudioFileCreateWithURL((CFURLRef)[NSURL fileURLWithPath:pp], kAudioFileWAVEType/*kAudioFileAIFFType*/, &stream, NULL, kAudioFileFlags_EraseFile, &audioFile);
	if (status == noErr) {
		UInt32 numFrames = (sndSize - 50);
		AudioBufferList buffer;
		buffer.mNumberBuffers = 1;
		buffer.mBuffers[0].mNumberChannels = 1;
		buffer.mBuffers[0].mDataByteSize = numFrames;
		buffer.mBuffers[0].mData = (char *)snd + 50;
#if 0
		UInt8 *t = (UInt8 *)buffer.mBuffers[0].mData;
		for (UInt32 i=0; i<numFrames; i++) {
			*t ^= 0x80;
			t++;
		}
#endif
		status = ExtAudioFileWrite(audioFile, numFrames, &buffer);
		if (status == noErr) {
			printf("SUCCESS\n");
		} else {
			printf("FAILED: %ld\n", status);
		}
		ExtAudioFileDispose(audioFile);
	} else {
		printf("Couldn't create ext audio file: %ld\n", (long)status);
	}
}
#endif
