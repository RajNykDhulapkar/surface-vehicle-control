#ifndef HELPERS_H
#define HELPERS_H
#include <Arduino.h>

int findIndexInBuffer(uint8_t *buff, uint8_t target, int size);
void printBuffer(uint8_t *buff, int size);

#endif