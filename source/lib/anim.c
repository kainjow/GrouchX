/*
 *  anim.c
 *
 *  Created by Kevin Wojniak on 3/3/11.
 *  Copyright 2011 Kevin Wojniak. All rights reserved.
 *
 */

#include "anim.h"
#include "anim_header.h"
#include <arpa/inet.h>
#include <ApplicationServices/ApplicationServices.h>
#include "color_palettes.h"

CFArrayRef anim_create_images(const char *data, size_t size)
{
	if (size < sizeof(ANIM_HEADER)) {
		return NULL;
	}
	
	ANIM_HEADER *header = (ANIM_HEADER *)data;
	header->sig = ntohl(header->sig);
	if (header->sig != ANIM_HEADER_SIG) {
		return NULL;
	}
	
	header->record_size = ntohl(header->record_size);
	if ((size_t)header->record_size != size) {
		return NULL;
	}
	
	header->num_images = ntohl(header->num_images);
	header->num_masks = ntohl(header->num_masks);
	if (header->num_images != header->num_masks) {
		return NULL;
	}
	
	header->image_offset = ntohl(header->image_offset);
	header->mask_offset = ntohl(header->mask_offset);
	if (header->image_offset >= size || header->mask_offset >= size) {
		return NULL;
	}

	header->bytes_per_image = ntohl(header->bytes_per_image);
	header->bytes_per_mask = ntohl(header->bytes_per_mask);
	header->rounded_width = ntohs(header->rounded_width);
	header->height = ntohs(header->height);
	
	if (((header->image_offset + (header->num_images * header->bytes_per_image)) > size) || 
		((header->mask_offset + (header->num_masks * header->bytes_per_mask)) > size)) {
		return NULL;
	}

	CFMutableArrayRef array = CFArrayCreateMutable(kCFAllocatorDefault, header->num_images, &kCFTypeArrayCallBacks);

	const char *mask_data = data + header->mask_offset;
	const char *img_data_start = data + header->image_offset;
	const char *img_data_end = img_data_start + (header->bytes_per_image * header->num_images);
	const char *img_data = img_data_start;
	uint8_t index;

	CGColorSpaceRef image_colorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
	CGContextRef bitmap_ctx = CGBitmapContextCreate(NULL, header->rounded_width, header->height, 8, header->rounded_width*4, image_colorspace, kCGImageAlphaPremultipliedLast);
	uint8_t *raw_data_start = CGBitmapContextGetData(bitmap_ctx);
	uint8_t *raw_data_end = raw_data_start + (header->rounded_width * header->height * 4);
	
	while (img_data < img_data_end) {
		CFBitVectorRef bit_vector = CFBitVectorCreate(kCFAllocatorDefault, (const UInt8 *)mask_data, header->bytes_per_mask * 8);
		CFIndex pixel_index = 0;

		uint8_t *raw_data = raw_data_start;
		while (raw_data < raw_data_end) {
			index = *img_data & 0xFF;
			*raw_data++ = palette_standard[index][0];
			*raw_data++ = palette_standard[index][1];
			*raw_data++ = palette_standard[index][2];
			*raw_data++ = CFBitVectorGetBitAtIndex(bit_vector, pixel_index) ? 0xFF : 0x00;
			img_data++;
			pixel_index++;
		}
		CGImageRef img = CGBitmapContextCreateImage(bitmap_ctx);
		if (img) {
			CFArrayAppendValue(array, img);
			CGImageRelease(img);
		}
		CFRelease(bit_vector);
		
		mask_data += header->bytes_per_mask;
	}

	if (bitmap_ctx) {
		CFRelease(bitmap_ctx);
	}
	CGColorSpaceRelease(image_colorspace);
	
	return array;
}
