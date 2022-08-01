#ifndef GPS_H
#define GPS_H
/*...
module: L80 M39 Quectel


...*/

#include <Arduino.h>

#define GPS_SETUP_TURN_OFF_ANT_DATA_NUISANCE "$PGCMD,33,0*6D\r\n\0"                               // turn off antenna data nuisance
#define GPS_SETUP_UPDATE_SAMPLING_1000ms "$PMTK220,1000*1F\r\n\0"                                 // set update frequency to 1Hz
#define GPS_SETUP_UPDATE_SAMPLING_5000ms "$PMTK220,5000*1B\r\n\0"                                 // set update frequency to 1Hz
#define GPS_SETUP_RMC_NMEA_SENTENCE "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n\0"     // RMC NMEA Sentence
#define GPS_SETUP_GGA_NMEA_SENTENCE "$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n\0"     // GGA NMEA Sentence
#define GPS_SETUP_GGA_RMC_NMEA_SENTENCE "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n\0" // GGA and RMC NMEA Sentence
#define GPS_SETUP_ALL_SENTENCE "$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0*29\r\n\0"              // turn on all sentences
#define GPS_SETUP_NO_SENTENCE "$PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n\0"               // turn off all sentences
#define GPS_SETUP_RESTORE_DEFAULT_SETTINGS "$PMTK314,-1*04\r\n\0"

namespace gps
{
    long int calculateCheckSum(char *str);

    char *calculateCheckSumInHexadecimal(char *str);

    char *decimalToHexadecimal(long int decimalNumber);

    void send_command_to_module(const char *, HardwareSerial &refSerial);

    void write_raw_module_data_to_prompt(HardwareSerial &refSerial, HardwareSerial &promptSerial);

    void write_processed_module_data_to_prompt(HardwareSerial &refSerial, HardwareSerial &promptSerial);

    float ConvertData(float RawDegrees);

}

#endif