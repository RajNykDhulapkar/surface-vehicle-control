#include <Servo.h>

Servo ESC;

int potValue = 1500;
byte escPin = 9;

void setup()
{
    Serial.begin(9600);
    ESC.attach(escPin); // (pin, min pulse width, max pulse width in microseconds)
    ESC.writeMicroseconds(1500);
    delay(7000);
}
void loop()
{
    // potValue = analogRead(A0);                     // reads the value of the potentiometer (value between 0 and 1023)
    // potValue = map(potValue, 0, 1023, 1100, 1900); // scale it to use it with the servo library (value between 0 and 180)
    while (Serial.available() > 0)
    {
        char c = Serial.read();
        if (c == 'u')
        {
            potValue += 10;
        }
        else if (c == 'd')
        {
            potValue -= 10;
        }
        constrain(potValue, 1100, 1900);
    }
    Serial.println(potValue);
    ESC.writeMicroseconds(potValue);

    // Send the signal to the ESC
}