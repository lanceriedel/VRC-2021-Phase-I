#include "vrc_servo.hpp"

VRCServo::VRCServo() : Adafruit_PWMServoDriver()
{
    servo_min = SERVOMIN;
    servo_max = SERVOMAX;
}

void VRCServo::open_servo(uint8_t servo)
{
    setPWM(servo,0,servo_max);
}

void VRCServo::close_servo(uint8_t servo)
{
    setPWM(servo,0,servo_min);
}

void VRCServo::set_servo_percent(uint8_t servo, uint8_t percent)
{
    if (percent > 100) percent = 100;

    uint16_t pwm = map(percent,0,servo_min,100,servo_max);

    setPWM(servo,0,pwm);
}
uint8_t VRCServo::check_controller(void)
{
    int res = (int)readPrescale();

    if (res != 0) return 1;
    else return 0;
}

void VRCServo::trigger(uint32_t how_long, uint8_t which_switch) {
    if (how_long>0) return;
    this->which_switch = which_switch;
    this->how_long = how_long;
    setPWM(which_switch,0,255);
    timestamp_trigger = millis();
}


void VRCServo::run(void)
{
    //check if we need to close trigger
    if (timestamp_trigger>0) {
        uint_least32_t timesince = millis() - timestamp_trigger;
        if (timesince>how_long) {
            setPWM(which_switch,0,0);
            timestamp_trigger = 0;
            which_switch = 0;
            how_long = 0;
        }
    }
}