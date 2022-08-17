#include "helpers.h"
#include <Arduino.h>

int findIndexInBuffer(uint8_t *buff, uint8_t target, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (buff[i] == target)
            return i;
    }
    return -1;
}
void printBuffer(uint8_t *buff, int size)
{

    Serial.print("<Buffer ");
    for (int j = 0; j < size; j++)
    {
        Serial.print("'");
        Serial.print(buff[j], HEX);
        Serial.print("', ");
    }
    Serial.print(">\n");
}
