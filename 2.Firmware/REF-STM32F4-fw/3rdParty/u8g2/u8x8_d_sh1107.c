/*

  u8x8_d_sh1107.c

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2017, olikraus@gmail.com
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

/* code copyied from SSD1306 */



static const uint8_t u8x8_d_sh1107_64x128_noname_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0af),		                /* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_sh1107_64x128_noname_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                /* display off */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_sh1107_64x128_noname_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a1),				/* segment remap a0/a1*/
  U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_sh1107_64x128_noname_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a0),				/* segment remap a0/a1*/
  U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static uint8_t u8x8_d_sh1107_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_sh1107_64x128_noname_display_info);
      break;
    */
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_sh1107_64x128_noname_init_seq);    
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_sh1107_64x128_noname_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_sh1107_64x128_noname_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_sh1107_64x128_noname_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_sh1107_64x128_noname_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int );	/* sh1107 has range from 0 to 255 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
      x *= 8;
      x += u8x8->x_offset;

      //u8x8_cad_SendCmd(u8x8, 0x040 ); /* set line offset to 0 */

      // set column address
      u8x8_cad_SendCmd(u8x8, 0x010 | (x >> 4));
      u8x8_cad_SendCmd(u8x8, 0x000 | ((x & 15))); /* probably wrong, should be SendCmd */
      
      // set page address
      u8x8_cad_SendCmd(u8x8, 0x0b0 | (((u8x8_tile_t *)arg_ptr)->y_pos)); /* probably wrong, should be SendCmd */
    
      do
      {
	c = ((u8x8_tile_t *)arg_ptr)->cnt;
	ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
	u8x8_cad_SendData(u8x8, c*8, ptr); 	/* note: SendData can not handle more than 255 bytes */
	/*
	do
	{
	  u8x8_cad_SendData(u8x8, 8, ptr);
	  ptr += 8;
	  c--;
	} while( c > 0 );
	*/
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}

/*==================================================*/

/* QG-6428TSWKG01 */
static const uint8_t u8x8_d_sh1107_64x128_noname_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  
  U8X8_C(0x0ae),		                /* display off */
  U8X8_CA(0x0dc, 0x000),		/* start line */
  U8X8_CA(0x081, 0x02f), 		/* [2] set contrast control */
  U8X8_C(0x020),		                /* addressing mode */

  // U8X8_C(0x0a1),				/* segment remap a0/a1*/
  // U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  // Flipmode
  U8X8_C(0x0a0),				/* segment remap a0/a1*/
  U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  
  U8X8_CA(0x0a8, 0x7f),		/* 0x03f) multiplex ratio */
  U8X8_CA(0x0d3, 0x060),		/* display offset */
  U8X8_CA(0x0d5, 0x051),		/* clock divide ratio (0x00=1) and oscillator frequency (0x8) */
  U8X8_CA(0x0d9, 0x022), 		/* [2] pre-charge period 0x022/f1*/
  U8X8_CA(0x0db, 0x035), 		/* vcomh deselect level */  
  
  U8X8_C(0x0b0), /* set page address */
  U8X8_CA(0x0da, 0x012), /* set com pins */
  U8X8_C(0x0a4),				/* output ram to display */
  U8X8_C(0x0a6),				/* none inverted normal display mode */
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const u8x8_display_info_t u8x8_sh1107_64x128_noname_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* sh1107: 3 us */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 50,		/* sh1107: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 50,	/* sh1107: 20ns, but cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 8000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	/* sh1107: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 8,
  /* tile_height = */ 16,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 64,
  /* pixel_height = */ 128
};

uint8_t u8x8_d_sh1107_64x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    
  if ( u8x8_d_sh1107_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_sh1107_64x128_noname_init_seq);    
      break;
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_sh1107_64x128_noname_display_info);
      break;
    default:
      return 0;
  }
  return 1;
}

/*==================================================*/

/* init sequence from Grove OLED 96x96 */
static const uint8_t u8x8_d_sh1107_seeed_96x96_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  
  U8X8_C(0x0ae),		                /* display off */
  U8X8_CA(0x0d5, 0x050),		/* clock divide ratio (0x00=1) and oscillator frequency (0x5) */
  U8X8_C(0x020),		                /* use page addressing mode */
  //U8X8_CA(0x0a8, 0x03f),		/* multiplex ratio */
  U8X8_CA(0x0d3, 0x000),		/* display offset */
  U8X8_CA(0x0dc, 0x000),		/* start line */
  //U8X8_CA(0x020, 0x000),		/* horizontal addressing mode */
  
  U8X8_C(0x0a1),				/* segment remap a0/a1*/
  U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  // Flipmode
  // U8X8_C(0x0a0),				/* segment remap a0/a1*/
  // U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  
  //U8X8_CA(0x0da, 0x012),		/* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */

  U8X8_CA(0x081, 0x080), 		/* [2] set contrast control */
  U8X8_CA(0x0ad, 0x080), 		/* */  
  U8X8_CA(0x0d9, 0x01f), 		/* [2] pre-charge period 0x022/f1*/
  U8X8_CA(0x0db, 0x027), 		/* vcomh deselect level */  
  // if vcomh is 0, then this will give the biggest range for contrast control issue #98
  // restored the old values for the noname constructor, because vcomh=0 will not work for all OLEDs, #116
  
  //U8X8_C(0x02e),				/* Deactivate scroll */ 
  U8X8_C(0x0a4),				/* output ram to display */
  U8X8_C(0x0a6),				/* none inverted normal display mode */
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const u8x8_display_info_t u8x8_sh1107_seeed_96x96_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 100,		/* cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 100,	/* cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	/* sh1107: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 12,
  /* tile_hight = */ 12,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 96,
  /* pixel_height = */ 96
};

