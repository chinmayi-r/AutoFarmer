#include <Arduino.h>
#include <math.h>

#define enc 2
#define enc2 3
#define enc3 18
#define enc4 19

const int onerev = 1890;
const int wheels_distance = 630;
volatile long long enc_val = 0;
volatile long long enc_val2 = 0;
volatile long long enc_val3 = 0;
volatile long long enc_val4 = 0;

void updateencoder()
{
    enc_val++;
}

void updateencoder2()
{
    enc_val2++;
}

void updateencoder3()
{
    enc_val3++;
}

void updateencoder4()
{
    enc_val4++;
}

void move(long long mm)
{
    bool encoders[] = {true, true, true, true};
    enc_val = 0;
    enc_val2 = 0;
    enc_val3 = 0;
    enc_val4 = 0;

    long long required = (long long)(5.72 * (float)(max(mm, mm * -1)));

    if(mm > 0)
    {
        digitalWrite(22, HIGH);
        digitalWrite(23, LOW);
        digitalWrite(24, HIGH);
        digitalWrite(25, LOW);
        digitalWrite(26, HIGH);
        digitalWrite(27, LOW);
        digitalWrite(28, HIGH);
        digitalWrite(29, LOW);
    }
    else
    {   
        digitalWrite(22, LOW);
        digitalWrite(23, HIGH);
        digitalWrite(24, LOW);
        digitalWrite(25, HIGH);
        digitalWrite(26, LOW);
        digitalWrite(27, HIGH);
        digitalWrite(28, LOW);
        digitalWrite(29, HIGH);
    }

    while(encoders[0] || encoders[1] || encoders[2] || encoders[3])
    {
        if(enc_val >= required && encoders[0])
        {
            digitalWrite(22, LOW);
            digitalWrite(23, LOW);
            encoders[0] = false;
        }
        if(enc_val2 >= required && encoders[1])
        {
            digitalWrite(24, LOW);
            digitalWrite(25, LOW);
            encoders[1] = false;
        }
        if(enc_val3 >= required && encoders[2])
        {
            digitalWrite(26, LOW);
            digitalWrite(27, LOW);
            encoders[2] = false;
        }
        if(enc_val4 >= required && encoders[3])
        {
            digitalWrite(28, LOW);
            digitalWrite(29, LOW);
            encoders[3] = false;
        }
    }

    digitalWrite(22, LOW);
    digitalWrite(23, LOW);
    digitalWrite(24, LOW);
    digitalWrite(25, LOW);
    digitalWrite(26, LOW);
    digitalWrite(27, LOW);
    digitalWrite(28, LOW);
    digitalWrite(29, LOW);
}

void turn(int degrees)
{
    bool encoders[] = {true, true, true, true};
    enc_val = 0;
    enc_val2 = 0;
    enc_val3 = 0;
    enc_val4 = 0;

    long long required = (long long)(5.72 * (float)((((0.01745329251 * (float)(wheels_distance)) * ((float)(max(degrees * -1, degrees)))) / 2)));
    
    if(degrees > 0)
    {
        digitalWrite(22, HIGH);
        digitalWrite(23, LOW);
        digitalWrite(24, HIGH);
        digitalWrite(25, LOW);
        digitalWrite(26, LOW);
        digitalWrite(27, HIGH);
        digitalWrite(28, LOW);
        digitalWrite(29, HIGH);
    }
    else
    {
        digitalWrite(22, LOW);
        digitalWrite(23, HIGH);
        digitalWrite(24, LOW);
        digitalWrite(25, HIGH);
        digitalWrite(26, HIGH);
        digitalWrite(27, LOW);
        digitalWrite(28, HIGH);
        digitalWrite(29, LOW);
    }

    while(encoders[0] || encoders[1] || encoders[2] || encoders[3])
    {
        if(enc_val >= required && encoders[0])
        {
            digitalWrite(22, LOW);
            digitalWrite(23, LOW);
            encoders[0] = false;
        }
        if(enc_val2 >= required && encoders[1])
        {
            digitalWrite(24, LOW);
            digitalWrite(25, LOW);
            encoders[1] = false;
        }
        if(enc_val3 >= required && encoders[2])
        {
            digitalWrite(26, LOW);
            digitalWrite(27, LOW);
            encoders[2] = false;
        }
        if(enc_val4 >= required && encoders[3])
        {
            digitalWrite(28, LOW);
            digitalWrite(29, LOW);
            encoders[3] = false;
        }
    }

    digitalWrite(22, LOW);
    digitalWrite(23, LOW);
    digitalWrite(24, LOW);
    digitalWrite(25, LOW);
    digitalWrite(26, LOW);
    digitalWrite(27, LOW);
    digitalWrite(28, LOW);
    digitalWrite(29, LOW);
}

void go_to_coordinates(long long x, long long y)
{
    turn((int)(atan((float)((float)(x) / (float)(y))) * 180.00/3.14));
    delay(200);
    if(y < 0)
        move(-1 * (sqrt((x * x) + (y * y))));
    else
        move(sqrt((x * x) + (y * y)));
    delay(200);
    turn(-1 * ((int)(atan((float)((float)(x) / (float)(y))) * 180.00/3.14)));
}

void setup()
{
    Serial.begin(9600);

    pinMode(enc, INPUT_PULLUP);
    pinMode(enc2, INPUT_PULLUP);
    pinMode(enc3, INPUT_PULLUP);
    pinMode(enc4, INPUT_PULLUP);
    
    pinMode(22, OUTPUT);
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);
    pinMode(25, OUTPUT);
    pinMode(26, OUTPUT);
    pinMode(27, OUTPUT);
    pinMode(28, OUTPUT);
    pinMode(29, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(enc), updateencoder, FALLING);
    attachInterrupt(digitalPinToInterrupt(enc2), updateencoder2, FALLING);
    attachInterrupt(digitalPinToInterrupt(enc3), updateencoder3, FALLING);
    attachInterrupt(digitalPinToInterrupt(enc4), updateencoder4, FALLING);
}

void loop()
{
    while(Serial.available() == 0){}
    long long x = Serial.parseInt();
    long long y = Serial.parseInt();
    go_to_coordinates(x, y);
    delay(10000);
}