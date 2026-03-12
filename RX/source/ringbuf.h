#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdint.h>

/*
 * Lock-free ring buffer for single-producer (ISR) / single-consumer (main).
 *
 * Size must be a power of two so we can use bitmask instead of modulo.
 * volatile head/tail ensure correct ordering between ISR and main loop.
 */

#define RING_SIZE  128u               /* must be power of 2 */
#define RING_MASK  (RING_SIZE - 1u)

typedef struct {
    volatile uint8_t  buf[RING_SIZE];
    volatile uint32_t head;           /* ISR writes here (next write index) */
    volatile uint32_t tail;           /* main reads here (next read index)  */
} ringbuf_t;

static inline void ring_init(ringbuf_t *r)
{
    r->head = 0;
    r->tail = 0;
}

/*
 * Push one byte (called from ISR).
 * Returns 1 on success, 0 if buffer is full (byte is dropped).
 */
static inline int ring_push(ringbuf_t *r, uint8_t c)
{
    uint32_t next = (r->head + 1u) & RING_MASK;
    if (next == r->tail) {
        return 0;   /* full */
    }
    r->buf[r->head] = c;
    r->head = next;
    return 1;
}

/*
 * Pop one byte (called from main loop).
 * Returns 1 if a byte was read, 0 if buffer is empty.
 */
static inline int ring_pop(ringbuf_t *r, uint8_t *out)
{
    if (r->tail == r->head) {
        return 0;   /* empty */
    }
    *out = r->buf[r->tail];
    r->tail = (r->tail + 1u) & RING_MASK;
    return 1;
}

/* Number of bytes currently in the buffer */
static inline uint32_t ring_count(const ringbuf_t *r)
{
    return (r->head - r->tail) & RING_MASK;
}

#endif /* RINGBUF_H */
