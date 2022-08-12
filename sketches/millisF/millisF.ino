unsigned long currentTime;
unsigned long previousTime1 = 0;
unsigned long previousTime2 = 0;
unsigned long previousTime3 = 0;
int eventInterval1 = 1000;
int eventInterval2 = 200;
int eventInterval3 = 400;
void setup()
{
    Serial.begin(9600);
    pinMode(13, OUTPUT);
}
void loop()
{
    unsigned long currentTime = millis();
    if (currentTime - previousTime1 >= eventInterval1)
    {
        Serial.print("event 1 : ");
        Serial.println(currentTime);
        task1();
        previousTime1 = currentTime;
    }
    if (currentTime - previousTime2 >= eventInterval2)
    {
        Serial.print("event 2 : ");
        Serial.println(currentTime);
        task2();
        previousTime2 = currentTime;
    }
    if (currentTime - previousTime3 >= eventInterval3)
    {
        Serial.print("event 3 : ");
        Serial.println(currentTime);
        task3();
        previousTime3 = currentTime;
    }
}

void task1()
{
    delay(100);
}
void task2()
{
    delay(80);
}
void task3()
{
    delay(30);
}