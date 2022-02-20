#include "serial_lib.hpp"

VRCSerialParser::VRCSerialParser(Adafruit_USBD_CDC port, cppQueue queue_q, cppQueue q_send_q)
{
    serial_bus = port;

    serial_bus.begin(115200);

    q = queue_q;
    q_send = q_send_q;
}

void VRCSerialParser::poll(void)
{
    Serial.println("poll:");
    send_messages();


    if (serial_bus.available())
    { 
        //See if there are any messages to send and send them first
        read_messages();
    } else {
                Serial.println("Serial bus not available");

    }

        

}

cmd_result VRCSerialParser::send_messages()
{


 //we have fully formed messages waiting, service them.
    if (q_send.getCount()>0)
    {

        packet_send_t packet; 
        q_send.pop(&packet);
        char msg[2048];
        memset(msg,0,sizeof(msg));

        available_send--;

        memcpy(&msg,packet.data,2048);
        size_t written = serial_bus.write(msg, 2048);

        if (written==sizeof(msg)) {

            return SUCCESS;
        }
        else    
            return DROPPED; 
    }
    else
    {
        Serial.println("q empty");

        return QUEUE_EMPTY;
    }
}

cmd_result VRCSerialParser::read_messages() {
    uint8_t byte = serial_bus.read();
        last_byte_received = millis();

        switch (state)
        {
            case IDLE:
            {
                if (byte == '$')
                {
                    state = START;
                } 
                write_index = 0;
            }
            break;
            case START:
            {
                if (byte == 'P')
                {
                    state = CC;
                } 
                else 
                {
                    state = IDLE;
                }
            }
            break;
            case CC:
            {
                if (byte == '<')
                {
                    state = DIRECTION; //incoming direction
                } 
                else 
                {
                    state = IDLE;
                }
            }
            break;
            case DIRECTION:
            {
                length_bytes[0] = byte;
                state = LENGTH_HI;
            }
            break;
            case LENGTH_HI:
            {
                length_bytes[1] = byte;
                length = length_bytes[0]<<8;
                length |= length_bytes[1];
                state = LENGTH_LOW;
                
            }
            break;
            case LENGTH_LOW:
            {
                memset(data_bytes,0,sizeof(data_bytes));
                data_bytes[write_index] = byte;
                write_index++;
                if (length == 1)
                {
                    state = CRC;
                }
                else state = DATA;
            }
            break;
            case DATA:
            {
                if (write_index < length) //read up to length bytes
                {
                    data_bytes[write_index] = byte;
                    write_index++;

                }
                if (write_index == length) //were done reading in data bytes
                {
                    state = CRC;
                }
            }
            break;
            case CRC:
            {
                crc_bytes[0] = byte;

                bool crc_passed = false;
                bool msg_parsed = false;
                //check the crc for the message
                uint8_t crc_buf[3 + sizeof(length_bytes) + length ];
                memcpy(crc_buf,(uint8_t*)incoming_preamble,3);//put the preamble in
                memcpy(crc_buf+3,length_bytes,sizeof(length_bytes));
                memcpy(crc_buf+3+sizeof(length_bytes),data_bytes,length);

                uint8_t crc = calc_crc(crc_buf,3 + sizeof(length_bytes) + length );

                if (crc == crc_bytes[0])
                {
                    crc_passed = true;
                }
                else 
                {
                    //Serial.println("CRC FAILED...");
                    //Serial.printf("CRC(MSG): %d CRC(CALCED): %d\n", crc_bytes[0], crc);
                }
                
                //try to parse message into message object.
                packet_t msg;

                if (data_bytes[0] >= SET_SERVO_OPEN_CLOSE && data_bytes[0] < COMMAND_END) //this is a valid command
                {
                    msg.command = data_bytes[0];              //copy command into msg struct
                    memcpy(&msg.data,&data_bytes[1],length-1); //copy from command byte to end of msg into msg.data
                    msg_parsed = true;
                }

                //set message available flag
                if (crc_passed && msg_parsed)
                {
                    if (q.getCount() < 10)
                    {
                        q.push(&msg);
                        available++;
                    }
                    else 
                    {
                        messages_dropped++;
                    }
                    
                }
                else
                {
                   messages_dropped++;
                }
                state = IDLE;
            }
            break;
            default:
            break;
        }
    

    if (millis() - last_byte_received > 1000)
    {
        if (state != IDLE)
        {
            state = IDLE;
            messages_dropped++;
        }
        
    }
    return SUCCESS;
}

cmd_result VRCSerialParser::set_command(packet_send_t* msg) {
  //we have fully formed messages waiting, service them.
    Serial.println("set Command");

    if (q_send.getCount()<10)
    {
        Serial.println("Added Command");
        bool suc = q_send.push(msg);
        Serial.print("set_command:[");  Serial.print(suc); Serial.print("] "); Serial.println(q_send.getCount());

        available_send++;
        return SUCCESS;
    }
    else
    {
        messages_send_dropped++;
        return DROPPED;
    }
    
}

cmd_result VRCSerialParser::get_command(packet_t* msg)
{
    //we have fully formed messages waiting, service them.
    if (q.getCount())
    {
        packet_t temp; 

        q.pop(&temp);
        available--;

        memcpy(msg,&temp,sizeof(packet_t));
        return SUCCESS;
    }
    else
    {
        return QUEUE_EMPTY;
    }
    
}


void VRCSerialParser::construct_payload(uint8_t* data_send_bytes_, int codecommand, int sizedata, uint8_t* data) {

        //# [$][P][>][CODE][LENGTH-HI][LENGTH-LOW][DATA]    -- skipping CRC for now [CRC]
        uint8_t* ptr;
        ptr=data_send_bytes_;
        

        //SP>
        int offset = 0;
        memcpy(ptr,(uint8_t*)outgoing_preamble,3);
        offset+=3;
        ptr+=offset;

        //write the code
        *ptr=codecommand;
        ptr+=1;

        //Write the size
        uint8_t LSB = sizedata;
        uint8_t MSB = sizedata >> 8;
        *ptr=LSB;
        ptr+=1;
        *ptr=MSB;
        ptr+=1;
        memcpy(ptr,(uint8_t*)data,sizedata);
  
        }

uint8_t VRCSerialParser::crc8_dvb_s2(uint8_t crc, unsigned char a)
{
    crc ^= a;
    for (int ii = 0; ii < 8; ++ii) {
        if (crc & 0x80) {
            crc = (crc << 1) ^ 0xD5;
        } else {
            crc = crc << 1;
        }
    }
    return crc;
}

uint8_t VRCSerialParser::calc_crc(uint8_t* buffer, uint16_t length)
{
  int i = 0;
  uint8_t crc = 0;

  for (i=0;i<length;i++)
  {
    crc = crc8_dvb_s2(crc,(unsigned char) buffer[i]);
  }
  return crc;
}