/*

  u8x8_d_t6963.c
  
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


  The t6963 controller does not support hardware graphics flip.
  Contrast adjustment is done by an external resistor --> no support for contrast adjustment
  
  
*/
#include "u8x8.h"



static const uint8_t u8x8_d_t6963_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x098),                            /* mode register: Display Mode, Graphics on, Text off, Cursor off */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_t6963_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x090),                             /* All Off */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_t6963_common(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t c, i;
  uint16_t y;
  uint8_t *ptr;
  switch(msg)
  {
    /* U8X8_MSG_DISPLAY_SETUP_MEMORY is handled by the calling function */
    /*
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      break;
    case U8X8_MSG_DISPLAY_INIT:
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_t6963_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_t6963_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      y = (((u8x8_tile_t *)arg_ptr)->y_pos);
      y*=8;
      y*= u8x8->display_info->tile_width;
      /* x = ((u8x8_tile_t *)arg_ptr)->x_pos; x is ignored... no u8x8 support */
      //u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, 200, NULL);	/* extra dely required */
      u8x8_cad_StartTransfer(u8x8);
      //u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, 200, NULL);	/* extra dely required */
      /* 
	Tile structure is reused here for the t6963, however u8x8 is not supported 
	tile_ptr points to data which has cnt*8 bytes (same as SSD1306 tiles)
	Buffer is expected to have 8 lines of code fitting to the t6963 internal memory
	"cnt" includes the number of horizontal bytes. width is equal to cnt*8
	
	TODO: Consider arg_int, however arg_int is not used by u8g2
      */
      c = ((u8x8_tile_t *)arg_ptr)->cnt;	/* number of tiles */
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;	/* data ptr to the tiles */
      for( i = 0; i < 8; i++ )
      {
	u8x8_cad_SendArg(u8x8, y&255);
	u8x8_cad_SendArg(u8x8, y>>8);
	u8x8_cad_SendCmd(u8x8, 0x024 );	/* set adr */
	u8x8_cad_SendCmd(u8x8, 0x0b0 );	/* auto write start */
	
	
	//c = ((u8x8_tile_t *)arg_ptr)->cnt;	/* number of tiles */
	u8x8_cad_SendData(u8x8, c, ptr);	/* note: SendData can not handle more than 255 bytes, send one line of data */
	
	u8x8_cad_SendCmd(u8x8, 0x0b2 );	/* auto write reset */
	ptr += u8x8->display_info->tile_width;
	y += u8x8->display_info->tile_width;
      }

      u8x8_cad_EndTransfer(u8x8);
      //u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, 200, NULL);	/* extra dely required */

      break;
    default:
      return 0;
  }
  return 1;
}

/*=============================================*/


static const u8x8_display_info_t u8x8_t6963_240x128_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 110,	/* T6963 Datasheet p30 */
  /* pre_chip_disable_wait_ns = */ 100,	/* T6963 Datasheet p30 */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 6, 
  /* sda_setup_time_ns = */ 20,		
  /* sck_pulse_width_ns = */  140,	
  /* sck_clock_hz = */ 1000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 80,
  /* write_pulse_width_ns = */ 80,
  /* tile_width = */ 30,
  /* tile_hight = */ 16,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 240,
  /* pixel_height = */ 128
};

