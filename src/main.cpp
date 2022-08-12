#include <Arduino.h>
#include <Wire.h>
#include <gps.h>
#include <imu.h>
#include <LatLonToUTM.h>
#include <Server.h>
#include <crc.h>
#include "MPU9250.h"

volatile byte count;
byte reload = 0x9C;

MPU9250 imu(Wire, 0x68);
int imu_status = 0;

/*
 * mode
 * 0 - auto
 * 1 - guided
 * 2 - manual
 */
int mode = 0;

double lat = 15.455954; // 15.456031
double lon = 73.802089; // 73.802099
double utm_easting = 0;
double utm_northing = 0;

double target_lat = 15.457496; // 15.457080
double target_lon = 73.803382; // 73.802948
double utm_target_easting = 0;
double utm_target_northing = 0;

volatile double dist;
volatile double psi_d;

// control parameters
double k_p = 180 * PI / 180, k_d = 0.00;
double comm_m = 500 * 2;
volatile double diff_m = 0;

int t_1 = (comm_m + diff_m) / 2, t_2 = (comm_m - diff_m) / 2;
void calculateDiff();
void calculatePsi_d();
void calculate_d();
void calculate_psi();
void sendLatLonToRecv(double lat, double lon);
void sendPOSToRecv();
void sendCtrlToRecv();
void sendStatus();

volatile double psi = 0;
volatile double yaw_rate = 0;

unsigned long currentTime;
unsigned long controlPreviousTime = 0;
unsigned long guidancePreviousTime = 0;
unsigned long recvPreviousTime = 0;
unsigned long sendPreviousTime = 0;
int controlInterval = 500;
int guidanceInterval = 900;
int recvInterval = 400;
int sendInterval = 1500;

boolean toggle4 = LOW;

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial3.begin(9600);
  Wire.begin();

  imu_status = imu.begin();
  if (imu_status < 0)
  {
    Serial.println("IMU initialization unsuccessful");
    Serial.println("Check IMU wiring or try cycling power");
    Serial.print("Status: ");
    Serial.println(imu_status);
    while (1)
      ;
  }
  // gps::send_command_to_module(GPS_SETUP_RESTORE_DEFAULT_SETTINGS, Serial1);

  LLtoUTM(target_lat, target_lon, &utm_target_easting, &utm_target_northing);

  gps::send_command_to_module(GPS_SETUP_UPDATE_SAMPLING_1500ms, Serial1);
  gps::send_command_to_module(GPS_SETUP_RMC_NMEA_SENTENCE, Serial1);
  // delay(100);
  delay(3000);
  Serial.println("Start");
}

void loop()
{
  // gps::write_raw_module_data_to_prompt(Serial1, Serial);
  // gps::getLatLon(&lat, &lon, Serial1);
  bool is_valid_cord = true;
  if (is_valid_cord)
  {
    // char buffer[50];
    // char temp_lat[10];
    // char temp_lon[10];
    lat = 15.456031;
    lon = 73.802099;
    // sendLatLonToRecv(lat, lon);
    // dtostrf(lon, 4, 5, temp_lon);
    // dtostrf(lat, 4, 5, temp_lat);

    // String sendString = "$gps,";
    // sendString += String(lat, 6);
    // sendString += ",";
    // sendString += String(lon, 6);
    // String sendString = "$gps,0.00,0.00";
    // sprintf(buffer, "$gps,%s,%s", String(lat, 6).c_str(), String(lon, 6).c_str());
    // Serial.println(sendString); // DEBUG
    // Serial3.println(sendString);
    // delay(1000);
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

  if (currentTime - controlPreviousTime >= controlInterval)
  {
    imu.readSensor();
    calculate_psi();
    calculateDiff();
    t_1 = (comm_m - diff_m) / 2;
    t_2 = (comm_m + diff_m) / 2;
    // sendCtrlToRecv();
    controlPreviousTime = currentTime;
  }

  if ((int)(currentTime - guidancePreviousTime) >= guidanceInterval)
  {
    LLtoUTM(lat, lon, &utm_easting, &utm_northing);
    calculatePsi_d();
    calculate_d();
    // sendPOSToRecv();
    guidancePreviousTime = currentTime;
  }

  if (currentTime - sendPreviousTime >= sendInterval)
  {
    // sendLatLonToRecv();
    sendStatus();
    sendPreviousTime = currentTime;
  }
}

void calculateDiff()
{
  diff_m = (k_p * (psi_d - psi) - k_d * yaw_rate);
  // return (psi_d - psi);
}

void calculate_d()
{
  dist = sqrtf(sq(utm_target_easting - utm_easting) + sq(utm_target_northing - utm_northing));
}

void calculatePsi_d()
{
  double d_y = (utm_target_northing - utm_northing);
  double d_x = (utm_target_easting - utm_easting);
  if ((d_x > 0 && d_y > 0) || (d_x < 0 && d_y > 0))
    psi_d = atan2(d_y, d_x) * 180 / PI;
  else
    psi_d = (atan2(d_y, d_x) * 180 / PI + 360);
}

void sendLatLonToRecv(double lat, double lon)
{
  String sendString = "$gps,";
  sendString.concat(String(lat, 6));
  sendString.concat(",");
  sendString.concat(String(lon, 6));
  Serial.print(signMessage(sendString)); // DEBUG
  Serial3.print(signMessage(sendString));
}

void sendPOSToRecv()
{
  String sendString = "$pos,";
  sendString.concat(String(psi_d, 6));
  sendString.concat(",");
  sendString.concat(String(dist, 6));
  Serial.print(signMessage(sendString)); // DEBUG
  Serial3.print(signMessage(sendString));
}

void sendCtrlToRecv()
{
  String sendString = "$ctrl,";
  sendString.concat(String(psi, 6));
  sendString.concat(",");
  sendString.concat(t_1);
  sendString.concat(",");
  sendString.concat(t_2);
  Serial.print(signMessage(sendString)); // DEBUG
  Serial3.print(signMessage(sendString));
}

void sendStatus()
{
  String sendString = "$status,";
  sendString.concat(String(lat, 6));
  sendString.concat(",");
  sendString.concat(String(lon, 6));
  sendString.concat(",");
  sendString.concat(String(psi_d, 6));
  sendString.concat(",");
  sendString.concat(String(dist, 6));
  sendString.concat(",");
  sendString.concat(String(psi, 6));
  sendString.concat(",");
  sendString.concat(t_1);
  sendString.concat(",");
  sendString.concat(t_2);
  Serial.print(signMessage(sendString)); // DEBUG
  Serial3.print(signMessage(sendString));
}

void calculate_psi()
{
  imu.readSensor();
  float magX = imu.getMagX_uT() - 19.03355;
  float magY = imu.getMagY_uT() - 27.47569;
  float magZ = imu.getMagZ_uT() - 66.334158;
  yaw_rate = imu.getGyroBiasZ_rads();

  magX = 0.89534 * magX - 0.017772 * magY - 0.01735 * magZ;
  magY = -0.017772 * magX + 0.883792 * magY - 0.022609;

  float temp = magX;
  magX = magY;
  magY = temp;

  psi = atan2(magY, magX) * 180 / PI;
  return;

  if ((magX > 0 && magY > 0) || (magX < 0 && magY > 0))
    if ((360 - (atan2(magY, magX) * 180 / PI)) > 270)
      psi = (360 - (atan2(magY, magX) * 180 / PI)) - 270;
    else
      psi = (360 - (atan2(magY, magX) * 180 / PI)) + 90;
  else
    psi = (360 - (atan2(magY, magX) * 180 / PI + 360)) + 90;
}
