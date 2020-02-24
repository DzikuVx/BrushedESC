#include <Arduino.h>
#include <PinChangeInterrupt.h>

#define INPUT_PIN 8
#define DIRECTION_PIN 16
#define PWM_PIN 10

volatile unsigned long risingStart = 0;
volatile long channelLength = 0;

#define PWM_LENGTH_MIN 500
#define PWM_LENGTH_MAX 2500

#define PWM_INPUT_MIN 1000
#define PWM_INPUT_MAX 2000
#define PWM_INPUT_NEUTRAL 1460
#define PWM_FORWARD_THRESHOLD 1490
#define PWM_BACKWARD_THRESHOLD 1430
#define INITIAL_POWER_LEVEL 24

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
    if (micros() - risingStart > 25000 || risingStart == 0) {
        return 0;
    } else {
        return channelLength;
    }
}

void motorStop(void) {
    analogWrite(PWM_PIN, 0);
}

void loop()
{

    const int input = getInput();

    if (input > PWM_LENGTH_MIN && input < PWM_LENGTH_MAX) {
        //We have the input, scale the power between 25 and 100%
        // analogWrite(PWM_PIN, constrain(map(getInput(), 1100, 1850, 32, 255), 0, 255));

        const int inputConstrained = constrain(input, PWM_INPUT_MIN, PWM_INPUT_MAX);
        int outputValue = 0;

        if (inputConstrained > PWM_FORWARD_THRESHOLD) {
            /*
             * Forward direction
             */
            outputValue = map(inputConstrained, PWM_FORWARD_THRESHOLD, PWM_INPUT_MAX, INITIAL_POWER_LEVEL, 255);

            digitalWrite(DIRECTION_PIN, LOW);
        } else if (inputConstrained < PWM_BACKWARD_THRESHOLD) {
            /*
             * Backward direction
             * Since in reverse mode DIRECTION_PIN is pulled up, PWM value on the output has to be reversed: 0 means full duty cycle
             */
            outputValue = map(inputConstrained, PWM_INPUT_MIN, PWM_BACKWARD_THRESHOLD, 0, 255 - INITIAL_POWER_LEVEL);

            digitalWrite(DIRECTION_PIN, HIGH);
        } else {
            /*
             * In deadband
             */
            outputValue = 0;
            digitalWrite(DIRECTION_PIN, LOW);
        }

        analogWrite(PWM_PIN, constrain(outputValue, 0, 255));
    } else {
        // No input, stop the motors
        motorStop();
    }

    delay(10);
}
