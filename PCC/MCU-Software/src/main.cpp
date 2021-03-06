#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

#include "serial_lib.hpp"
#include "vrc_led.hpp"
#include "vrc_servo.hpp"


//////////////// S E R I A L  I N T E R F A C E ///////////////
uint16_t queue_len = 10;
uint16_t entry_size = sizeof(packet_t);

cppQueue q(entry_size,queue_len,FIFO);

VRCSerialParser serial(Serial,q);
///////////////////////////////////////////////////////////////

///////////////// N E O - P I X E L S /////////////////////////
#define NEO_PIN 5
#define PWR_PIN 10
#define LASER_PIN A4

#define NUM_PIXELS 30

VRCLED strip(NEO_PIN,NUM_PIXELS,NEO_GRB);
VRCLED onboard(8,2,NEO_GRB);

///////////////////////////////////////////////////////////////

/////////////// S E R V O S ///////////////////////////////////
VRCServo servos = VRCServo();
///////////////////////////////////////////////////////////////

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PWR_PIN,OUTPUT);
  pinMode(LASER_PIN,OUTPUT);


  digitalWrite(PWR_PIN,HIGH);

  //////////// N E O - P I X E L  S E T U P ////////////////////
  strip.begin();
  strip.setBrightness(255);
  strip.show();

  onboard.begin();
  onboard.setBrightness(64);
  onboard.show();
  //////////////////////////////////////////////////////////////

  ///////////// S E R V O  S E T U P ///////////////////////////
  servos.begin();
  servos.setOscillatorFrequency(27000000);
  servos.setPWMFreq(SERVO_FREQ);
  //////////////////////////////////////////////////////////////

  Serial.println("init");
}

unsigned long light_on = 0;

void loop() {
  // put your main code here, to run repeatedly:

  serial.poll(); 

  //if image from thermal available -- create a message for it
  uint8_t thermal_reading[512];
  thermal_reading[0]=1;thermal_reading[1]=2;thermal_reading[2]=3;thermal_reading[3]=4;thermal_reading[4]=5;
  uint8_t data_send_bytes[2048] = {0};
  serial.construct_payload(data_send_bytes, SEND_THERMAL_READING, 512, thermal_reading);
  packet_send_t packet_to_send;
  packet_to_send.command=SEND_THERMAL_READING;
  memcpy(packet_to_send.data,(uint8_t*)data_send_bytes,2048);
  serial.set_command(&packet_to_send);

  if(serial.available > 0)
  {
    packet_t message;
    cmd_result res = serial.get_command(&message);
    //Serial.printf("res: %d", res);
    digitalWrite(LED_BUILTIN,HIGH);
    light_on = millis();

    switch(message.command)
    {
      case SET_BASE_COLOR:
      {
        uint8_t white = message.data[0];
        uint8_t red = message.data[1];
        uint8_t green = message.data[2];
        uint8_t blue = message.data[3];
        strip.set_base_color_target(white, red, green, blue);
        //onboard.set_color_target(0,red,green,blue);
      }
      break;
      case SET_TEMP_COLOR:
      {
        uint8_t white = message.data[0];
        uint8_t red = message.data[1];
        uint8_t green = message.data[2];
        uint8_t blue = message.data[3];

        float time = 1.0;

        memcpy(&time, &message.data[4],sizeof(float));

        //Serial.printf("Time %f",time);

        strip.set_temp_color_target(white, red, green, blue);

        uint32_t long_time = (uint32_t) (time * 1000.0);
        strip.show_temp_color(long_time);
        //onboard.set_temp_color_target(0,red,green,blue);
      }
      break;
      case SET_SERVO_OPEN_CLOSE:
      {
        uint8_t which_servo = message.data[0];
        uint8_t value = message.data[1];

        if (value > 127)
        {
          servos.open_servo(which_servo);
          onboard.set_base_color_target(0,255,0,0);
        }
        else
        {
          servos.close_servo(which_servo);
          onboard.set_base_color_target(0,0,255,0);
        }
      }
      break;
      case SET_SERVO_PCT:
      {
        uint8_t which_servo = message.data[0];
        uint8_t percent = message.data[1];

        servos.set_servo_percent(which_servo, percent);
      }
      break;
      case RESET_VRC_PERIPH:
      {
        //digitalWrite(RST_PIN,LOW);
      }
      break;
      case SET_LASER_ON:
      {
        digitalWrite(LASER_PIN,HIGH);
      }
      break;
      case SET_LASER_OFF:
      {
        digitalWrite(LASER_PIN,LOW);
      }
      break;
      case CHECK_SERVO_CONTROLLER:
      {
        //Serial.printf("Checking controller...\n");
        uint8_t res = servos.check_controller();
        //Serial.printf("Res: %d\n",res);
      }
      break;

  
      case SET_PIXEL_CYCLE:
      {
        uint32_t ms_per = message.data[0];
        uint16_t cycle_to_pixel = message.data[1];

        strip.set_cycle_to_pixel(ms_per,cycle_to_pixel );


      }
      break; 

      case SET_TRIGGER_SWITCH:
      {
         uint8_t which_switch = message.data[0];
        uint32_t how_long = message.data[1];
        servos.trigger(how_long, which_switch);
      }
      break;

      case SET_SWITCH_ON:
      {
         uint8_t which_switch = message.data[0];
        servos.onswitch(which_switch);
      }
      break;

      case SET_SWITCH_OFF:
      {
         uint8_t which_switch = message.data[0];
        servos.offswitch(which_switch);
      }
      break;

    }
  }

  if (millis() - light_on > 100)
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
  
  strip.run();
  onboard.run();
  servos.run();
  
}

// void colorWipe(void)
// {
//   uint16_t i;

//   // 'Color wipe' across all pixels
//   for(uint32_t c = 0xFF000000; c; c >>= 8) { // Red, green, blue  wgrb
//     for(i=0;i<strip.numPixels(); i++) {
//       strip.setPixelColor(i, c);
//       strip.show();
//       delay(500);
//     }
//   }
// }