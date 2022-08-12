#include <Arduino.h>
#include "gps.h"
#include <string.h>

long int gps::calculateCheckSum(char *str)
{
    long int checkSum = 0;
    while (*str != '\0')
    {
        checkSum ^= (int)*(str++);
    }
    return checkSum;
}

char *gps::calculateCheckSumInHexadecimal(char *str)
{
    return gps::decimalToHexadecimal(gps::calculateCheckSum(str));
}

char *gps::decimalToHexadecimal(long int decimalNumber)
{
    {
        long int quotient;
        long int remainder;
        static char hexDecNum[100]; // must be static
        static char returnHexDecNum[100];

        quotient = decimalNumber;
        int i = 0;
        unsigned int j;
        while (quotient != 0)
        {
            remainder = quotient % 16;

            // to convert integer into character
            if (remainder < 10)
            {
                remainder = remainder + 48;
            }
            else
            {
                remainder = remainder + 55;
            }
            hexDecNum[i++] = remainder;
            quotient = quotient / 16;
        }
        hexDecNum[i] = '\0';
        for (j = 0; j < strlen(hexDecNum); j++)
        {
            returnHexDecNum[j] = hexDecNum[strlen(hexDecNum) - 1 - j];
        }
        returnHexDecNum[j] = '\0';
        return returnHexDecNum;
    }
}

void gps::send_command_to_module(const char *command, HardwareSerial &refSerial)
{
    const char *ptrCommand = command;
    while (!(refSerial.available() > 0))
    {
        if (*ptrCommand != '\0')
        {
            refSerial.write(*(ptrCommand++));
        }
        else
        {
            break;
        }
    }
}

void gps::write_raw_module_data_to_prompt(HardwareSerial &refSerial, HardwareSerial &promptSerial)
{
    while (refSerial.available())
    {
        promptSerial.write((char)refSerial.read());
    }
}

void gps::write_processed_module_data_to_prompt(HardwareSerial &refSerial, HardwareSerial &promptSerial)
{
    String ReadString;
    bool isValid = false;

    ReadString = Serial1.readStringUntil(13); // NMEA data ends with 'return' character, which is ascii(13)
    ReadString.trim();
    if (ReadString.startsWith("$GPRMC"))
    {
        // I picked this sentence, you can pick any of the other labels and rearrange/add sections as needed.
        promptSerial.print("$gps;");
        int Pos = ReadString.indexOf(',');
        ReadString.remove(0, Pos + 1);
        Pos = ReadString.indexOf(',');
        char UTC_time[Pos + 5]; // declare character array UTC_time with a size of the dbit of data
        int i;
        for (i = 0; i <= Pos - 1; i++)
        { // load charcters into array
            UTC_time[i] = ReadString.charAt(i);
        }
        UTC_time[i] = '\0';
        promptSerial.print(UTC_time);
        promptSerial.print(";");
        ReadString.remove(0, Pos + 1);

        // validity
        Pos = ReadString.indexOf(',');
        char validity[Pos + 2]; // declare character array UTC_time with a size of the dbit of data
        for (i = 0; i <= Pos - 1; i++)
        { // load charcters into array
            validity[i] = ReadString.charAt(i);
        }
        validity[i] = '\0';
        isValid = (strcmp(validity, "A") == 0);
        promptSerial.print(validity);
        promptSerial.print(";");
        ReadString.remove(0, Pos + 1);

        // latitude
        Pos = ReadString.indexOf(',');
        char latitude[Pos + 3]; // declare character array UTC_time with a size of the dbit of data
        for (i = 0; i <= Pos - 1; i++)
        { // load charcters into array
            latitude[i] = ReadString.charAt(i);
        }
        latitude[i] = '\0';
        // promptSerial.print(latitude);
        // promptSerial.print(" ");
        ReadString.remove(0, Pos + 1);

        // latitude side
        Pos = ReadString.indexOf(',');
        char latitudeSide[Pos + 3]; // declare character array UTC_time with a size of the dbit of data
        for (i = 0; i <= Pos - 1; i++)
        { // load characters into array
            latitudeSide[i] = ReadString.charAt(i);
        }
        latitudeSide[i] = '\0';
        // promptSerial.println(latitudeSide);
        ReadString.remove(0, Pos + 1);

        float LatAsFloat = atof(latitude); // atof converts the char array to a float type
        float LatInDeg;
        if (latitudeSide[0] == char(78))             // N
        {                                            // char(69) is decimal for the letter "N" in ascii chart
            LatInDeg = gps::ConvertData(LatAsFloat); // call the conversion
        }
        if (latitudeSide[0] == char(83))                // S
        {                                               // char(69) is decimal for the letter "S" in ascii chart
            LatInDeg = -(gps::ConvertData(LatAsFloat)); // call the conversion
        }
        if (isValid)
            promptSerial.print(LatInDeg, 15);
        else
            promptSerial.print("0.00");
        promptSerial.print(";");

        // longitude
        Pos = ReadString.indexOf(',');
        char longitude[Pos + 3]; // declare character array UTC_time with a size of the dbit of data
        for (i = 0; i <= Pos - 1; i++)
        { // load charcters into array
            longitude[i] = ReadString.charAt(i);
        }
        longitude[i] = '\0';
        // promptSerial.print(longitude);
        // promptSerial.print(" ");
        ReadString.remove(0, Pos + 1);

        // longitude side
        Pos = ReadString.indexOf(',');
        char longitudeSide[Pos + 3]; // declare character array UTC_time with a size of the dbit of data
        for (i = 0; i <= Pos - 1; i++)
        { // load characters into array
            longitudeSide[i] = ReadString.charAt(i);
        }
        longitudeSide[i] = '\0';
        // promptSerial.println(longitudeSide);
        ReadString.remove(0, Pos + 1);

        // convert to degrees Google can use
        float LongitudeAsFloat = atof(longitude); // atof converts the char array to a float type
        float LongInDeg;
        if (longitudeSide[0] == char(69))              // E
        {                                              // char(69) is decimal for the letter "E" in ascii chart
            LongInDeg = ConvertData(LongitudeAsFloat); // call the conversion
        }
        if (longitudeSide[0] == char(87))                 // W
        {                                                 // char(87) is decimal for the letter "W" in ascii chart
            LongInDeg = -(ConvertData(LongitudeAsFloat)); // call the conversion
        }
        if (isValid)
            promptSerial.print(LongInDeg, 15);
        else
            promptSerial.print("0.00");
        promptSerial.print(";");

        // get the speed
        Pos = ReadString.indexOf(',');
        char speed[Pos + 5]; // declare character array UTC_time with a size of the dbit of data
        for (i = 0; i <= Pos - 1; i++)
        { // load charcters into array
            speed[i] = ReadString.charAt(i);
        }
        speed[i] = '\0';
        promptSerial.print(speed);
        promptSerial.print(";");
        ReadString.remove(0, Pos + 1);

        // get the true course
        Pos = ReadString.indexOf(',');
        char trueCourse[Pos + 5]; // declare character array UTC_time with a size of the dbit of data
        for (i = 0; i <= Pos - 1; i++)
        { // load charcters into array
            trueCourse[i] = ReadString.charAt(i);
        }
        trueCourse[i] = '\0';
        promptSerial.println(trueCourse);
        // promptSerial.println(";");
        ReadString.remove(0, Pos + 1);
    }
    // else
    // {
    //     promptSerial.print("$gps;NO_DATA;\n");
    // }
}

