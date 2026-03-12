#ifndef SENSOR_SAMPLE_H
#define SENSOR_SAMPLE_H

#include <stdint.h>
#include "sensor_status.h"
#define SNAPSHOT_PAYLOAD_BYTES  20u

typedef struct {
    uint8_t  ir_obs[IR_COUNT];   /* 0=obstacle, 1=clear */
    uint8_t  us_obs[US_COUNT];   /* 0=obstacle, 1=clear */
    uint8_t  tof_obstacle;       /* 0=obstacle, 1=clear */
    uint8_t  gps_valid;          /* 0=invalid,  1=valid  */
    int32_t  lat_deg7;           /* latitude  * 1e7 */
    int32_t  lon_deg7;           /* longitude * 1e7 */
} snapshot_t;

static inline void snapshot_sample(snapshot_t *s)
{
    uint8_t i;
    for (i = 0; i < IR_COUNT; i++)
        s->ir_obs[i] = g_sensor_status.ir_obs[i];
    for (i = 0; i < US_COUNT; i++)
        s->us_obs[i] = g_sensor_status.us_obs[i];
    s->tof_obstacle = g_sensor_status.tof_obstacle;
    s->gps_valid    = g_sensor_status.gps_valid;
    s->lat_deg7     = g_sensor_status.lat_deg7;
    s->lon_deg7     = g_sensor_status.lon_deg7;
}

static inline void snapshot_pack(const snapshot_t *s, uint8_t *buf)
{
    uint8_t i;

    for (i = 0; i < IR_COUNT; i++)
        buf[i] = s->ir_obs[i];                             /* [0..5]  */

    for (i = 0; i < US_COUNT; i++)
        buf[6 + i] = s->us_obs[i];                        /* [6..9]  */

    buf[10] = s->tof_obstacle;                             /* [10]    */
    buf[11] = s->gps_valid;                                /* [11]    */

    /* lat_deg7 big-endian */
    buf[12] = (uint8_t)((uint32_t)s->lat_deg7 >> 24);     /* [12..15] */
    buf[13] = (uint8_t)((uint32_t)s->lat_deg7 >> 16);
    buf[14] = (uint8_t)((uint32_t)s->lat_deg7 >>  8);
    buf[15] = (uint8_t)((uint32_t)s->lat_deg7       );

    /* lon_deg7 big-endian */
    buf[16] = (uint8_t)((uint32_t)s->lon_deg7 >> 24);     /* [16..19] */
    buf[17] = (uint8_t)((uint32_t)s->lon_deg7 >> 16);
    buf[18] = (uint8_t)((uint32_t)s->lon_deg7 >>  8);
    buf[19] = (uint8_t)((uint32_t)s->lon_deg7       );
}

static inline void snapshot_unpack(snapshot_t *s, const uint8_t *buf)
{
    uint8_t i;

    for (i = 0; i < IR_COUNT; i++)
        s->ir_obs[i] = buf[i];

    for (i = 0; i < US_COUNT; i++)
        s->us_obs[i] = buf[6 + i];

    s->tof_obstacle = buf[10];
    s->gps_valid    = buf[11];

    s->lat_deg7 = (int32_t)(
        ((uint32_t)buf[12] << 24) |
        ((uint32_t)buf[13] << 16) |
        ((uint32_t)buf[14] <<  8) |
        ((uint32_t)buf[15]      ));

    s->lon_deg7 = (int32_t)(
        ((uint32_t)buf[16] << 24) |
        ((uint32_t)buf[17] << 16) |
        ((uint32_t)buf[18] <<  8) |
        ((uint32_t)buf[19]      ));
}

static inline uint8_t snapshot_any_obstacle(const snapshot_t *s)
{
    uint8_t i;
    for (i = 0; i < IR_COUNT; i++)
        if (s->ir_obs[i] == 0) return 1;
    for (i = 0; i < US_COUNT; i++)
        if (s->us_obs[i] == 0) return 1;
    if (s->tof_obstacle == 0) return 1;
    return 0;
}

#endif /* SENSOR_SAMPLE_H */
