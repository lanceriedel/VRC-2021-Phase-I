#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define SERVOMIN  150 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  425 // This is the 'maximum' pulse length count (out of 4096)
#define USMIN  600 // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX  2400 // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

class VRCServo : public Adafruit_PWMServoDriver
{
    public:
    VRCServo();

    void open_servo(uint8_t servo);
    void close_servo(uint8_t servo);
    void set_servo_percent(uint8_t servo, uint8_t percent);
    void set_servo_min();
    void set_servo_max();
    uint8_t check_controller(void);
    void trigger(uint32_t how_long, uint8_t which_switch);
    void onswitch(uint8_t which_switch_);
    void offswitch(uint8_t which_switch_);
    void writeToSwitch(uint8_t num, uint8_t val);

    void run(void);


    private:
    uint16_t servo_min;
    uint16_t servo_max;
    unsigned long how_long = 0;
    unsigned long timestamp_trigger = 0;
    unsigned long timestamp_last = 0;

    uint8_t which_switch = 32;

};