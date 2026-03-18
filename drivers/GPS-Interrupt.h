
#ifndef GPS-INTERRUPT_H
#define GPS-INTERRUPT_H

#define parse_RMC(char *sentence);
typedef struct {
    double latitude;   // decimal degrees
    double longitude;  // decimal degrees
    char status;       // 'A' = valid, 'V' = void
    double speedKnots; // speed over ground
} RMC_Data;
#endif