#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <string.h>


#define FRAME_SOF0          0xAAu
#define FRAME_SOF1          0x55u
#define FRAME_HEADER_SIZE   5u      /* SOF(2) + LEN(1) + TYPE(1) + SEQ(1) */
#define FRAME_CRC_SIZE      2u
#define FRAME_MAX_PAYLOAD   255u
#define FRAME_TYPE_SENSOR   0x01u


static inline uint16_t crc16_update(uint16_t crc, uint8_t byte)
{
    crc ^= (uint16_t)byte << 8;
    for (uint8_t i = 0; i < 8; i++) {
        if (crc & 0x8000u)
            crc = (crc << 1) ^ 0x1021u;
        else
            crc <<= 1;
    }
    return crc;
}

static inline uint16_t crc16_calc(const uint8_t *data, uint8_t len)
{
    uint16_t crc = 0xFFFFu;
    for (uint8_t i = 0; i < len; i++)
        crc = crc16_update(crc, data[i]);
    return crc;
}

static inline uint8_t frame_pack(uint8_t *buf, uint8_t type, uint8_t seq,
                                  const uint8_t *payload, uint8_t payload_len)
{
    uint8_t pos = 0;

    buf[pos++] = FRAME_SOF0;
    buf[pos++] = FRAME_SOF1;
    buf[pos++] = payload_len;
    buf[pos++] = type;
    buf[pos++] = seq;

    for (uint8_t i = 0; i < payload_len; i++)
        buf[pos++] = payload[i];

    /* CRC over LEN + TYPE + SEQ + PAYLOAD (skip SOF) */
    uint16_t crc = crc16_calc(&buf[2], (uint8_t)(pos - 2));
    buf[pos++] = (uint8_t)(crc >> 8);
    buf[pos++] = (uint8_t)(crc & 0xFF);

    return pos;
}


typedef enum {
    PARSE_SOF0,
    PARSE_SOF1,
    PARSE_LEN,
    PARSE_TYPE,
    PARSE_SEQ,
    PARSE_PAYLOAD,
    PARSE_CRC_HI,
    PARSE_CRC_LO
} parse_state_t;

typedef struct {
    parse_state_t state;
    uint8_t  len;
    uint8_t  type;
    uint8_t  seq;
    uint8_t  payload[FRAME_MAX_PAYLOAD];
    uint8_t  payload_idx;
    uint8_t  crc_hi;
} parser_t;

static inline void parser_init(parser_t *p)
{
    p->state = PARSE_SOF0;
    p->payload_idx = 0;
}

#define PARSE_INCOMPLETE  0   /* need more bytes */
#define PARSE_OK          1   /* valid frame received */
#define PARSE_BAD_CRC    -1   /* CRC mismatch; frame discarded */

/* Feed one byte; returns PARSE_OK, PARSE_BAD_CRC, or PARSE_INCOMPLETE. */
static inline int parser_feed(parser_t *p, uint8_t byte)
{
    switch (p->state) {

    case PARSE_SOF0:
        if (byte == FRAME_SOF0)
            p->state = PARSE_SOF1;
        return PARSE_INCOMPLETE;

    case PARSE_SOF1:
        if (byte == FRAME_SOF1) {
            p->state = PARSE_LEN;
        } else {
            /* Re-check in case this byte is a new SOF0 */
            p->state = (byte == FRAME_SOF0) ? PARSE_SOF1 : PARSE_SOF0;
        }
        return PARSE_INCOMPLETE;

    case PARSE_LEN:
        p->len = byte;
        p->state = PARSE_TYPE;
        return PARSE_INCOMPLETE;

    case PARSE_TYPE:
        p->type = byte;
        p->state = PARSE_SEQ;
        return PARSE_INCOMPLETE;

    case PARSE_SEQ:
        p->seq = byte;
        p->payload_idx = 0;
        if (p->len == 0)
            p->state = PARSE_CRC_HI;
        else
            p->state = PARSE_PAYLOAD;
        return PARSE_INCOMPLETE;

    case PARSE_PAYLOAD:
        p->payload[p->payload_idx++] = byte;
        if (p->payload_idx >= p->len)
            p->state = PARSE_CRC_HI;
        return PARSE_INCOMPLETE;

    case PARSE_CRC_HI:
        p->crc_hi = byte;
        p->state = PARSE_CRC_LO;
        return PARSE_INCOMPLETE;

    case PARSE_CRC_LO: {
        uint16_t rx_crc = ((uint16_t)p->crc_hi << 8) | byte;

        /* Compute CRC over LEN + TYPE + SEQ + PAYLOAD */
        uint8_t hdr[3] = { p->len, p->type, p->seq };
        uint16_t calc_crc = 0xFFFFu;
        for (uint8_t i = 0; i < 3; i++)
            calc_crc = crc16_update(calc_crc, hdr[i]);
        for (uint8_t i = 0; i < p->len; i++)
            calc_crc = crc16_update(calc_crc, p->payload[i]);

        p->state = PARSE_SOF0;

        return (calc_crc == rx_crc) ? PARSE_OK : PARSE_BAD_CRC;
    }

    default:
        p->state = PARSE_SOF0;
        return PARSE_INCOMPLETE;
    }
}

#endif /* PROTOCOL_H */
