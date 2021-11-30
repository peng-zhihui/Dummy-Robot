/*

  u8x8_d_ssd1306_128x80_noname.c

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
  
*/


#include "u8x8.h"


/* more or less generic setup of all these small OLEDs */
static const uint8_t u8x8_d_ssd1306_128x80_noname_init_seq[] = {

    U8X8_START_TRANSFER(),                /* enable chip, delay is part of the transfer start */

    U8X8_C(0xAE), /*display off*/
    U8X8_C(0x00), /*set lower column address*/
    U8X8_C(0x10), /*set higher column address*/
    U8X8_C(0x20), /* Set Memory addressing mode (0x20/0x21) */
    U8X8_C(0x81), /*contract control*/
    U8X8_C(0x6f), /*b0*/
    U8X8_C(0xA0), /*set segment remap*/
    U8X8_C(0xC0), /*Com scan direction*/
    U8X8_C(0xA4), /*Disable Entire Display On (0xA4/0xA5)*/
    U8X8_C(0xA6), /*normal / reverse*/
    U8X8_C(0xD5), /*set osc division*/
    U8X8_C(0x91),
    U8X8_C(0xD9), /*set pre-charge period*/
    U8X8_C(0x22),
    U8X8_C(0xdb), /*set vcomh*/
    U8X8_C(0x3f),
    U8X8_C(0xA8), /*multiplex ratio*/
    U8X8_C(0x4F), /*duty = 1/80*/
    U8X8_C(0xD3), /*set display offset*/
    U8X8_C(0x68), /*18*/
    U8X8_C(0xdc), /*Set Display Start Line*/
    U8X8_C(0x00),
    U8X8_C(0xad), /*set charge pump enable*/
    U8X8_C(0x8a), /*Set DC-DC enable (a=0:disable; a=1:enable) */


    U8X8_END_TRANSFER(),                /* disable chip */
    U8X8_END()                        /* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_128x80_noname_powersave0_seq[] = {
    U8X8_START_TRANSFER(),                /* enable chip, delay is part of the transfer start */
    U8X8_C(0x0af),                        /* display on */
    U8X8_END_TRANSFER(),                /* disable chip */
    U8X8_END()                        /* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_128x80_noname_powersave1_seq[] = {
    U8X8_START_TRANSFER(),                /* enable chip, delay is part of the transfer start */
    U8X8_C(0x0ae),                        /* display off */
    U8X8_END_TRANSFER(),                /* disable chip */
    U8X8_END()                        /* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_128x80_noname_flip0_seq[] = {
    U8X8_START_TRANSFER(),                /* enable chip, delay is part of the transfer start */
    U8X8_C(0x0a1),                /* segment remap a0/a1*/
    U8X8_C(0x0c8),                /* c0: scan dir normal, c8: reverse */
    U8X8_END_TRANSFER(),                /* disable chip */
    U8X8_END()                        /* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_128x80_noname_flip1_seq[] = {
    U8X8_START_TRANSFER(),                /* enable chip, delay is part of the transfer start */
    U8X8_C(0x0a0),                /* segment remap a0/a1*/
    U8X8_C(0x0c0),                /* c0: scan dir normal, c8: reverse */
    U8X8_END_TRANSFER(),                /* disable chip */
    U8X8_END()                        /* end of sequence */
};


static uint8_t u8x8_d_ssd1306_sh1106_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    uint8_t x, c;
    uint8_t *ptr;
    switch (msg)
    {
        /* handled by the calling function
        case U8X8_MSG_DISPLAY_SETUP_MEMORY:
          u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1306_128x80_noname_display_info);
          break;
        */
        /* handled by the calling function
        case U8X8_MSG_DISPLAY_INIT:
          u8x8_d_helper_display_init(u8x8);
          u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x80_noname_init_seq);
          break;
        */
        case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
            if (arg_int == 0)
                u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x80_noname_powersave0_seq);
            else
                u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x80_noname_powersave1_seq);
            break;
        case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
            if (arg_int == 0)
            {
                u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x80_noname_flip0_seq);
                u8x8->x_offset = u8x8->display_info->default_x_offset;
            } else
            {
                u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x80_noname_flip1_seq);
                u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
            }
            break;
