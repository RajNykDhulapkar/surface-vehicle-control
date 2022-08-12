#ifndef UTM_H
#define UTM_H

#include <Arduino.h>

/**
 * Geo coordinate are entered in displayed in degrees
 * utm coordinate in meters
 * The ellipsoid model used for computations is WGS84.
 *
 * converted from python
 *
 */

class UTM
{
public:
    float degToFLoat(int degrees, int minute, int seconds);
    float degToRad(float degrees);
    float radToDeg(float radians);
    float arcLengthOfMeridian(float phi);
    float UTMCentralMeridian(int zone);
    float footPointLatitude(float y);
    float mapLatLonToXY(float phi, float lambda_pt, float lambda_ctr);
    float mapXYToLatLon(float x, float y, float lambda_ctr);
    float LatLonToUTMXY(float lat, float lon, int zone);
    float UTMXYToLatLon(float x, float y, int zone, bool southhemi);
    // float LatLonToUtm(lat, lon);
    // float UtmToLatLon(x, y, zone, hemi);

private:
    // Ellipsoid model constants(actual values here are for WGS84)
    const float sm_a = 6378137.0;
    const float sm_b = 6356752.314;
    const float sm_EccSquared = 6.69437999013e-03;
    const float UTMScaleFactor = 0.9996;
}

#endif