#ifndef SENSOR_STATUS_H
#define SENSOR_STATUS_H

#include <stdint.h>

/*
 * sensor_status.h — Shared Sensor State Contract
 *
 * This is the single source of truth for all sensor data on the Sensor Board.
 *
 * Rules:
 *   - g_sensor_status is DEFINED once in sensor_board/main.c
 *   - g_sensor_status is UPDATED ONLY by update_sensor_status() in main.c
 *   - sensor_sample.h reads from g_sensor_status (never writes to it)
 *   - Drivers do NOT write to g_sensor_status directly
 *
 * Sensor counts:
 *   IR:         6 obstacle sensors (active-low: 0=obstacle, 1=clear)
 *   Ultrasonic: 4 obstacle sensors (active-low: 0=obstacle, 1=clear)
 *   ToF:        1 obstacle sensor  (0=obstacle, 1=clear)
 *   GPS:        validity flag + lat/lon in degrees * 1e7
 */

#define IR_COUNT   6u
#define US_COUNT   4u

typedef struct {
    uint8_t  ir_obs[IR_COUNT];   /* IR sensor readings:    0=obstacle, 1=clear */
    uint8_t  us_obs[US_COUNT];   /* Ultrasonic readings:   0=obstacle, 1=clear */
    uint8_t  tof_obstacle;       /* ToF reading:           0=obstacle, 1=clear */
    uint8_t  gps_valid;          /* GPS fix valid:         0=invalid,  1=valid  */
    int32_t  lat_deg7;           /* Latitude  * 1e7 (e.g. 374230000 = 37.423 N) */
    int32_t  lon_deg7;           /* Longitude * 1e7 (e.g. -1220840000 = -122.084 W) */
} sensor_status_t;

/* Defined in sensor_board/main.c */
extern sensor_status_t g_sensor_status;

#endif /* SENSOR_STATUS_H */