/* 240x128 */
static const uint8_t u8x8_d_t6963_240x128_init_seq[] = {
  U8X8_DLY(100),
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_DLY(100),
  
  U8X8_AAC(0x00,0x00,0x021),	/* low, high, set cursor pos */
  U8X8_AAC(0x00,0x00,0x022),	/* low, high, set offset */
  U8X8_AAC(0x00,0x00,0x040),	/* low, high, set text home */
  U8X8_AAC(240/8,0x00,0x041),	/* low, high, set text columns */
  U8X8_AAC(0x00,0x00,0x042),	/* low, high, graphics home */  
  U8X8_AAC(240/8,0x00,0x043),	/* low, high, graphics columns */
  U8X8_DLY(2),					/* delay 2ms */
  // mode set
  // 0x080: Internal CG, OR Mode
  // 0x081: Internal CG, EXOR Mode
  // 0x083: Internal CG, AND Mode
  // 0x088: External CG, OR Mode
  // 0x089: External CG, EXOR Mode
  // 0x08B: External CG, AND Mode
  U8X8_C(0x080),            			/* mode register: OR Mode, Internal Character Mode */
  // display mode
  // 0x090: Display off
  // 0x094: Graphic off, text on, cursor off, blink off
  // 0x096: Graphic off, text on, cursor on, blink off
  // 0x097: Graphic off, text on, cursor on, blink on
  // 0x098: Graphic on, text off, cursor off, blink off
  // 0x09a: Graphic on, text off, cursor on, blink off
  // ...
  // 0x09c: Graphic on, text on, cursor off, blink off
  // 0x09f: Graphic on, text on, cursor on, blink on
  U8X8_C(0x090),                             /* All Off */
  U8X8_AAC(0x00,0x00,0x024),	/* low, high, set adr pointer */
  
  U8X8_DLY(100),
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_DLY(100),
};

uint8_t u8x8_d_t6963_240x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_t6963_240x128_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_t6963_240x128_init_seq);
      break;
    default:
      return u8x8_d_t6963_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}



/*=============================================*/

static const u8x8_display_info_t u8x8_t6963_240x64_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 110,	/* T6963 Datasheet p30 */
  /* pre_chip_disable_wait_ns = */ 100,	/* T6963 Datasheet p30 */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 6, 
  /* sda_setup_time_ns = */ 20,		
  /* sck_pulse_width_ns = */  140,	
  /* sck_clock_hz = */ 1000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 80,
  /* write_pulse_width_ns = */ 80,
  /* tile_width = */ 30,
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 240,
  /* pixel_height = */ 64
};


/* 240x64 */
static const uint8_t u8x8_d_t6963_240x64_init_seq[] = {
  U8X8_DLY(100),
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_DLY(100),
  
  U8X8_AAC(0x00,0x00,0x021),	/* low, high, set cursor pos */
  U8X8_AAC(0x00,0x00,0x022),	/* low, high, set offset */
  U8X8_AAC(0x00,0x00,0x040),	/* low, high, set text home */
  U8X8_AAC(240/8,0x00,0x041),	/* low, high, set text columns */
  U8X8_AAC(0x00,0x00,0x042),	/* low, high, graphics home */  
  U8X8_AAC(240/8,0x00,0x043),	/* low, high, graphics columns */
  U8X8_DLY(2),					/* delay 2ms */
  // mode set
  // 0x080: Internal CG, OR Mode
  // 0x081: Internal CG, EXOR Mode
  // 0x083: Internal CG, AND Mode
  // 0x088: External CG, OR Mode
  // 0x089: External CG, EXOR Mode
  // 0x08B: External CG, AND Mode
  U8X8_C(0x080),            			/* mode register: OR Mode, Internal Character Mode */
  // display mode
  // 0x090: Display off
  // 0x094: Graphic off, text on, cursor off, blink off
  // 0x096: Graphic off, text on, cursor on, blink off
  // 0x097: Graphic off, text on, cursor on, blink on
  // 0x098: Graphic on, text off, cursor off, blink off
  // 0x09a: Graphic on, text off, cursor on, blink off
  // ...
  // 0x09c: Graphic on, text on, cursor off, blink off
  // 0x09f: Graphic on, text on, cursor on, blink on
  U8X8_C(0x090),                             /* All Off */
  U8X8_AAC(0x00,0x00,0x024),	/* low, high, set adr pointer */
  
  U8X8_DLY(100),
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_DLY(100),
};

uint8_t u8x8_d_t6963_240x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_t6963_240x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_t6963_240x64_init_seq);
      break;
    default:
      return u8x8_d_t6963_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}



/*=============================================*/



