
void setup()
{
    Serial.begin(9600);
}
void loop()
{
    double lat = 15.123456;
    byte byteArray[4];
    *((double *)byteArray) = lat;
    Serial.println(byteArray[0], HEX);
    Serial.println(byteArray[1], HEX);
    Serial.println(byteArray[2], HEX);
    Serial.println(byteArray[3], HEX);
    Serial.println((int)byteArray[0]);
    Serial.println((int)byteArray[1]);
    Serial.println((int)byteArray[2]);
    Serial.println((int)byteArray[3]);
    double value = *((double *)(byteArray));
    Serial.println(value, 6);
    Serial.println(String((char *)byteArray));

    uint16_t crc = 23456;
    uint8_t arr[2];

    delay(10000);
}
