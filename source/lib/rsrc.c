/*
 *  rsrc.c
 *
 *  Created by Kevin Wojniak on 3/1/11.
 *  This is a C port of Andrew Willmott's C++ Resource class in macfork.
 *
 */

#include "rsrc.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>


struct rsrc_header_t {
	uint32_t data_offset;
	uint32_t map_offset;
	uint32_t data_length;
	uint32_t map_length;
};
typedef struct rsrc_header_t rsrc_header_t;


struct rsrc_map_t {
	uint8_t reserved_header[16];
	uint8_t reserved_next_map[4];
	uint8_t reserved_file_ref[2];
	int16_t attributes;
	int16_t	type_offset;
	int16_t	name_offset;
	int16_t	num_types;
};
typedef struct rsrc_map_t rsrc_map_t;


struct rsrc_type_t {
	uint32_t id;
	int16_t items;
	int16_t offset;
};
typedef struct rsrc_type_t rsrc_type_t;


struct rsrc_resource_t {
	int16_t id;
	int16_t name_offset;
	int8_t attr;
	uint32_t data_offset;
	char *name;
};
typedef struct rsrc_resource_t rsrc_resource_t;


struct rsrc_t {
	FILE *file;
	rsrc_header_t header;
	rsrc_map_t map;
	rsrc_type_t *types;
	rsrc_resource_t **lists;
};


static int rsrc_read_uint16(rsrc_t *rsrc, uint16_t *value)
{
	uint16_t buf;
	if (fread(&buf, 1, sizeof(buf), rsrc->file) != sizeof(buf)) {
		return -1;
	}
	if (value) {
		*value = ntohs(buf);
	}
	return 0;
}


static int rsrc_read_uint32(rsrc_t *rsrc, uint32_t *value)
{
	uint32_t buf;
	if (fread(&buf, 1, sizeof(buf), rsrc->file) != sizeof(buf)) {
		return -1;
	}
	if (value) {
		*value = ntohl(buf);
	}
	return 0;
}


char* rsrc_read_resource(rsrc_t *rsrc, uint32_t *size)
{
	uint32_t len;
	char *result;
	
	if (rsrc_read_uint32(rsrc, &len) != 0) {
		return NULL;
	}
	
	result = calloc(len, sizeof(char));
	if (fread(result, 1, len, rsrc->file) != len) {
		free(result);
		return NULL;
	}
	if (size) {
		*size = len;
	}
	
	return(result);
}


char* rsrc_read_resource_(rsrc_t *rsrc, uint16_t i, uint16_t j, uint32_t *size)
{
	if (fseek(rsrc->file, rsrc->header.data_offset + rsrc->lists[i][j].data_offset, SEEK_SET) != 0) {
		return NULL;
	}
	return rsrc_read_resource(rsrc, size);
}


uint16_t rsrc_get_num_types(rsrc_t *rsrc)
{
	return rsrc->map.num_types;
}


uint16_t rsrc_get_num_resources(rsrc_t *rsrc, uint16_t index)
{
	return rsrc->types[index].items;
}


int16_t rsrc_get_type_number(rsrc_t *rsrc, uint32_t type_id)
{
	int16_t i;
	for (i=0; i<rsrc->map.num_types; i++) {
		if (rsrc->types[i].id == type_id) {
			return i;
		}
	}
	return -1;
}


int16_t rsrc_get_resource_number(rsrc_t *rsrc, uint16_t index, int16_t res_id)
{
	int16_t j;
	for (j=0; j<rsrc->types[index].items; j++) {
		if (rsrc->lists[index][j].id == res_id) {
			return j;
		}
	}
	return -1;
}


char *rsrc_get_resource(rsrc_t *rsrc, uint32_t type_id, int16_t res_id, uint32_t *size)
{
	int16_t i, j;
	if ((i = rsrc_get_type_number(rsrc, type_id)) >= 0 
		&& (j = rsrc_get_resource_number(rsrc, i, res_id)) >= 0) {
		return rsrc_read_resource_(rsrc, i, j, size);
	}
	return NULL;
}


const char* rsrc_get_resource_name(rsrc_t *rsrc, uint32_t type_id, int16_t res_id)
{
	int16_t i, j;
	if ((i = rsrc_get_type_number(rsrc, type_id)) >= 0 
		&& (j = rsrc_get_resource_number(rsrc, i, res_id)) >= 0) {
		return rsrc->lists[i][j].name;
	}
	return NULL;
}


