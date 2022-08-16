

void setup()
{
    Serial3.begin(9600);
    Serial.begin(9600);
    Serial.println("start");
}
void loop()
{
    while (Serial.available() > 0)
    {
        char c = Serial.read();
        // Serial1.write(c);
        Serial3.write(c);
    }
}