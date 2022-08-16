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
 * -1 - inactive
 *  0 - auto
 *  1 - guided
 *  2 - manual
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
void sendLatLonToRecv();
void sendPOSToRecv();
void sendCtrlToRecv();
void sendStatus();
void handleCMDMessage(String);
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
int recvInterval = 400;
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

  if ((int)(currentTime - recvPreviousTime) >= recvInterval)
  {
    // sendLatLonToRecv();
    if (Serial3.available() > 0)
    {
      readRecvString = Serial3.readStringUntil('\n');
      readRecvString.trim();
      bool isValid = false;
      if (readRecvString.startsWith("$"))
      {
        isValid = validateMessage(readRecvString);
        Serial.print("msg : ");
        Serial.println(readRecvString);
        String messageId;
        String mainMessage;
        Serial.println(signMessage("$cmd,0,0,0"));
        if (isValid)
        {
          Serial.print("msg [A] : ");
          Serial.println(readRecvString);
          messageId = getMessageId(readRecvString);
          mainMessage = getMainMessage(readRecvString);
          Serial.println(messageId);
          Serial.println(mainMessage);
          if (messageId.equals("cmd"))
          {
            handleCMDMessage(mainMessage);
          }
        }
      }
    }
    recvPreviousTime = currentTime;
  }

  if ((int)(currentTime - controlPreviousTime) >= controlInterval)
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

void sendLatLonToRecv()
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
  // sendLatLonToRecv();
  // delay(100);
  // sendPOSToRecv();
  // String sendString = "$status";
  uint8_t sendString[29];
  uint8_t i = 0;
  sendString[i++] = 0x73; // 's' status message , message id
  append_double(sendString, lat, i);
  append_double(sendString, lon, i);
  append_double(sendString, psi_d, i);
  append_double(sendString, dist, i);
  append_double(sendString, psi, i);
  append_int(sendString, t_1, i);
  append_int(sendString, t_2, i);
  uint16_t crc = crcChecksumCalculator(sendString, i);
  uint8_t arr[2];
  *((double *)arr) = crc;
  sendString[i++] = arr[0];
  sendString[i++] = arr[1];
  sendString[i++] = 0x0D;
  sendString[i++] = 0x0A;
  // sendString[i++] = 0x00;
  for (int j = 0; j < 29; j++)
  {
    Serial.print("'");
    Serial.print(sendString[j], HEX);
    Serial.print("', ");
  }
  Serial.print("\n");
  // Serial3.print((char *)sendString);
  Serial3.write(sendString, 29);

  // sendString.concat(String(lat, 6));
  // sendString.concat(",");
  // sendString.concat(String(lon, 6));
  // sendString.concat(",");
  // sendString.concat(String(psi_d, 6));
  // sendString.concat(",");
  // sendString.concat(String(dist, 6));
  // sendString.concat(",");
  // sendString.concat(String(psi, 6));
  // sendString.concat(",");
  // sendString.concat(t_1);
  // sendString.concat(",");
  // sendString.concat(t_2);

  // Serial.print(signMessage(sendString)); // DEBUG
  // Serial3.print(signMessage(sendString));
  // Serial3.print(signMessage("HelloHelloHello"));
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
void handleCMDMessage(String mainMessage)
{
  String tokens[3];
  gps::tokenize(mainMessage, tokens, 3);
  if (tokens[2].equals("0"))
  { // mode select
    Serial.println(mode);
    if (tokens[3].toInt() >= -1 && tokens[3].toInt() <= 2) // mode select range
    {
      mode = tokens[3].toInt();
    }
    Serial.println(mode);
  }
}
