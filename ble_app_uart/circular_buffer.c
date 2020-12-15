#include "circular_buffer.h"

void circular_buf_reset(circular_buf_t* cbuf)
{
    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->error = 0;
}

circular_buf_t* circular_buf_init(uint8_t* buffer, size_t size)
{
	circular_buf_t* cbuf = malloc(sizeof(circular_buf_t));

	cbuf->buffer = buffer;
	cbuf->len = size;
	circular_buf_reset(cbuf);

	return cbuf;
}

void circular_buf_free(circular_buf_t* cbuf)
{
	free(cbuf);
}

bool circular_buf_empty(circular_buf_t* cbuf)
{
    return (cbuf->head == cbuf->tail);
}

size_t circular_buf_size(circular_buf_t* cbuf)
{
	size_t size = cbuf->len;

	if(cbuf->head >= cbuf->tail)
	{
		size = (cbuf->head - cbuf->tail);
	}
	else
	{
		size = (cbuf->len + cbuf->head - cbuf->tail);
	}

	return size;
}

void circular_buf_put(circular_buf_t* cbuf, uint8_t data)
{
    cbuf->buffer[cbuf->head] = data;
    cbuf->head = (cbuf->head + 1) % cbuf->len;
    cbuf->error += (cbuf->head == cbuf->tail);
}

int circular_buf_get(circular_buf_t* cbuf, uint8_t* data)
{
    if (!circular_buf_empty(cbuf))
    {
        *data = cbuf->buffer[cbuf->tail];
        cbuf->tail = (cbuf->tail + 1) % cbuf->len;
        return 1;
    }
    return 0;
}