#include <Arduino.h>
#include <Wire.h>
#include <gps.h>
#include <imu.h>
#include <LatLonToUTM.h>
#include <Server.h>
#include <crc.h>

volatile byte count;
byte reload = 0x9C;

IMU imu(Wire, 0x68);
int imu_status = 0;

double lat = 0; // 15.455710, 73.802659
double lon = 0;
double utm_easting = 0;
double utm_northing = 0;

double target_lat = 0; // 15.495666, 73.947474
double target_lon = 0;
double utm_target_easting = 0;
double utm_target_northing = 0;

double dist;
double psi_d;

// control parameters
double k_p = 100 * PI / 180, k_d = 0.00;
double comm_m = 500 * 2;
double diff_m = 0;

double t_1 = (comm_m + diff_m) / 2, t_2 = (comm_m - diff_m) / 2;
double calculateDiff(double psi_d, double psi, double yaw_rate);
double calculatePsi_d();
double calculate_d();

double psi = 0;

unsigned long control_time_now = 0;
unsigned long guidance_time_now = 0;
unsigned long recv_time_now = 0;
int control_period = 100;
int guidance_period = 1500;
int recv_period = 100;
unsigned long currentTime;
unsigned long controlPreviousTime = 0;
unsigned long guidancePreviousTime = 0;
unsigned long recvPreviousTime = 0;
int controlInterval1 = 1000;
int guidanceInterval2 = 200;
int recvInterval3 = 400;

boolean toggle4 = LOW;

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial3.begin(9600);

  // imu_status = imu.begin();
  // if (imu_status < 0)
  // {
  //   Serial.println("IMU initialization unsuccessful");
  //   Serial.println("Check IMU wiring or try cycling power");
  //   Serial.print("Status: ");
  //   Serial.println(imu_status);
  //   while (1)
  //     ;
  // }
  // gps::send_command_to_module(GPS_SETUP_RESTORE_DEFAULT_SETTINGS, Serial1);

  target_lat = 15.495666;
  target_lon = 73.947474;
  LLtoUTM(target_lat, target_lon, &utm_target_easting, &utm_target_northing);

  // gps::send_command_to_module(GPS_SETUP_UPDATE_SAMPLING_1000ms, Serial1);
  // gps::send_command_to_module(GPS_SETUP_RMC_NMEA_SENTENCE, Serial1);
  // delay(100);
  delay(3000);
}

void loop()
{
  // gps::write_raw_module_data_to_prompt(Serial1, Serial);
  bool is_valid_cord = gps::getLatLon(&lat, &lon, Serial1);
  if (is_valid_cord)
  {
    char buffer[50];
    char lat_temp[10];
    char lon_temp[10];
    dtostrf(lat, 4, 6, lat_temp);
    dtostrf(lon, 4, 6, lon_temp);
    sprintf(buffer, "$gps,%s,%s", lat_temp, lon_temp);
    Serial.println(signMessage(String(buffer))); // DEBUG
    Serial3.println(signMessage(String(buffer)));
    // LLtoUTM(lat, lon, &utm_easting, &utm_northing);
    // LLtoUTM(target_lat, target_lon, &utm_target_easting, &utm_target_northing);
  }
  currentTime = millis();
  // gps::write_raw_module_data_to_prompt(Serial1, Serial);
  // Serial.print(gps::getLatLon(&lat, &lon, Serial1));
  // Serial.print(" ");
  // lat = 15.455904;
  // lon = 73.802584;

  // LLtoUTM(lat, lon, &utm_easting, &utm_northing);
  // target_lat = 15.456914;
  // target_lon = 73.802735;
  // LLtoUTM(target_lat, target_lon, &utm_target_easting, &utm_target_northing);

  // imu.readSensor();
  // imu.getHeading(&psi);
  // Serial.print(psi, 6);
  // Serial.print(" ");
  // Serial.print(lat, 6);
  // Serial.print(" ");
  // Serial.print(lon, 6);
  // Serial.print(" ");
  // Serial.print(utm_easting);
  // Serial.print(" ");
  // Serial.print(utm_northing);
  // Serial.print(" ");
  // Serial.print(target_lat);
  // Serial.print(" ");
  // Serial.print(target_lon);
  // Serial.print(" ");
  // Serial.print(utm_target_easting);
  // Serial.print(" ");
  // Serial.print(utm_target_northing);
  // double psi_d = calculatePsi_d();
  // Serial.print(" ");
  // Serial.print(psi_d, 6);
  // Serial.print(" ");
  // diff_m = calculateDiff(psi_d, psi, imu.getGyroZ_rads());
  // // diff_m = constrain(diff_m, 200, -200);
  // Serial.print(diff_m, 6);
  // Serial.print(" ");
  // t_1 = (comm_m - diff_m) / 2;
  // Serial.print(t_1, 2);
  // Serial.print(" ");
  // t_2 = (comm_m + diff_m) / 2;
  // Serial.print(t_2, 2);
  // float d = sqrtf(sq(utm_target_easting - utm_easting) + sq(utm_target_northing - utm_northing));
  // Serial.print(" ");
  // Serial.println(d, 2);
  // delay(1000);

  if (millis() >= guidance_time_now + guidance_period)
  {
    guidance_time_now += control_period;
    LLtoUTM(lat, lon, &utm_easting, &utm_northing);
    calculatePsi_d();
    char buffer[50];
    char psi_d_temp[10];
    char d_temp[10];
    dtostrf(psi_d, 4, 6, psi_d_temp);
    dtostrf(calculate_d(), 4, 6, d_temp);
    sprintf(buffer, "$pos,%s,%s", psi_d_temp, d_temp);
    Serial.println(signMessage(String(buffer))); // DEBUG
    Serial3.println(signMessage(String(buffer)));
  }

  // TODO remove this block
  // log received data on Serial0
  // this block of code will be present on the receiver module
  if (millis() >= recv_time_now + recv_period)
  {
    recv_time_now += recv_period;
    LLtoUTM(lat, lon, &utm_easting, &utm_northing);
  }

  if (millis() >= control_time_now + control_period)
  {
    control_time_now += control_period;
  }

  // if (gps::getLatLon(&lat, &lon, Serial1) && false)
  // {
  //   String sendString;
  //   sendString.concat("$gps,");
  //   sendString.concat(String(lat, 10));
  //   sendString.concat(",");
  //   sendString.concat(String(lon, 10));
  //   Serial.println(sendString);
  //   Serial3.println(sendString);
  //   LLtoUTM(lat, lon, &utm_easting, &utm_northing);
  //   LLtoUTM(target_lat, target_lon, &utm_target_easting, &utm_target_northing);
  // }
}

double calculateDiff(double psi_d, double psi, double yaw_rate)
{
  return (k_p * (psi_d - psi) - k_d * yaw_rate);
  // return (psi_d - psi);
}

double calculate_d()
{
  return sqrtf(sq(utm_target_easting - utm_easting) + sq(utm_target_northing - utm_northing));
}

double calculatePsi_d()
{
  double d_y = (utm_target_northing - utm_northing);
  double d_x = (utm_target_easting - utm_easting);
  if ((d_x > 0 && d_y > 0) || (d_x < 0 && d_y > 0))
    return atan2(d_y, d_x) * 180 / PI;
  else
    return (atan2(d_y, d_x) * 180 / PI + 360);
}
