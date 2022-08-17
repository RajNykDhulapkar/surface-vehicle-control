#include <Arduino.h>
#include <Wire.h>
#include <gps.h>
#include <imu.h>
#include <LatLonToUTM.h>
#include <Server.h>
#include <crc.h>
#include "MPU9250.h"
#include <helpers.h>

volatile byte count;
byte reload = 0x9C;

MPU9250 imu(Wire, 0x68);
int imu_status = 0;

/*
 * mode
 *  0 - auto
 *  1 - guided
 *  2 - abort
 */
uint8_t mode = 2;

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
double comm_m = 0;
volatile double diff_m = 0;

int t_1 = (comm_m + diff_m) / 2, t_2 = (comm_m - diff_m) / 2;
void calculateDiff();
void calculatePsi_d();
void calculate_d();
void calculate_psi();
void sendLatLonToRecv();
void sendPOSToRecv();
void sendCtrlToRecv();
void sendStatus();
void handleCMDMessage(uint8_t *message, int startIndex, int endIndex);
void handleMISMessage(uint8_t *message, int startIndex, int endIndex);
void double_to_byteArray(double, uint8_t *);
void int_to_byteArray(int, uint8_t *);
void append_double(uint8_t *, double, uint8_t &);
void append_int(uint8_t *, int, uint8_t &);

volatile double psi = 0;
volatile double yaw_rate = 0;

unsigned long currentTime;
unsigned long controlPreviousTime = 0;
unsigned long guidancePreviousTime = 0;
unsigned long recvPreviousTime = 0;
unsigned long sendPreviousTime = 0;
int controlInterval = 500;
int guidanceInterval = 900;
int recvInterval = 300;
int sendInterval = 1500;

boolean toggle4 = LOW;
String readRecvString;

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
  bool is_valid_cord = gps::getLatLon(&lat, &lon, Serial1);
  if (true)
  {
    lat = 15.456031;
    lon = 73.802099;
  }
  currentTime = millis();

  if ((int)(currentTime - recvPreviousTime) >= recvInterval)
  {
    if (Serial3.available())
    {

      uint8_t buf[15];
      Serial3.readBytesUntil('\n', buf, sizeof(buf));
      Serial.print("    recv : ");
      printBuffer(buf, (int)sizeof(buf));
      int startIndex = findIndexInBuffer(buf, (uint8_t)'$', (int)sizeof(buf)) + 1;
      int endIndex = findIndexInBuffer(buf, (uint8_t)'\r', (int)sizeof(buf));
      if (startIndex != -1 && endIndex != -1)
      {
        if (buf[startIndex] == (uint8_t)'c')
          handleCMDMessage(buf, startIndex, endIndex);
        else if (buf[startIndex] == (uint8_t)'m')
          handleMISMessage(buf, startIndex, endIndex);
      }
    }

    recvPreviousTime = currentTime;
  }

  if ((int)(currentTime - controlPreviousTime) >= controlInterval)
  {
    imu.readSensor();
    calculate_psi();
    calculateDiff();

    if ((mode == 0))
    {
      t_1 = (comm_m - diff_m) / 2;
      t_2 = (comm_m + diff_m) / 2;
    }

    controlPreviousTime = currentTime;
  }

  if (((int)(currentTime - guidancePreviousTime) >= guidanceInterval))
  {
    if (mode == 0)
    {
      LLtoUTM(lat, lon, &utm_easting, &utm_northing);
      calculatePsi_d();
      calculate_d();
      Serial.print("updated pdi_d");
    }

    // sendPOSToRecv();
    guidancePreviousTime = currentTime;
  }

  if ((int)(currentTime - sendPreviousTime) >= sendInterval)
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

void sendStatus()
{
  uint8_t sendString[30];
  uint8_t i = 0;
  sendString[i++] = 0x73;              // 's' status message , message id
  append_double(sendString, lat, i);   //  1- 4
  append_double(sendString, lon, i);   // 5 - 8
  append_double(sendString, psi_d, i); // 9 - 12
  append_double(sendString, dist, i);  // 13 - 16
  append_double(sendString, psi, i);   // 17 - 20
  sendString[i++] = mode;              // 21
  append_int(sendString, t_1, i);      // 22 - 23
  append_int(sendString, t_2, i);      // 24 -25
  uint16_t crc = crcChecksumCalculator(sendString, i);
  uint8_t arr[2];
  *((uint16_t *)arr) = crc;
  sendString[i++] = arr[0]; // 26
  sendString[i++] = arr[1]; // 27
  sendString[i++] = 0x0D;   // 28
  sendString[i++] = 0x0A;   // 29
  Serial.print("send : ");
  printBuffer(sendString, (int)sizeof(sendString));
  Serial3.write(sendString, (int)sizeof(sendString));
}

void append_double(uint8_t *buff, double num, uint8_t &i)
{
  uint8_t temp[4];
  double_to_byteArray(num, temp);
  for (int k = 0; k < 4; k++)
  {
    buff[i++] = temp[k];
  }
}

void append_int(uint8_t *buff, int num, uint8_t &i)
{
  uint8_t temp[2];
  int_to_byteArray(num, temp);
  for (int k = 0; k < 2; k++)
  {
    buff[i++] = temp[k];
  }
}
void int_to_byteArray(int num, uint8_t *arr)
{
  *((int *)arr) = num;
}

void double_to_byteArray(double num, uint8_t *arr)
{
  *((double *)arr) = num;
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

// handling recv message
void handleCMDMessage(uint8_t *message, int startIndex, int endIndex)
{
  // message: 0x63 | b1 | b2 | b3 | b4 | cs_b5 cs_b6
  Serial.println("        Handling command message");
  if (message[startIndex + 3] == 0)
  {
    mode = message[startIndex + 4];
    Serial.print("        Mode select operation : set mode to ");
    Serial.println(mode);
    if (mode == 0)
    {
      diff_m = 0;
      comm_m = 500 * 2;
      t_1 = (comm_m + diff_m) / 2;
      t_2 = (comm_m - diff_m) / 2;
    }
    else if (mode == 2)
    {
      diff_m = 0;
      comm_m = 0;
      t_1 = 0;
      t_2 = 0;
    }
  }
}

void handleMISMessage(uint8_t *message, int startIndex, int endIndex)
{
  // message: 0x6d |  b1  b2  b3  b4 | b5 b6 b7 b8 | cs_b5 cs_b6
  Serial.println("        Handling mission message");
  uint8_t temp[4];
  int j = 3;
  startIndex++;
  for (int i = startIndex; i < startIndex + 4; i++)
  {
    temp[j--] = message[i];
  }
  double lat = *((double *)(temp));
  Serial.print("        ");
  printBuffer(temp, (int)sizeof(temp));
  Serial.print("        ");
  Serial.println(lat, 6);
  target_lat = lat;

  j = 3;
  for (int i = startIndex + 4; i < startIndex + 8; i++)
  {
    temp[j--] = message[i];
  }
  double lon = *((double *)(temp));
  Serial.print("        ");
  printBuffer(temp, (int)sizeof(temp));
  Serial.print("        ");
  Serial.println(lon, 6);
  target_lon = lon;

  LLtoUTM(target_lat, target_lon, &utm_target_easting, &utm_target_northing);
}
