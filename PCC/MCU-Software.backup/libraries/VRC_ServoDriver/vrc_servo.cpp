#include "vrc_servo.hpp"

VRCServo::VRCServo(const uint8_t addr, TwoWire &i2c) : Adafruit_PWMServoDriver (addr, &_i2c )

{
    servo_min = SERVOMIN;
    servo_max = SERVOMAX;
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(9, OUTPUT);

    digitalWrite(5, HIGH);
    

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

void VRCServo::writeToSwitch(uint8_t num, uint8_t val) {
    digitalWrite(5, HIGH);
}
void VRCServo::trigger(uint32_t how_long_, uint8_t which_switch_) {
    if (this->how_long>0) return;
    this->which_switch = which_switch_;
    this->how_long = how_long_*10;
    //setPWM(which_switch, 0, 254);
    //writeToSwitch(which_switch,250);
    digitalWrite(which_switch, HIGH);
    timestamp_trigger = millis();
}

void VRCServo::onswitch(uint8_t which_switch_) {
    digitalWrite(which_switch_, HIGH);
}

void VRCServo::offswitch(uint8_t which_switch_) {
    digitalWrite(which_switch_, LOW);
}

void VRCServo::run(void)
{
    //check if we need to close trigger
    uint32_t timesince_last = millis() - timestamp_last;
 

    if (timestamp_trigger>0) {
        uint32_t timesince = millis() - timestamp_trigger;
        if (timesince>how_long) {
            digitalWrite(which_switch, LOW);

           // writeToSwitch(which_switch,1);
           //setPWM(which_switch,0,0);
            timestamp_trigger = 0;
            which_switch = 0;
            how_long = 0;
        }
    }
}