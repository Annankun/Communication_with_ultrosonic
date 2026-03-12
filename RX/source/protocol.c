#include "protocol.h"

uint16_t crc16_update(uint16_t crc, uint8_t byte)
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

uint16_t crc16_calc(const uint8_t *data, uint8_t len)
{
    uint16_t crc = 0xFFFFu;
    for (uint8_t i = 0; i < len; i++)
        crc = crc16_update(crc, data[i]);
    return crc;
}

uint8_t frame_pack(uint8_t *buf, uint8_t type, uint8_t seq,
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

    uint16_t crc = crc16_calc(&buf[2], (uint8_t)(pos - 2));
    buf[pos++] = (uint8_t)(crc >> 8);
    buf[pos++] = (uint8_t)(crc & 0xFF);

    return pos;
}

void parser_init(parser_t *p)
{
    p->state = PARSE_SOF0;
    p->payload_idx = 0;
}

int parser_feed(parser_t *p, uint8_t byte)
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
        p->state = (p->len == 0) ? PARSE_CRC_HI : PARSE_PAYLOAD;
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
