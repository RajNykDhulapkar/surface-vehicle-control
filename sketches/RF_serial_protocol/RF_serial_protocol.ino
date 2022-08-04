/*

  Multiple Serial test

  Receives from the main serial port, sends to the others.

  Receives from serial port 1, sends to the main serial (Serial 0).

  This example works only with boards with more than one serial like Arduino Mega, Due, Zero etc.

  The circuit:

  - any serial device attached to Serial port 1

  - Serial Monitor open on Serial port 0

  created 30 Dec 2008

  modified 20 May 2012

  by Tom Igoe & Jed Roach

  modified 27 Nov 2015

  by Arturo Guadalupi

  This example code is in the public domain.

*/

int k = 0;

String readString;

#define msg_modeSelectAutonomous "$mode,0*1F\r\n"
#define msg_modeSelectManual "$mode,1*1E\r\n"
#define msg_setup "$setup,0,0,0,0,0"
#define status_msg "$status,0,0,0,0,0"
#define gps_msg "$gps,15.23,76.98,A,0.00,0.00"
#define imu_msg "$imu,0.00,0.00,15.89,0,0"

String commands[10] = {
    status_msg,
    gps_msg,
    imu_msg,
};

String message_ids[10] = {
    // send
    "setup",
    "mode",
    "control",
    // receive
    "status",
    "gps",
    "imu",
};

void setup()
{
    Serial.begin(9600);
    Serial1.begin(9600);
}

void loop()
{

    Serial1.print(signMessage(commands[k++]));
    if (k > 2)
        k = 0;
    delay(1000);
    // Serial1.write(msg_modeSelectAutonomous);
    // delay(1000);

    // read from port 1, send to port 0:

    if (Serial.available())
    {

        readString = Serial.readStringUntil('\n');
        readString.trim();
        bool isValid = false;
        if (readString.startsWith("$"))
        {
            // char checkSum[5];
            // getCheckSum(readString, checkSum, sizeof(checkSum));
            Serial.print("- ");
            // Serial.print(readString);
            // Serial.print(" , ");
            // Serial.print(checkSum);
            // Serial.print(" , ");
            // Serial.print(getMainMessage(readString).substring(1));
            // Serial.print(" , ");
            // Serial.print(calculateCheckSumInHexadecimal(getMainMessage(readString).substring(1)));
            // isValid = strncmp(checkSum, calculateCheckSumInHexadecimal(getMainMessage(readString).substring(1)), sizeof(checkSum)) == 0;
            // Serial.print(" , ");
            // Serial.print(isValid);
            // Serial.print(" , ");
            Serial.print(readString);
            Serial.print(" , [");
            String mainMessage = getMainMessage(readString);
            Serial.print(mainMessage);
            Serial.print("] , ");
            Serial.print(getCRCCheckSum(readString));
            Serial.print(" , ");
            Serial.print(validateMessage(readString));
            Serial.print(" , ");
            String messageId = getMessageId(readString);
            Serial.print(messageId);
            Serial.println(" | ");

            if (messageId.equals("status"))
            {
                handleStatusMessage(mainMessage);
            }
            else if (messageId.equals("gps"))
            {
                handleGPSMessage(mainMessage);
            }
            else if (messageId.equals("imu"))
            {
                handleIMUMessage(mainMessage);
            }
        }
        // int inByte = Serial.read();

        // Serial.write(inByte);
    }
}

String getMessageId(String msg)
{
    int pos = msg.indexOf(',');
    return msg.substring(1, pos);
}

String signMessage(String msg)
{
    msg.trim();
    uint16_t checkSum = checksumCalculator(msg.substring(1));
    msg.concat("*");
    msg.concat(checkSum);
    msg.concat("\r\n");
    return msg;
}

uint16_t checksumCalculator(String str)
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
    return checksumCalculator(mainMessage) == checkSum;
}

bool handleStatusMessage(String mainMessage)
{
    Serial.println("status message handled");
    return true;
}
bool handleGPSMessage(String mainMessage)
{
    Serial.println("gps message handled");
    return true;
}
bool handleIMUMessage(String mainMessage)
{
    Serial.println("imu message handled");
    return true;
}

// ------------------------------------

char *getCheckSum(String str, char *checkSum, int sizeCheckSum)
{
    int pos = str.indexOf('*');
    int i = 0;
    while (i < sizeCheckSum || str.charAt(i) == '\0')
    {
        // Serial.print(str.charAt(pos + i + 1));
        *(checkSum++) = str.charAt(pos + i + 1);
        i++;
    }
    *checkSum = '\0';
    return checkSum;
}

long int calculateCheckSum(String str)
{
    long int checkSum = 0;
    int i = 0;
    while (str.charAt(i) != '\0')
    {
        checkSum ^= (int)str.charAt(i);
        i++;
    }
    return checkSum;
}

char *calculateCheckSumInHexadecimal(String str)
{
    return decimalToHexadecimal(calculateCheckSum(str));
}

char *decimalToHexadecimal(long int decimalNumber)
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