#ifdef U8X8_WITH_SET_CONTRAST
        case U8X8_MSG_DISPLAY_SET_CONTRAST:
            u8x8_cad_StartTransfer(u8x8);
            u8x8_cad_SendCmd(u8x8, 0x081);
            u8x8_cad_SendArg(u8x8, arg_int);    /* ssd1306 has range from 0 to 255 */
            u8x8_cad_EndTransfer(u8x8);
            break;
#endif
        case U8X8_MSG_DISPLAY_DRAW_TILE:
            u8x8_cad_StartTransfer(u8x8);
            x = ((u8x8_tile_t *) arg_ptr)->x_pos;
            x *= 8;
            x += u8x8->x_offset;

            u8x8_cad_SendCmd(u8x8, 0x040);    /* set line offset to 0 */

            u8x8_cad_SendCmd(u8x8, 0x010 | (x >> 4));
            u8x8_cad_SendArg(u8x8, 0x000 | ((x & 15)));                    /* probably wrong, should be SendCmd */
            u8x8_cad_SendArg(u8x8,
                             0x0b0 | (((u8x8_tile_t *) arg_ptr)->y_pos));    /* probably wrong, should be SendCmd */


            do
            {
                c = ((u8x8_tile_t *) arg_ptr)->cnt;
                ptr = ((u8x8_tile_t *) arg_ptr)->tile_ptr;
                u8x8_cad_SendData(u8x8, c * 8, ptr);    /* note: SendData can not handle more than 255 bytes */
                /*
                do
                {
                  u8x8_cad_SendData(u8x8, 8, ptr);
                  ptr += 8;
                  c--;
                } while( c > 0 );
                */
                arg_int--;
            } while (arg_int > 0);

            u8x8_cad_EndTransfer(u8x8);
            break;
        default:
            return 0;
    }
    return 1;
}


static const u8x8_display_info_t u8x8_ssd1306_128x80_noname_display_info =
    {
        /* chip_enable_level = */ 0,
        /* chip_disable_level = */ 1,

        /* post_chip_enable_wait_ns = */ 20,
        /* pre_chip_disable_wait_ns = */ 10,
        /* reset_pulse_width_ms = */ 100,    /* SSD1306: 3 us */
        /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
        /* sda_setup_time_ns = */ 50,        /* SSD1306: 15ns, but cycle time is 100ns, so use 100/2 */
        /* sck_pulse_width_ns = */
                                  50,    /* SSD1306: 20ns, but cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
        /* sck_clock_hz = */
                                  8000000UL,    /* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
        /* spi_mode = */ 0,        /* active high, rising edge */
        /* i2c_bus_clock_100kHz = */ 4,
        /* data_setup_time_ns = */ 40,
        /* write_pulse_width_ns = */ 150,    /* SSD1306: cycle time is 300ns, so use 300/2 = 150 */
        /* tile_width = */ 10,
        /* tile_hight = */ 16,
        /* default_x_offset = */ 0,
        /* flipmode_x_offset = */ 0,
        /* pixel_width = */ 80,
        /* pixel_height = */ 128
    };

uint8_t u8x8_d_ssd1306_128x80_noname(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{

    if (u8x8_d_ssd1306_sh1106_generic(u8x8, msg, arg_int, arg_ptr) != 0)
        return 1;

    switch (msg)
    {
        case U8X8_MSG_DISPLAY_INIT:
            u8x8_d_helper_display_init(u8x8);
            u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x80_noname_init_seq);
            break;
        case U8X8_MSG_DISPLAY_SETUP_MEMORY:
            u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1306_128x80_noname_display_info);
            break;
        default:
            return 0;
    }
    return 1;
}

