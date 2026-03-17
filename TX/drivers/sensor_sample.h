#ifndef SENSOR_SAMPLE_H
#define SENSOR_SAMPLE_H

#include <stdint.h>
#include "sensor_status.h"

#define SNAPSHOT_PAYLOAD_BYTES  17u

typedef struct {
    uint8_t  ir_obs[IR_COUNT];
    uint8_t  us_priority;
    uint8_t  tof_obstacle;
    uint8_t  gps_valid;
    int32_t  lat_deg7;
    int32_t  lon_deg7;
} snapshot_t;

void snapshot_sample(snapshot_t *s);
void snapshot_pack(const snapshot_t *s, uint8_t *buf);
void snapshot_unpack(snapshot_t *s, const uint8_t *buf);
uint8_t snapshot_any_obstacle(const snapshot_t *s);

#endif /* SENSOR_SAMPLE_H */
