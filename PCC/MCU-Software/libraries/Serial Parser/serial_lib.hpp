#include <Arduino.h>
#include "cppQueue.h"

typedef enum 
{
    IDLE,
    START,
    CC,
    DIRECTION,
    LENGTH_HI,
    LENGTH_LOW,
    DATA,
    CRC
}uart_states;


typedef enum
{
    SET_SERVO_OPEN_CLOSE,
    SET_SERVO_MIN,
    SET_SERVO_MAX,
    SET_SERVO_PCT,
    SET_BASE_COLOR,
    SET_TEMP_COLOR,
    SET_PIXEL_CYCLE,
    SET_TRIGGER_SWITCH,
    SET_SWITCH_ON,
    SET_SWITCH_OFF,
    SET_LASER_ON,
    SET_LASER_OFF,
    RESET_VRC_PERIPH,
    CHECK_SERVO_CONTROLLER,
    SEND_THERMAL_READING,
    COMMAND_END
} commands;


typedef struct 
{
  uint8_t command;
  uint8_t data[128];
} packet_t;

typedef struct 
{
  uint8_t command;
  uint8_t data[2048];
} packet_send_t;

typedef enum
{
    QUEUE_EMPTY,
    SUCCESS,
    DROPPED
} cmd_result;

static char* outgoing_preamble = "$P>";//towards arduino
static char* incoming_preamble = "$P<";//towards jetson



class VRCSerialParser 
{
  public:
    VRCSerialParser(Adafruit_USBD_CDC port, cppQueue queue_q);
    void poll(void);
    uart_states get_state(void);
    cmd_result get_command(packet_t* msg);
    cmd_result set_command(packet_send_t* msg);
    void construct_payload(uint8_t* data_send_bytes_, int codecommand, int sizedata, uint8_t* data);

    uint32_t available = 0;
    uint32_t available_send = 0;

  private:
    Adafruit_USBD_CDC serial_bus;  //arduino serial object
    uart_states state;
    unsigned long last_byte_received = 0;

    //temp space for receiving sm to work with...
    uint8_t length_bytes[2] = {0};
    uint16_t length = 0;
    uint8_t data_bytes[512] = {0};
    uint16_t write_index = 0;
    uint8_t crc_bytes[2] = {0};
    uint8_t messages_available = 0;

    //Space for sending sm to work with
    char data_send_bytes[4096] = {0};

    
    uint8_t crc8_dvb_s2(uint8_t crc, unsigned char a);
    uint8_t calc_crc(uint8_t* buffer, uint16_t length);

    uint32_t messages_dropped=0;
    uint32_t messages_send_dropped=0;
    cmd_result send_messages();
    cmd_result read_messages();



    cppQueue q;
    cppQueue q_send;


    
};


