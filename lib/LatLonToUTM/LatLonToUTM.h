#ifndef LatLonToUTM_H
#define LatLonToUTM_H

#include <Arduino.h>
#include <math.h>

String LLtoUTM(const double Lat, const double Long, double *utm_easting, double *utm_northing);
String MGRSZoneDesignator(double UTMEasting, double UTMNorthing);
char UTMLetterDesignator(double Lat);

const double FOURTHPI = PI / 4;
const double deg2rad = PI / 180;
const double rad2deg = 180.0 / PI;
const double equrad = 6377563;
const double squecc = 0.00667054;

#endif