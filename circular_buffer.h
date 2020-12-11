#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "def.h"


struct circular_buf_t {
	uint8_t * buffer;
	size_t head;
	size_t tail;
	size_t len; //of the buffer
	size_t error;
};
typedef struct circular_buf_t circular_buf_t;

circular_buf_t* circular_buf_init(uint8_t* buffer, size_t size);
void circular_buf_free(circular_buf_t* cbuf);
bool circular_buf_empty(circular_buf_t* cbuf);
size_t circular_buf_size(circular_buf_t* cbuf);
void circular_buf_put(circular_buf_t* cbuf, uint8_t data);
int circular_buf_get(circular_buf_t* cbuf, uint8_t* data);

#endif