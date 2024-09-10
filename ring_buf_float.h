#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <stdbool.h>
#include <stdint.h>
#include "math.h"

#define MAX_RBUFS               2


/// Opaque ring buffer structure
typedef struct ring_buf_t ring_buf_t;

/// Handle type, the way users interact with the API
typedef ring_buf_t* rbuf_handle_t;

/// Pass in a storage buffer and size, returns a ring buffer handle
/// Requires: buffer is not NULL, size > 0 (size > 1 for the threadsafe
//  version, because it holds size - 1 elements)
/// Ensures: me has been created and is returned in an empty state
/// Memory allocation is static, rbuf structures are taken from an existing pool
rbuf_handle_t ring_buf_static_init(float_t* buffer, size_t size);

/// Pass in a storage buffer and size, returns a ring buffer handle
/// Requires: buffer is not NULL, size > 0 (size > 1 for the threadsafe
//  version, because it holds size - 1 elements)
/// Ensures: me has been created and is returned in an empty state
/// Memory allocation is dynamic, rbuf structures come from malloc
rbuf_handle_t ring_buf_dynamic_init(float_t* buffer, size_t size);

/// Free a ring buffer structure
/// Requires: me is valid and created by ring_buf_init
/// Does not free data buffer; owner is responsible for that
void ring_buf_free(rbuf_handle_t me);

/// Reset the ring buffer to empty, head == tail. Data not cleared
/// Requires: me is valid and created by ring_buf_init
void ring_buf_reset(rbuf_handle_t me);

/// Put that continues to add data if the buffer is full
/// Old data is overwritten
/// Note: if you are using the threadsafe version, this API cannot be used, because
/// it modifies the tail pointer in some cases. Use ring_buf_try_put instead.
/// Requires: me is valid and created by ring_buf_init
void ring_buf_put(rbuf_handle_t me, float_t data);

/// Put that rejects new data if the buffer is full
/// Note: if you are using the threadsafe version, *this* is the put you should use
/// Requires: me is valid and created by ring_buf_static_init
/// Returns 0 on success, -1 if buffer is full
int ring_buf_try_put(rbuf_handle_t me, float_t data);

/// Retrieve a value from the buffer
/// Requires: me is valid and created by ring_buf_init
/// Returns 0 on success, -1 if the buffer is empty
int ring_buf_get(rbuf_handle_t me, float_t* data);

/// CHecks if the buffer is empty
/// Requires: me is valid and created by ring_buf_init
/// Returns true if the buffer is empty
bool ring_buf_empty(rbuf_handle_t me);

/// Checks if the buffer is full
/// Requires: me is valid and created by ring_buf_static_init
/// Returns true if the buffer is full
bool ring_buf_full(rbuf_handle_t me);

/// Check the capacity of the buffer
/// Requires: me is valid and created by ring_buf_init
/// Returns the maximum capacity of the buffer
size_t ring_buf_capacity(rbuf_handle_t me);

/// Check the number of elements stored in the buffer
/// Requires: me is valid and created by ring_buf_static_init
/// Returns the current number of elements in the buffer
size_t ring_buf_size(rbuf_handle_t me);

/// Look ahead at values stored in the ring buffer without removing the data
/// Requires:
///		- me is valid and created by ring_buf_init
///		- look_ahead_counter is less than or equal to the value returned by ring_buf_size()
/// Returns 0 if successful, -1 if data is not available
int ring_buf_peek(rbuf_handle_t me, float_t* data, unsigned int look_ahead_counter);

#endif // RING_BUFFER_H_
