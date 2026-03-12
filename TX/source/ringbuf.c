#include "ringbuf.h"

void ring_init(ringbuf_t *r)
{
    r->head = 0;
    r->tail = 0;
}

int ring_push(ringbuf_t *r, uint8_t c)
{
    uint32_t next = (r->head + 1u) & RING_MASK;
    if (next == r->tail) {
        return 0;   /* full */
    }
    r->buf[r->head] = c;
    r->head = next;
    return 1;
}

int ring_pop(ringbuf_t *r, uint8_t *out)
{
    if (r->tail == r->head) {
        return 0;   /* empty */
    }
    *out = r->buf[r->tail];
    r->tail = (r->tail + 1u) & RING_MASK;
    return 1;
}

uint32_t ring_count(const ringbuf_t *r)
{
    return (r->head - r->tail) & RING_MASK;
}