// return true if lat lon are set
bool gps::getLatLon(double *lat, double *lon, HardwareSerial &refSerial)
{
    while (refSerial.available() > 0)
    {
        String readString = refSerial.readStringUntil('\n');
        readString.trim();
        if (readString.startsWith("$"))
        {
            String messageId = getMessageId(readString);
            if (messageId == "GPRMC")
            {
                String tokens[10];
                tokenize(getMainMessage(readString), tokens, 10);
                // Serial.print(" ");
                // Serial.print(tokens[2]);
                // Serial.print(" ");
                if (tokens[2].equals("V"))
                    return false;
                *lat = (double)getDecimalCord(tokens[3], tokens[4].charAt(0));
                // Serial.print((double)getDecimalCord(tokens[3], tokens[4].charAt(0)), 6);
                *lon = (double)getDecimalCord(tokens[5], tokens[6].charAt(0));
                return true;
            }
        }
    }
    return false;
}
String gps::getMessageId(String msg)
{
    int pos = msg.indexOf(',');
    return msg.substring(1, pos);
}
String gps::getMainMessage(String str)
{
    int pos = str.indexOf('*');
    return str.substring(1, pos);
}

float gps::getDecimalCord(String data, char side)
{
    float dataAsFloat = atof(data.c_str());
    if (side == char(78) || side == char(69))
    {
        return ConvertData(dataAsFloat);
    }
    else
    {
        return -(ConvertData(dataAsFloat));
    }
}

// str must be the main message
int gps::tokenize(String str, String tokens[], int size_tokens)
{
    int count = 0;
    while (count < size_tokens)
    {
        int pos = str.indexOf(',');
        tokens[count++] = str.substring(0, pos);
        str.remove(0, pos + 1);
    }
    return count;
}

// Conversion function
float gps::ConvertData(float RawDegrees)
{
    float RawAsFloat = RawDegrees;
    int firstdigits = ((int)RawAsFloat) / 100; // Get the first digits by turning f into an integer, then doing an integer divide by 100;
    float nexttwodigits = RawAsFloat - (float)(firstdigits * 100);
    float Converted = (float)(firstdigits + nexttwodigits / 60.0);
    return Converted;
}
