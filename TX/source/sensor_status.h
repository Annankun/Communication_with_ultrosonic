#ifndef SENSOR_STATUS_H
#define SENSOR_STATUS_H

#include <stdint.h>

#define IR_COUNT   6u
#define US_COUNT   3u

typedef struct {
    uint8_t  ir_obs[IR_COUNT];
    uint8_t  us_obs[US_COUNT];
    uint8_t  tof_obstacle;
    uint8_t  gps_valid;
    int32_t  lat_deg7;
    int32_t  lon_deg7;
} sensor_status_t;

extern sensor_status_t g_sensor_status;

#endif /* SENSOR_STATUS_H */
