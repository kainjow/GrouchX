/*
 *  rsrc.h
 *
 *  Created by Kevin Wojniak on 3/1/11.
 *  This is a C port of Andrew Willmott's C++ Resource class in macfork.
 *
 */

#ifndef RSRC_H
#define RSRC_H

#include <stdint.h>
	
typedef struct rsrc_t rsrc_t;


enum rsrc_fork_t {
	rsrc_fork_data		= 0,
	rsrc_fork_resource	= 1,
};
typedef enum rsrc_fork_t rsrc_fork_t;


rsrc_t* rsrc_open(const char *path, rsrc_fork_t fork);
void rsrc_close(rsrc_t *rsrc);
	
void rsrc_print_all_resources(rsrc_t *rsrc);
void rsrc_print_resources(rsrc_t *rsrc, int16_t index);
	
uint16_t rsrc_get_num_types(rsrc_t *rsrc);
uint16_t rsrc_get_num_resources(rsrc_t *rsrc, uint16_t index);
	
char *rsrc_get_resource(rsrc_t *rsrc, uint32_t type_id, int16_t res_id, uint32_t *size);
const char* rsrc_get_resource_name(rsrc_t *rsrc, uint32_t type_id, int16_t res_id);
	
int16_t rsrc_get_type_number(rsrc_t *rsrc, uint32_t type_id);
int16_t rsrc_get_resource_number(rsrc_t *rsrc, uint16_t index, int16_t res_id);

#endif
