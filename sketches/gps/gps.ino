void setup()
{
    Serial.begin(9600);
    Serial1.begin(9600);
}

void loop()
{
    while (Serial1.available() > 0)
    {
        char c = Serial1.read();
        Serial.print(c);
    }
}