/*
 *  snd.h
 *
 *  Created by Kevin Wojniak on 3/3/11.
 *  Copyright 2011 Kevin Wojniak. All rights reserved.
 *
 */

#ifndef SND_H
#define SND_H

#include <stddef.h>

typedef struct snd_t snd_t;

snd_t* snd_new(const char *data, size_t size);

int snd_play(snd_t *snd);

#endif
