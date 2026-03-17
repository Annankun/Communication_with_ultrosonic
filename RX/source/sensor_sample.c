#include "sensor_sample.h"

void snapshot_sample(snapshot_t *s)
{
    uint8_t i;
    for (i = 0; i < IR_COUNT; i++)
        s->ir_obs[i] = g_sensor_status.ir_obs[i];
    s->us_priority = g_sensor_status.us_priority;
    s->tof_obstacle = g_sensor_status.tof_obstacle;
    s->gps_valid    = g_sensor_status.gps_valid;
    s->lat_deg7     = g_sensor_status.lat_deg7;
    s->lon_deg7     = g_sensor_status.lon_deg7;
}

void snapshot_pack(const snapshot_t *s, uint8_t *buf)
{
    uint8_t i;

    for (i = 0; i < IR_COUNT; i++)
        buf[i] = s->ir_obs[i];

    buf[6] = s->us_priority;

    buf[7] = s->tof_obstacle;
    buf[8] = s->gps_valid;

    buf[9]  = (uint8_t)((uint32_t)s->lat_deg7 >> 24);
    buf[10] = (uint8_t)((uint32_t)s->lat_deg7 >> 16);
    buf[11] = (uint8_t)((uint32_t)s->lat_deg7 >> 8);
    buf[12] = (uint8_t)((uint32_t)s->lat_deg7);

    buf[13] = (uint8_t)((uint32_t)s->lon_deg7 >> 24);
    buf[14] = (uint8_t)((uint32_t)s->lon_deg7 >> 16);
    buf[15] = (uint8_t)((uint32_t)s->lon_deg7 >> 8);
    buf[16] = (uint8_t)((uint32_t)s->lon_deg7);
}

void snapshot_unpack(snapshot_t *s, const uint8_t *buf)
{
    uint8_t i;

    for (i = 0; i < IR_COUNT; i++)
        s->ir_obs[i] = buf[i];

    s->us_priority = buf[6];

    s->tof_obstacle = buf[7];
    s->gps_valid    = buf[8];

    s->lat_deg7 = (int32_t)(
        ((uint32_t)buf[9] << 24) |
        ((uint32_t)buf[10] << 16) |
        ((uint32_t)buf[11] << 8) |
        ((uint32_t)buf[12]));

    s->lon_deg7 = (int32_t)(
        ((uint32_t)buf[13] << 24) |
        ((uint32_t)buf[14] << 16) |
        ((uint32_t)buf[15] << 8) |
        ((uint32_t)buf[16]));
}

uint8_t snapshot_any_obstacle(const snapshot_t *s)
{
    uint8_t i;
    for (i = 0; i < IR_COUNT; i++)
        if (s->ir_obs[i] == 0) return 1;
    if (s->us_priority != 0u) return 1;
    if (s->tof_obstacle == 0) return 1;
    return 0;
}
