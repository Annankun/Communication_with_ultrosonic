#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdint.h>

#define RING_SIZE  128u               /* must be power of 2 */
#define RING_MASK  (RING_SIZE - 1u)

typedef struct {
    volatile uint8_t  buf[RING_SIZE];
    volatile uint32_t head;           /* ISR writes here (next write index) */
    volatile uint32_t tail;           /* main reads here (next read index)  */
} ringbuf_t;

void ring_init(ringbuf_t *r);
int ring_push(ringbuf_t *r, uint8_t c);
int ring_pop(ringbuf_t *r, uint8_t *out);
uint32_t ring_count(const ringbuf_t *r);

#endif /* RINGBUF_H */
