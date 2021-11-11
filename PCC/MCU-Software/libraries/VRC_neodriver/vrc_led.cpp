#include "vrc_led.hpp"

VRCLED::VRCLED(uint8_t pin, uint8_t num_pixels,neoPixelType t) : Adafruit_NeoPixel(num_pixels,pin,t)
{
    current_color = 255 << 24;
}

void VRCLED::show_temp_color(uint32_t seconds)
{
    //set up the operation
    temp_duration = seconds;
    current_color = temp_color;

    //make it happen
    set_strip_color();
    temp_start = millis();
    temp_running = true;
}

void VRCLED::set_temp_color_target(uint8_t white, uint8_t red, uint8_t green, uint8_t blue)
{
    uint32_t c = 0;

    //format the color
    c = white << 24;
    c |= red << 16;
    c |= green << 8;
    c |= blue;
    //record the color
    temp_color = c;

}

void VRCLED::set_base_color_target(uint8_t white, uint8_t red, uint8_t green, uint8_t blue)
{
    uint32_t c = 0;

    //format the color
    c = white << 24;
    c |= red << 16;
    c |= green << 8;
    c |= blue;
    //record the color
    base_color = c;
}

void VRCLED::set_strip_color()
{
    //update the memory matrix for the strip color
    uint16_t i=0;
    for(i=0; i<numPixels(); i++) 
    {
        setPixelColor(i, current_color);
    }
    
    needs_color_update = true;
}

void VRCLED::set_cycle_to_pixel(uint32_t ms_per, uint16_t target_pixel)
{
    current_cycle = numPixels()/2;
    target_cycle_pixel = target_pixel;
    ms_per_cycle = ms_per;
    needs_color_update = true;
}

void VRCLED::cycle_pixel()
{
    int32_t highlightCW = target_cycle_pixel - current_cycle;
    int32_t highlightCCW = target_cycle_pixel + current_cycle;

    if (highlightCCW>=numPixels()) highlightCCW = highlightCCW - numPixels();
    if (highlightCW<0) highlightCW = highlightCW + numPixels();
    uint32_t c = 0;

             //format the color to white
    c = 255 << 24;
    c |= 0 << 16;
    c |= 0 << 8;
    c |= 0;
    uint16_t i=0;
    for(i=0; i<numPixels(); i++) 
    {
        if (i!=highlightCCW && i!=highlightCW) {
            setPixelColor(i, current_color);
        } else {
             
            setPixelColor(i,c);
        }
    }
    setPixelColor(i,target_cycle_pixel);

    current_cycle--;
    
    needs_color_update = true;
}

void VRCLED::run(void)
{
    //see if were running a temporary color
    if (temp_running)
    {
        //if we are, see if the timer has expired
        if (millis() - temp_start > temp_duration)
        {
            //if the timer has expired, place the base color back onto the strip
            temp_running = false;
            current_color = base_color;
            set_strip_color();
        } 
    }
    else 
    {
        if(current_color != base_color)
        {
            current_color = base_color;
            set_strip_color();
        }
    }
    //see if we need to update the strip color
    if ((millis() - last_strip_show > 10) && needs_color_update)
    {
        show();
        needs_color_update = false;
        last_strip_show = millis();
    }

    if (current_cycle > 0 &&( (millis() - last_cycle_update > ms_per_cycle) ) && needs_color_update)
    {
        cycle_pixel();
        show();
        needs_color_update = false;
        last_cycle_update = millis();
    }


}