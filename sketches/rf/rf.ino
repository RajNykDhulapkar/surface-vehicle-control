#include <Servo.h>

void setup()
{
    Serial1.begin(9600);
    Serial.begin(9600);
    Serial.println("start");
}
void loop()
{
    while (Serial1.available() > 0)
    {
        char c = Serial1.read();
        // Serial1.write(c);
        Serial.write(c);
    }
}