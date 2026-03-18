#include "GPS_helper.h"
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>



volatile double boundary1LatTgt = 45.388889;
volatile double boundary1LonTgt = (-1)*75.698055;

volatile double boundary2LatTgt = 45.389166;
volatile double boundary2LonTgt = (-1) * 75.698333;

volatile double boundary3LatTgt= 45.388889;
volatile double boundary3LonTgt= ((-1)*75.698611);

volatile double boundary4LatTgt = 45.388611;
volatile double boundary4LonTgt = (-1) * 75.698611;

volatile double latNorth = 45.389166;
volatile double latSouth = 45.388611;
volatile double lonWest = (-1) * 75.698611;
volatile double lonEast = (-1)*75.698055;

double bearingAngle(double lat,double lon){
	double angle = atan2(lon,lat);
	double angleDeg = angle * (180.0/PI);
	if (angleDeg < 0){
		return angleDeg + 360;
	}
	return angleDeg;
}


/**
 * Calculates the conversion of decimal degrees to meters per latitude and longitude.
 * Uses pointers to update the delta latitude and delta longitude.
 */
void latLonToMeters(double latDeg, double lonDeg, double latTgt, double lonTgt,double *diffLat, double *diffLon){
	double latRad = (latDeg * PI)/180.0;
	*diffLat = (latTgt - latDeg) * LAT_Constant;
	*diffLon = (lonTgt - lonDeg) * LAT_Constant * cos(latRad);
}
/**
 * Determines whether current latitude and longitude inside the boundaries of the parking lot.
 */
bool insideLot(double currentLat, double currentLon){
	if(currentLat <=latNorth && currentLat>=latSouth){
		if(currentLon >= lonWest && currentLon <= lonEast){
			return true;
		}
	}
	return false;
}
/**
 * Calculates the closest set boundary point to the current latitude and longitude.
 * Returns the number to the closest boundary point and updates the current distance to goal.
 *
 *
 *
 */
void calcClosestBoundary(double currentLat, double currentLon, double *currDist, int *closestBound, double *angle){
	double b1Lat = 0, b1Lon = 0;
	double b2Lat = 0, b2Lon= 0;
	double b3Lat = 0, b3Lon = 0;
	double b4Lat = 0,b4Lon = 0;
	//Check proximity to boundary 1
	latLonToMeters(currentLat,currentLon,boundary1LatTgt,boundary1LonTgt,&b1Lat,&b1Lon);
	//Check proximity to boundary 2
	latLonToMeters(currentLat,currentLon,boundary2LatTgt,boundary2LonTgt,&b2Lat,&b2Lon);
	//Check proximity to boundary 3
	latLonToMeters(currentLat,currentLon,boundary3LatTgt,boundary3LonTgt,&b3Lat,&b3Lon);
	//Check proximity to boundary 4
	latLonToMeters(currentLat,currentLon,boundary4LatTgt,boundary4LonTgt,&b4Lat,&b4Lon);

	double d1 = sqrt((b1Lat * b1Lat)+(b1Lon*b1Lon));
	double d2 = sqrt((b2Lat * b2Lat)+(b2Lon*b2Lon));
	double d3 = sqrt((b3Lat * b3Lat)+(b3Lon*b3Lon));
	double d4 = sqrt((b4Lat * b4Lat)+(b4Lon*b4Lon));
	double min = d1;
	int closest = 1;
	if(min > d2){
		min = d2;
		closest = 2;
	}
	if(min > d3){
		min = d3;
		closest =3;
	}
	if(min > d4){
		min = d4;
		closest =4;
	}
	*currDist = min;
	*closestBound = closest;

	switch (closest) {
			case 1:
				*angle = bearingAngle(b1Lat, b1Lon);
				break;
			case 2:
				*angle = bearingAngle(b2Lat, b2Lon);
				break;
			case 3:
				*angle = bearingAngle(b3Lat, b3Lon);
				break;
			case 4:
				*angle = bearingAngle(b4Lat, b4Lon);
				break;
		}


}

