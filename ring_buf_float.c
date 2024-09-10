#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include <ring_buf_float/ring_buf_float.h>

// The definition of our ring buffer structure is hidden from the user
struct ring_buf_t
{
	float_t* buffer;
	size_t head;
	size_t tail;
	size_t max; // of the buffer
	bool full;
};

#pragma mark - Ring buffers allocation -
//=====================================================================================
//
// Ring buffers allocation
// Note: the actual rbuf is the variable passed to the function ring_buf_static_init()!
//
//=====================================================================================
ring_buf_t rbuf_array[MAX_RBUFS];
uint16_t rbufs_number;

#pragma mark - Private Functions -

static inline size_t advance_headtail_value(size_t value, size_t max)
{
    if(++value == max)
    {
        value = 0;
    }

    return value;
}

static void advance_head_pointer(rbuf_handle_t me)
{
	assert(me);

	if(ring_buf_full(me))
	{
		me->tail = advance_headtail_value(me->tail, me->max);
	}

	me->head = advance_headtail_value(me->head, me->max);
	me->full = (me->head == me->tail);
}

#pragma mark - APIs -
rbuf_handle_t ring_buf_static_init(float_t* buffer, size_t size)
{
    assert(buffer && size);

    // the rbuf structure allocation is static:
    // once MAX_RBUFS structures are used, no more are available and the init fails
    if (MAX_RBUFS > rbufs_number)
    {
        // a new rbuf is available
        rbuf_handle_t rbuf = &rbuf_array[rbufs_number];
        assert(rbuf);
        rbufs_number++;

        rbuf->buffer = buffer;
        rbuf->max = size;
        ring_buf_reset(rbuf);

        assert(ring_buf_empty(rbuf));

        return rbuf;
    }
    else
    {
        // a new rbuf is not available
        return NULL;
    }
}

rbuf_handle_t ring_buf_dynamic_init(float_t* buffer, size_t size)
{
    assert(buffer && size);

    rbuf_handle_t rbuf = malloc(sizeof(ring_buf_t));
    assert(rbuf);

    rbuf->buffer = buffer;
    rbuf->max = size;
    ring_buf_reset(rbuf);

    assert(ring_buf_empty(rbuf));

    return rbuf;
}

void ring_buf_free(rbuf_handle_t me)
{
	assert(me);
	free(me);
}

void ring_buf_reset(rbuf_handle_t me)
{
	assert(me);

	me->head = 0;
	me->tail = 0;
	me->full = false;
}

size_t ring_buf_size(rbuf_handle_t me)
{
	assert(me);

	size_t size = me->max;

	if(!ring_buf_full(me))
	{
		if(me->head >= me->tail)
		{
			size = (me->head - me->tail);
		}
		else
		{
			size = (me->max + me->head - me->tail);
		}
	}

	return size;
}

size_t ring_buf_capacity(rbuf_handle_t me)
{
	assert(me);

	return me->max;
}

void ring_buf_put(rbuf_handle_t me, float_t data)
{
	assert(me && me->buffer);

	me->buffer[me->head] = data;

	advance_head_pointer(me);
}

int ring_buf_try_put(rbuf_handle_t me, float_t data)
{
	int r = -1;

	assert(me && me->buffer);

	if(!ring_buf_full(me))
	{
		me->buffer[me->head] = data;
		advance_head_pointer(me);
		r = 0;
	}

	return r;
}

int ring_buf_get(rbuf_handle_t me, float_t* data)
{
	assert(me && data && me->buffer);

	int r = -1;

	if(!ring_buf_empty(me))
	{
		*data = me->buffer[me->tail];
		me->tail = advance_headtail_value(me->tail, me->max);
		me->full = false;
		r = 0;
	}

	return r;
}

bool ring_buf_empty(rbuf_handle_t me)
{
	assert(me);

	return (!ring_buf_full(me) && (me->head == me->tail));
}

bool ring_buf_full(rbuf_handle_t me)
{
	assert(me);

	return me->full;
}

int ring_buf_peek(rbuf_handle_t me, float_t* data, unsigned int look_ahead_counter)
{
	int r = -1;
	size_t pos;

	assert(me && data && me->buffer);

	// We can't look beyond the current buffer size
	if(ring_buf_empty(me) || look_ahead_counter > ring_buf_size(me))
	{
		return r;
	}

	pos = me->tail;
	for(unsigned int i = 0; i < look_ahead_counter; i++)
	{
		data[i] = me->buffer[pos];
		pos = advance_headtail_value(pos, me->max);
	}

	return 0;
}