static const u8x8_display_info_t u8x8_t6963_256x64_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 110,	/* T6963 Datasheet p30 */
  /* pre_chip_disable_wait_ns = */ 100,	/* T6963 Datasheet p30 */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 6, 
  /* sda_setup_time_ns = */ 20,		
  /* sck_pulse_width_ns = */  140,	
  /* sck_clock_hz = */ 1000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 80,
  /* write_pulse_width_ns = */ 80,
  /* tile_width = */ 32,
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 256,
  /* pixel_height = */ 64
};

/* 256x64 */
static const uint8_t u8x8_d_t6963_256x64_init_seq[] = {
  U8X8_DLY(100),
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_DLY(100),
  
  U8X8_AAC(0x00,0x00,0x021),	/* low, high, set cursor pos */
  U8X8_AAC(0x00,0x00,0x022),	/* low, high, set offset */
  U8X8_AAC(0x00,0x00,0x040),	/* low, high, set text home */
  U8X8_AAC(256/8,0x00,0x041),	/* low, high, set text columns */
  U8X8_AAC(0x00,0x00,0x042),	/* low, high, graphics home */  
  U8X8_AAC(256/8,0x00,0x043),	/* low, high, graphics columns */
  U8X8_DLY(2),					/* delay 2ms */
  // mode set
  // 0x080: Internal CG, OR Mode
  // 0x081: Internal CG, EXOR Mode
  // 0x083: Internal CG, AND Mode
  // 0x088: External CG, OR Mode
  // 0x089: External CG, EXOR Mode
  // 0x08B: External CG, AND Mode
  U8X8_C(0x080),            			/* mode register: OR Mode, Internal Character Mode */
  // display mode
  // 0x090: Display off
  // 0x094: Graphic off, text on, cursor off, blink off
  // 0x096: Graphic off, text on, cursor on, blink off
  // 0x097: Graphic off, text on, cursor on, blink on
  // 0x098: Graphic on, text off, cursor off, blink off
  // 0x09a: Graphic on, text off, cursor on, blink off
  // ...
  // 0x09c: Graphic on, text on, cursor off, blink off
  // 0x09f: Graphic on, text on, cursor on, blink on
  U8X8_C(0x090),                             /* All Off */
  U8X8_AAC(0x00,0x00,0x024),	/* low, high, set adr pointer */
  
  U8X8_DLY(100),
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_DLY(100),
};

uint8_t u8x8_d_t6963_256x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_t6963_256x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_t6963_256x64_init_seq);
      break;
    default:
      return u8x8_d_t6963_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}


/*=============================================*/

static const u8x8_display_info_t u8x8_t6963_128x64_display_info =
{
  /* chip_enable_level = */ 1,
  /* chip_disable_level = */ 0,
  
  /* post_chip_enable_wait_ns = */ 10,	/* T6963 Datasheet p30 */
  /* pre_chip_disable_wait_ns = */ 100,	/* T6963 Datasheet p30 */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 6, 
  /* sda_setup_time_ns = */ 20,		
  /* sck_pulse_width_ns = */  140,	
  /* sck_clock_hz = */ 1000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 80,
  /* write_pulse_width_ns = */ 80,
  /* tile_width = */ 16,
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 128,
  /* pixel_height = */ 64
};

