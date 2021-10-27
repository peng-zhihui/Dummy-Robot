/*

  u8x8_d_ls013b7dh03.c
  
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


  The LS013B7DH02 is a simple display and controller
  --> no support for contrast adjustment, flip and power down.
*/

#include "u8x8.h"

#define SWAP8(a) ((((a) & 0x80) >> 7) | (((a) & 0x40) >> 5) | (((a) & 0x20) >> 3) | (((a) & 0x10) >> 1) | (((a) & 0x08) << 1) | (((a) & 0x04) << 3) | (((a) & 0x02) << 5) | (((a) & 0x01) << 7))

#define LS013B7DH03_CMD_UPDATE     (0x01)
#define LS013B7DH03_CMD_ALL_CLEAR  (0x04)
#define LS013B7DH03_VAL_TRAILER    (0x00)

static const u8x8_display_info_t u8x8_ls013b7dh03_128x128_display_info =
{
  /* chip_enable_level = */ 1,
  /* chip_disable_level = */ 0,
  /* post_chip_enable_wait_ns = */ 50,
  /* pre_chip_disable_wait_ns = */ 50,
  /* reset_pulse_width_ms = */ 1,
  /* post_reset_wait_ms = */ 6,
  /* sda_setup_time_ns = */ 227,	/* 227 nsec according to the datasheet */		
  /* sck_pulse_width_ns = */  255,	/* 450 nsec according to the datasheet */
  /* sck_clock_hz = */ 1000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 2,		/* active low, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 100,
  /* write_pulse_width_ns = */ 100,
  /* tile_width = */ 16,
  /* tile_hight = */ 16,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 128,
  /* pixel_height = */ 128
};

uint8_t u8x8_d_ls013b7dh03_128x128(u8x8_t *u8x8, uint8_t msg, U8X8_UNUSED uint8_t arg_int, void *arg_ptr)
{
  uint8_t y, c, i;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ls013b7dh03_128x128_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);

      /* clear screen */
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, SWAP8(LS013B7DH03_CMD_ALL_CLEAR) );
      u8x8_cad_SendCmd(u8x8, LS013B7DH03_VAL_TRAILER);
      u8x8_cad_EndTransfer(u8x8);

      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      /* not available for the ls013b7dh03 */
      break;
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      /* each tile is 8 lines, with the data starting at the left edge */
      y = ((((u8x8_tile_t *)arg_ptr)->y_pos) * 8) + 1;

      c = ((u8x8_tile_t *)arg_ptr)->cnt;
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;

      /* send data mode byte */
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, SWAP8(LS013B7DH03_CMD_UPDATE) );

      /* send 8 lines of 16 bytes (=128 pixels) */
      for( i = 0; i < 8; i++ )
      {
        u8x8_cad_SendCmd(u8x8, SWAP8(y + i) );
        u8x8_cad_SendData(u8x8, c, ptr);
        u8x8_cad_SendCmd(u8x8, LS013B7DH03_VAL_TRAILER);

        ptr += c;
      }

      /* finish with a trailing byte */
      u8x8_cad_SendCmd(u8x8, LS013B7DH03_VAL_TRAILER);
      u8x8_cad_EndTransfer(u8x8);

      break;
    default:
      return 0;
  }
  return 1;
}


static const u8x8_display_info_t u8x8_ls027b7dh01_400x240_display_info =
{
  /* chip_enable_level = */ 1,
  /* chip_disable_level = */ 0,
  /* post_chip_enable_wait_ns = */ 50,
  /* pre_chip_disable_wait_ns = */ 50,
  /* reset_pulse_width_ms = */ 1,
  /* post_reset_wait_ms = */ 6,
  /* sda_setup_time_ns = */ 227,	/* 227 nsec according to the datasheet */		
  /* sck_pulse_width_ns = */  255,	/* 450 nsec according to the datasheet */
  /* sck_clock_hz = */ 1000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 2,		/* active low, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 100,
  /* write_pulse_width_ns = */ 100,
  /* tile_width = */ 50,
  /* tile_hight = */ 30,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 400,
  /* pixel_height = */ 240
};

uint8_t u8x8_d_ls027b7dh01_400x240(u8x8_t *u8x8, uint8_t msg, U8X8_UNUSED uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ls027b7dh01_400x240_display_info);
      break;
    default:
      return u8x8_d_ls013b7dh03_128x128(u8x8, msg, arg_int, arg_ptr);
  }    
  return 1;
}

static const u8x8_display_info_t u8x8_ls027b7dh01_m0_400x240_display_info =
{
  /* chip_enable_level = */ 1,
  /* chip_disable_level = */ 0,
  /* post_chip_enable_wait_ns = */ 50,
  /* pre_chip_disable_wait_ns = */ 50,
  /* reset_pulse_width_ms = */ 1,
  /* post_reset_wait_ms = */ 6,
  /* sda_setup_time_ns = */ 227,	/* 227 nsec according to the datasheet */		
  /* sck_pulse_width_ns = */  255,	/* 450 nsec according to the datasheet */
  /* sck_clock_hz = */ 1000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active low, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 100,
  /* write_pulse_width_ns = */ 100,
  /* tile_width = */ 50,
  /* tile_hight = */ 30,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 400,
  /* pixel_height = */ 240
};

uint8_t u8x8_d_ls027b7dh01_m0_400x240(u8x8_t *u8x8, uint8_t msg, U8X8_UNUSED uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ls027b7dh01_m0_400x240_display_info);
      break;
    default:
      return u8x8_d_ls013b7dh03_128x128(u8x8, msg, arg_int, arg_ptr);
  }    
  return 1;
}


static const u8x8_display_info_t u8x8_ls013b7dh05_144x168_display_info =
{
  /* chip_enable_level = */ 1,
  /* chip_disable_level = */ 0,
  /* post_chip_enable_wait_ns = */ 50,
  /* pre_chip_disable_wait_ns = */ 50,
  /* reset_pulse_width_ms = */ 1,
  /* post_reset_wait_ms = */ 6,
  /* sda_setup_time_ns = */ 227,	/* 227 nsec according to the datasheet */		
  /* sck_pulse_width_ns = */  255,	/* 450 nsec according to the datasheet */
  /* sck_clock_hz = */ 1000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 2,		/* active low, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 100,
  /* write_pulse_width_ns = */ 100,
  /* tile_width = */ 18,
  /* tile_hight = */ 21,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 144,
  /* pixel_height = */ 168
};

uint8_t u8x8_d_ls013b7dh05_144x168(u8x8_t *u8x8, uint8_t msg, U8X8_UNUSED uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ls013b7dh05_144x168_display_info);
      break;
    default:
      return u8x8_d_ls013b7dh03_128x128(u8x8, msg, arg_int, arg_ptr);
  }    
  return 1;
}


