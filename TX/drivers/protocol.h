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

#define PARSE_INCOMPLETE  0
#define PARSE_OK          1
#define PARSE_BAD_CRC    -1

uint16_t crc16_update(uint16_t crc, uint8_t byte);
uint16_t crc16_calc(const uint8_t *data, uint8_t len);
uint8_t frame_pack(uint8_t *buf, uint8_t type, uint8_t seq,
                   const uint8_t *payload, uint8_t payload_len);
void parser_init(parser_t *p);
int parser_feed(parser_t *p, uint8_t byte);

#endif /* PROTOCOL_H */
