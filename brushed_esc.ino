#include <Arduino.h>
#include <PinChangeInterrupt.h>

#define INPUT_PIN 8
#define DIRECTION_PIN 16
#define PWM_PIN 10

volatile unsigned long risingStart = 0;
volatile long channelLength = 0;

void setup()
{
    Serial.begin(115200);

    pinMode(INPUT_PIN, INPUT);

    pinMode(DIRECTION_PIN, OUTPUT);
    pinMode(PWM_PIN, OUTPUT);

    digitalWrite(DIRECTION_PIN, LOW);
    analogWrite(PWM_PIN, 0);

    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(INPUT_PIN), onRising0, CHANGE);
}

void onRising0(void)
{
    const uint8_t trigger = getPinChangeInterruptTrigger(digitalPinToPCINT(INPUT_PIN));

    if (trigger == RISING)
    {
        risingStart = micros();
    }
    else if (trigger == FALLING)
    {
        channelLength = micros() - risingStart;
    }
}

long getInput() {
    if (micros() - risingStart > 1000000L || risingStart == 0) {
        return 0;
    } else {
        return channelLength;
    }
}

void loop()
{

    if (getInput() > 1100 && getInput() < 2200) {
        //We have the input, scale the power between 25 and 100%
        analogWrite(PWM_PIN, constrain(map(getInput(), 1100, 1850, 32, 255), 0, 255));
    } else {
        // No input, stop the motors
        analogWrite(PWM_PIN, 0);
    }
}
