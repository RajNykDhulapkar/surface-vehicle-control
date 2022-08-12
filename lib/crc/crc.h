#ifndef CRC_H
#define CRC_H
#include <Arduino.h>

String getMessageId(String msg);
String signMessage(String msg);
uint16_t crcChecksumCalculator(String str);
String getMainMessage(String str);
String getCRCCheckSum(String msg);
bool validateMessage(String msg);

#endif