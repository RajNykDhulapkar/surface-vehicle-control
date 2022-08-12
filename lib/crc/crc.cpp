#include <Arduino.h>
#include <crc.h>

String getMessageId(String msg)
{
    msg.trim();
    int pos = msg.indexOf(',');
    return msg.substring(1, pos);
}

String signMessage(String msg)
{
    msg.trim();
    uint16_t checkSum = crcChecksumCalculator(msg.substring(1));
    msg.concat("*");
    msg.concat(checkSum);
    msg.concat("\r\n");
    return msg;
}

uint16_t crcChecksumCalculator(String str)
{
    uint16_t curr_crc = 0x0101;
    uint8_t sum1 = (uint8_t)curr_crc;
    uint8_t sum2 = (uint8_t)(curr_crc >> 8);
    int index;
    for (index = 0; index < str.length(); index = index + 1)
    {
        sum1 = (sum1 + str.charAt(index)) % 255;
        sum2 = (sum2 + sum1) % 255;
    }
    return (sum2 << 8) | sum1;
}
String getMainMessage(String str)
{
    int pos = str.indexOf('*');
    return str.substring(1, pos);
}
String getCRCCheckSum(String msg)
{
    int pos = msg.indexOf('*');
    return msg.substring(pos + 1);
}
bool validateMessage(String msg)
{
    uint16_t checkSum = getCRCCheckSum(msg).toInt();
    String mainMessage = getMainMessage(msg);
    Serial.print(" , ");
    Serial.print(crcChecksumCalculator(mainMessage));
    Serial.print(" , ");
    return crcChecksumCalculator(mainMessage) == checkSum;
}