static int rsrc_read_map(rsrc_t *rsrc)
{
	int i, j, len;
	
	if (fseek(rsrc->file, rsrc->header.map_offset + offsetof(rsrc_map_t, attributes)/*22*/, SEEK_SET) != 0) {
		return -1;
	}
	
	if ((rsrc_read_uint16(rsrc, (uint16_t *)&rsrc->map.attributes) != 0) ||
		(rsrc_read_uint16(rsrc, (uint16_t *)&rsrc->map.type_offset) != 0) ||
		(rsrc_read_uint16(rsrc, (uint16_t *)&rsrc->map.name_offset) != 0) ||
		(rsrc_read_uint16(rsrc, (uint16_t *)&rsrc->map.num_types) != 0)) {
		return -1;
	}
	rsrc->map.num_types++;
	
	if (fseek(rsrc->file, rsrc->header.map_offset + rsrc->map.type_offset + 2, SEEK_SET) != 0) {
		return -1;
	}
	
	rsrc->types = calloc(rsrc->map.num_types, sizeof(rsrc_type_t));
	if (rsrc->types == NULL) {
		return -1;
	}
	
	for (i = 0; i < rsrc->map.num_types; i++)
	{
		if (rsrc_read_uint32(rsrc, &rsrc->types[i].id) != 0) {
			return -1;
		}
		if ((rsrc_read_uint16(rsrc, (uint16_t *)&rsrc->types[i].items) != 0) ||
			(rsrc_read_uint16(rsrc, (uint16_t *)&rsrc->types[i].offset) != 0)) {
			return -1;
		}
		rsrc->types[i].items++;
	}
	
	rsrc->lists = calloc(rsrc->map.num_types, sizeof(rsrc_resource_t*));
	if (rsrc->lists == NULL) {
		return -1;
	}
	
	for (i = 0; i < rsrc->map.num_types; i++)
	{
		rsrc->lists[i] = calloc(rsrc->types[i].items, sizeof(rsrc_resource_t));
		if (!rsrc->lists) {
			return -1;
		}
		if (fseek(rsrc->file, rsrc->types[i].offset + rsrc->header.map_offset + rsrc->map.type_offset, SEEK_SET) != 0) {
			return -1;
		}
		
		for (j = 0; j < rsrc->types[i].items; j++)
		{
			rsrc_resource_t *resPtr = rsrc->lists[i] + j;
			
			if ((rsrc_read_uint16(rsrc, (uint16_t *)&resPtr->id) != 0) || 
				(rsrc_read_uint16(rsrc, (uint16_t *)&resPtr->name_offset) != 0) || 
				(rsrc_read_uint32(rsrc, &resPtr->data_offset) != 0) ||
				(rsrc_read_uint32(rsrc, NULL) != 0)) {
				return -1;
			}
			resPtr->name = 0;
			
			resPtr->attr = resPtr->data_offset >> 24;
			resPtr->data_offset &= 0xFFFFFF;
		}
		
		for (j = 0; j < rsrc->types[i].items; j++)
		{
			if (rsrc->lists[i][j].name_offset != -1)
			{
				if (fseek(rsrc->file, rsrc->lists[i][j].name_offset + rsrc->header.map_offset + rsrc->map.name_offset, SEEK_SET) != 0) {
					return -1;
				}
				len = fgetc(rsrc->file);
				rsrc->lists[i][j].name = calloc(len + 1, sizeof(char));
				rsrc->lists[i][j].name[len] = 0;
				if (fread(rsrc->lists[i][j].name, 1, len, rsrc->file) != len) {
					return -1;
				}
			}
		}
	}
	
	return 0;
}


rsrc_t* rsrc_open(const char *path, rsrc_fork_t fork)
{
	if (path == NULL || (fork != rsrc_fork_data && fork != rsrc_fork_resource)) {
		return NULL;
	}
	
	rsrc_t *rsrc = calloc(1, sizeof(rsrc_t));
	if (rsrc == NULL) {
		return NULL;
	}
	
	rsrc->file = fopen(path, "r");
	if (rsrc->file == NULL) {
		free(rsrc);
		return NULL;
	}
	
	if ((rsrc_read_uint32(rsrc, &rsrc->header.data_offset) != 0) ||
		(rsrc_read_uint32(rsrc, &rsrc->header.map_offset) != 0) ||
		(rsrc_read_uint32(rsrc, &rsrc->header.data_length) != 0) ||
		(rsrc_read_uint32(rsrc, &rsrc->header.map_length) != 0) ||
		(rsrc_read_map(rsrc) != 0)) {
		fclose(rsrc->file);
		free(rsrc);
		return NULL;
	}
	
	return rsrc;
}


void rsrc_close(rsrc_t *rsrc)
{
	if (rsrc) {
		if (rsrc->types) {
			free(rsrc->types);
		}
		if (rsrc->lists) {
			for (int16_t i=0; i<rsrc->map.num_types; i++) {
				if (rsrc->lists[i]) {
					if (rsrc->lists[i]->name) {
						free(rsrc->lists[i]->name);
					}
					free(rsrc->lists[i]);
				}
			}
			free(rsrc->lists);
		}
		if (rsrc->file) {
			fclose(rsrc->file);
		}
		free(rsrc);
	}
}


void rsrc_print_all_resources(rsrc_t *rsrc)
{
	int16_t i;
	if (rsrc == NULL) {
		return;
	}
	for (i=0; i<rsrc->map.num_types; i++) {
		printf("Type '%c%c%c%c'\n",
			   (rsrc->types[i].id >> 24) & 0xFF,
			   (rsrc->types[i].id >> 16) & 0xFF,
			   (rsrc->types[i].id >> 8) & 0xFF,
			   (rsrc->types[i].id & 0xFF));
		rsrc_print_resources(rsrc, i);
	}
}


void rsrc_print_resources(rsrc_t *rsrc, int16_t index)
{
	int16_t j;
	uint32_t len;
	if (rsrc == NULL || (index < 0) || (index >= rsrc->map.num_types)) {
		return;
	}
	for (j=0; j<rsrc->types[index].items; j++) {
		if ((fseek(rsrc->file, rsrc->header.data_offset + rsrc->lists[index][j].data_offset, SEEK_SET) != 0) ||
			(rsrc_read_uint32(rsrc, &len) != 0)) {
			continue;
		}
		printf("    %6d   %8u", rsrc->lists[index][j].id, len);
		if (rsrc->lists[index][j].name) {
			printf("      \"%s\"\n", rsrc->lists[index][j].name);
		} else {
			printf("\n");
		}
	}
}