uint8_t u8x8_d_sh1107_seeed_96x96(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    
  if ( u8x8_d_sh1107_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_sh1107_seeed_96x96_init_seq);    
      break;
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_sh1107_seeed_96x96_display_info);
      break;
    default:
      return 0;
  }
  return 1;
}


/*==================================================*/
/* 128x128 OLED: this display has a very strange x offset */

/* sequence taken over from 64x128 sequence, because it seems to work mostly */
static const uint8_t u8x8_d_sh1107_128x128_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  
  U8X8_C(0x0ae),		                /* display off */
  U8X8_CA(0x0dc, 0x000),		/* start line */
  U8X8_CA(0x081, 0x02f), 		/* [2] set contrast control */
  U8X8_C(0x020),		                /* use page addressing mode */

  // U8X8_C(0x0a1),				/* segment remap a0/a1*/
  // U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  // Flipmode
  U8X8_C(0x0a0),				/* segment remap a0/a1*/
  U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  
  U8X8_CA(0x0a8, 0x7f),		/* 0x03f multiplex ratio */
  //U8X8_CA(0x0d3, 0x060),		/* display offset (removed, not in datasheet ) */
  U8X8_CA(0x0d5, 0x050),		/* clock divide ratio (0x00=1) and oscillator frequency (0x8), changed to 0x051, issue 501 */
  U8X8_CA(0x0d9, 0x022), 		/* [2] pre-charge period 0x022/f1*/
  U8X8_CA(0x0db, 0x035), 		/* vcomh deselect level */  
  
  U8X8_C(0x0b0), /* set page address */
  U8X8_CA(0x0da, 0x012), /* set com pins */
  U8X8_C(0x0a4),				/* output ram to display */
  U8X8_C(0x0a6),				/* none inverted normal display mode */
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const u8x8_display_info_t u8x8_sh1107_128x128_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 100,		/* cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 100,	/* cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	/* sh1107: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 16,
  /* tile_hight = */ 16,
  /* default_x_offset = */ 96,
  /* flipmode_x_offset = */ 96,
  /* pixel_width = */ 128,
  /* pixel_height = */ 128
};

uint8_t u8x8_d_sh1107_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    
  if ( u8x8_d_sh1107_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_sh1107_128x128_init_seq);    
      break;
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_sh1107_128x128_display_info);
      break;
    default:
      return 0;
  }
  return 1;
}

/*==================================================*/
/* pimoroni_128x128_display */

static const u8x8_display_info_t u8x8_sh1107_pimoroni_128x128_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 100,		/* cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 100,	/* cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	/* sh1107: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 16,
  /* tile_hight = */ 16,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 128,
  /* pixel_height = */ 128
};

uint8_t u8x8_d_sh1107_pimoroni_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    
  if ( u8x8_d_sh1107_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_sh1107_128x128_init_seq);    
      break;
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_sh1107_pimoroni_128x128_display_info);
      break;
    default:
      return 0;
  }
  return 1;
}

/*==================================================*/
/*
Name: 	SH1107_seeed_128x128
URL: 	https://www.seeedstudio.com/Grove-OLED-Display-1-12-V2.html 
Display is there in my lab. Backside PCB label: "OLED Display 1.12 inch v1.0"
Tookover code from SSD1327_SEEED_96X96 because none of the other displays did work
and at least the 96x96 driver did show something.
*/

static const u8x8_display_info_t u8x8_seeed_128x128_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 100,		/* cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 100,	/* cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 2,		// 400kHz does not work, but 200kHz seems to be ok
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	/* sh1107: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 16,
  /* tile_hight = */ 16,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 128,
  /* pixel_height = */ 128
};

uint8_t u8x8_d_sh1107_seeed_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    
  if ( u8x8_d_sh1107_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_sh1107_128x128_init_seq); 
      break;
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_seeed_128x128_display_info);
      break;
    default:
      return 0;
  }
  return 1;
}