/* 128x64 */
static const uint8_t u8x8_d_t6963_128x64_init_seq[] = {
  U8X8_DLY(100),
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_DLY(100),
  
  U8X8_AAC(0x00,0x00,0x021),	/* low, high, set cursor pos */
  U8X8_AAC(0x00,0x00,0x022),	/* low, high, set offset */
  U8X8_AAC(0x00,0x00,0x040),	/* low, high, set text home */
  U8X8_AAC(128/8,0x00,0x041),	/* low, high, set text columns */
  U8X8_AAC(0x00,0x00,0x042),	/* low, high, graphics home */  
  U8X8_AAC(128/8,0x00,0x043),	/* low, high, graphics columns */
  U8X8_DLY(2),					/* delay 2ms */
  // mode set
  // 0x080: Internal CG, OR Mode
  // 0x081: Internal CG, EXOR Mode
  // 0x083: Internal CG, AND Mode
  // 0x088: External CG, OR Mode
  // 0x089: External CG, EXOR Mode
  // 0x08B: External CG, AND Mode
  U8X8_C(0x080),            			/* mode register: OR Mode, Internal Character Mode */
  // display mode
  // 0x090: Display off
  // 0x094: Graphic off, text on, cursor off, blink off
  // 0x096: Graphic off, text on, cursor on, blink off
  // 0x097: Graphic off, text on, cursor on, blink on
  // 0x098: Graphic on, text off, cursor off, blink off
  // 0x09a: Graphic on, text off, cursor on, blink off
  // ...
  // 0x09c: Graphic on, text on, cursor off, blink off
  // 0x09f: Graphic on, text on, cursor on, blink on
  U8X8_C(0x090),                             /* All Off */
  U8X8_AAC(0x00,0x00,0x024),	/* low, high, set adr pointer */
  
  U8X8_DLY(100),
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_DLY(100),
};

uint8_t u8x8_d_t6963_128x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_t6963_128x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_t6963_128x64_init_seq);
      break;
    default:
      return u8x8_d_t6963_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}

/*=============================================*/

static const u8x8_display_info_t u8x8_t6963_160x80_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 10,	/* T6963 Datasheet p30 */
  /* pre_chip_disable_wait_ns = */ 100,	/* T6963 Datasheet p30 */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 6, 
  /* sda_setup_time_ns = */ 20,		
  /* sck_pulse_width_ns = */  140,	
  /* sck_clock_hz = */ 1000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 80,
  /* write_pulse_width_ns = */ 80,
  /* tile_width = */ 20,
  /* tile_hight = */ 10,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 160,
  /* pixel_height = */ 80
};

/* 128x64 */
static const uint8_t u8x8_d_t6963_160x80_init_seq[] = {
  U8X8_DLY(100),
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_DLY(100),
  
  U8X8_AAC(0x00,0x00,0x021),	/* low, high, set cursor pos */
  U8X8_AAC(0x00,0x00,0x022),	/* low, high, set offset */
  U8X8_AAC(0x00,0x00,0x040),	/* low, high, set text home */
  U8X8_AAC(160/8,0x00,0x041),	/* low, high, set text columns */
  U8X8_AAC(0x00,0x00,0x042),	/* low, high, graphics home */  
  U8X8_AAC(160/8,0x00,0x043),	/* low, high, graphics columns */
  U8X8_DLY(2),					/* delay 2ms */
  // mode set
  // 0x080: Internal CG, OR Mode
  // 0x081: Internal CG, EXOR Mode
  // 0x083: Internal CG, AND Mode
  // 0x088: External CG, OR Mode
  // 0x089: External CG, EXOR Mode
  // 0x08B: External CG, AND Mode
  U8X8_C(0x080),            			/* mode register: OR Mode, Internal Character Mode */
  // display mode
  // 0x090: Display off
  // 0x094: Graphic off, text on, cursor off, blink off
  // 0x096: Graphic off, text on, cursor on, blink off
  // 0x097: Graphic off, text on, cursor on, blink on
  // 0x098: Graphic on, text off, cursor off, blink off
  // 0x09a: Graphic on, text off, cursor on, blink off
  // ...
  // 0x09c: Graphic on, text on, cursor off, blink off
  // 0x09f: Graphic on, text on, cursor on, blink on
  U8X8_C(0x090),                             /* All Off */
  U8X8_AAC(0x00,0x00,0x024),	/* low, high, set adr pointer */
  
  U8X8_DLY(100),
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_DLY(100),
};

uint8_t u8x8_d_t6963_160x80(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_t6963_160x80_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_t6963_160x80_init_seq);
      break;
    default:
      return u8x8_d_t6963_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}


/* alternative version for the 128x64 t6963 display: use the 160x80 init sequence */
uint8_t u8x8_d_t6963_128x64_alt(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_t6963_128x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_t6963_160x80_init_seq);
      break;
    default:
      return u8x8_d_t6963_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}


  

  