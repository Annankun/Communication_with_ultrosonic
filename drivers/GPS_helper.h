#include <stdbool.h>
#include <stddef.h>
#ifndef GPS_HELPER_H
#define GPS_HELPER_H

#define PI 3.14159265
#define B1 1
#define B2 2
#define B3 3
#define B4 4

#define MAX_SENTENCE 128
#define MAX_PARSE 256


#define LAT_Constant 111000

double bearingAngle(double lat,double lon);

void latLonToMeters(double latDeg, double lonDeg, double latTgt, double lonTgt,double *diffLat, double *diffLon);

bool insideLot(double currentLat, double currentLon);
void calcClosestBoundary(double currentLat, double currentLon, double *currDist, int *closestBound, double *angle);
#endif
