/*

  u8x8_d_ssd1320.c
  
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2020, olikraus@gmail.com
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


  https://github.com/olikraus/u8g2/issues/1351
  SSD1320: 
    160 x 160 dot matrix
    16 gray scale
  
  Adapted from u8x8_d_ssd1322.c with the command set of the SSD1320 controller
  "official" procedure is described here: https://github.com/olikraus/u8g2/wiki/internal
  
  NOTE: U8x8 does NOT work!
  
*/

#include "u8x8.h"

static const uint8_t u8x8_d_ssd1320_cs1_160x132_nhd_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0af),		                /* ssd1320: display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1320_cs1_160x132_nhd_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                /* ssd1320: display off */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


/*
  input:32
    one tile (8 Bytes; 1 byte per column)
  output:
    Tile for SSD1320 (32 Bytes)

  The origin of the display seems to be in the upper right-hand corner. Therefore
  compared to SSD1322, the order inside each byte is swapped.
*/

static uint8_t u8x8_ssd1320_to32_dest_buf[32];

static uint8_t *u8x8_ssd1320_8to32(U8X8_UNUSED u8x8_t *u8x8, uint8_t *ptr)
{
  uint8_t v;
  uint8_t a,b;
  uint8_t i, j;
  uint8_t *dest;
  
  for( j = 0; j < 4; j++ )
  {
    dest = u8x8_ssd1320_to32_dest_buf;
    dest += j;
    a =*ptr;
    ptr++;
    b = *ptr;
    ptr++;
    for( i = 0; i < 8; i++ )
    {
      v = 0;
      if ( a&1 ) v |= 0x0f;
      if ( b&1 ) v |= 0xf0;
      *dest = v;
      dest+=4;
      a >>= 1;
      b >>= 1;
    }
  }
  
  return u8x8_ssd1320_to32_dest_buf;
}


uint8_t u8x8_d_ssd1320_common(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x; 
  uint8_t y, c;
  uint8_t *ptr;
  switch(msg)
  {
    /* U8X8_MSG_DISPLAY_SETUP_MEMORY is handled by the calling function */
    /*
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1320_256x64_init_seq);
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1320_cs1_160x132_nhd_powersave0_seq);
      else
	      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1320_cs1_160x132_nhd_powersave1_seq);
      break;

#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int );	/* ssd1320 has range from 1 to 255 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif

    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
      y = (((u8x8_tile_t *)arg_ptr)->y_pos);
      x += u8x8->x_offset;		
    
      y *= 8;
    
      
      u8x8_cad_SendCmd(u8x8, 0x022 );	/* set row address, moved out of the loop (issue 302) */
      u8x8_cad_SendArg(u8x8, y);
      u8x8_cad_SendArg(u8x8, y+7);
      
      do {
	      c = ((u8x8_tile_t *)arg_ptr)->cnt;
	      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;

        do {
          u8x8_cad_SendCmd(u8x8, 0x021 );	/* set column address */
          u8x8_cad_SendArg(u8x8, x );	/* start */
          u8x8_cad_SendArg(u8x8, x+3 );	/* end */
          
          u8x8_cad_SendData(u8x8, 32, u8x8_ssd1320_8to32(u8x8, ptr));
          
          ptr += 8;
          x += 4;
          c--;
        } while( c > 0 );
      
      //x += 2;
      arg_int--;
    } while( arg_int > 0 );
    
    u8x8_cad_EndTransfer(u8x8);
    break;

    default:
      return 0;
  }
  return 1;
}

/*=========================================================*/
/* 160x32 */

static const uint8_t u8x8_d_ssd1320_cs1_160x32_nhd_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a0),		/* remap */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1320_cs1_160x32_nhd_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a1),		/* remap */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const u8x8_display_info_t u8x8_d_ssd1320_cs1_160x32_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* ssd1320: 2 us */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 50,		/* ssd1320: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 50,	/* ssd1320: 20ns, but cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 10000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns, increased to 8MHz (issue 215), 10 MHz (issue 301) */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 10,
  /* write_pulse_width_ns = */ 150,	/* ssd1320: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 20,		/* 160 pixel, so we require 20 bytes for this */
  /* tile_hight = */ 4,
  /* default_x_offset = */ 0,	/* this is the byte offset (there are two pixel per byte with 4 bit per pixel) */
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 160,
  /* pixel_height = */ 32
};

// initialisation sequence from the Arduino Library
// (see https://github.com/sparkfun/SparkFun_SSD1320_OLED_Arduino_Library)
static const uint8_t u8x8_d_ssd1320_cs1_160x32_init_seq[] = {
    
    U8X8_DLY(1),
    U8X8_START_TRANSFER(),    /* enable chip, delay is part of the transfer start */
    U8X8_DLY(1),
    
    U8X8_C(0xae),		          /* display off */
    U8X8_CA(0xd5, 0xC2),			/* set display clock divide ratio/oscillator frequency (set clock as 80 frames/sec)  */  
    U8X8_CA(0xa8, 0x1f),			/* multiplex ratio 1/64 Duty (0x0F~0x3F) */  
    U8X8_CA(0xa2, 0x00),			/* display start line */  

    U8X8_C(0xa0),	                /* Set Segment Re-Map: column address 0 mapped to SEG0  CS1 */ 
    // U8X8_C(0xa1),	                /* Set Segment Re-Map: column address 0 mapped to SEG0  CS2 */ 

    U8X8_C(0xc8),	             /* Set COM Output Scan Direction: normal mode CS1 */
    // U8X8_C(0xc0),			        /* Set COM Output Scan Direction: normal mode CS2 */
    
    U8X8_CA(0xd3, 0x72),        /* CS1 */
    // U8X8_CA(0xd3, 0x92),        /* CS2 */
    
    U8X8_CA(0xda, 0x12),	    /* Set SEG Pins Hardware Configuration:  */  
    U8X8_CA(0x81, 0x5a),			/* contrast */  
    U8X8_CA(0xd9, 0x22),			/* Set Phase Length */  
    U8X8_CA(0xdb, 0x30),		  /* VCOMH Deselect Level */
    U8X8_CA(0xad, 0x10),			/* Internal IREF Enable */  
    U8X8_CA(0x20, 0x00),	    /* Memory Addressing Mode: Horizontal */  
    U8X8_CA(0x8d, 0x01),			/* disable internal charge pump 1 */  
    U8X8_CA(0xac, 0x00),			/* disable internal charge pump 2 */  
    U8X8_C(0xa4),		        	/* display on */  
    U8X8_C(0xa6),		          /* normal display */

    U8X8_DLY(1),					/* delay 2ms */

    U8X8_END_TRANSFER(),             	/* disable chip */
    U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_ssd1320_160x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
        u8x8_d_helper_display_setup_memory(u8x8, &u8x8_d_ssd1320_cs1_160x32_display_info);
      break;

    case U8X8_MSG_DISPLAY_INIT:
        u8x8_d_helper_display_init(u8x8);
        u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1320_cs1_160x32_init_seq);
      break;

    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 ){
        u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1320_cs1_160x32_nhd_flip0_seq);
        u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else{
        u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1320_cs1_160x32_nhd_flip1_seq);
        u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
    
    default:
      return u8x8_d_ssd1320_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}

/*=========================================================*/
/* 160x132 (actually 320x132) */

static const uint8_t u8x8_d_ssd1320_cs1_160x132_nhd_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a0),		/* remap */
  U8X8_C(0xc8),	             /* Set COM Output Scan Direction: normal mode CS1 */
  U8X8_CA(0xd3, 0x0e),        /* CS1 */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1320_cs1_160x132_nhd_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a1),		/* remap */
  U8X8_C(0xc0),	             /* Set COM Output Scan Direction: normal mode CS1 */
  U8X8_CA(0xd3, 0x92),        /* CS1 */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const u8x8_display_info_t u8x8_d_ssd1320_cs1_160x132_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* ssd1320: 2 us */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 50,		/* ssd1320: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 50,	/* ssd1320: 20ns, but cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 10000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns, increased to 8MHz (issue 215), 10 MHz (issue 301) */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 10,
  /* write_pulse_width_ns = */ 150,	/* ssd1320: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 20,		/* 160 pixel, so we require 20 bytes for this */
  /* tile_hight = */ 17,
  /* default_x_offset = */ 0,	/* this is the byte offset (there are two pixel per byte with 4 bit per pixel) */
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 160,
  /* pixel_height = */ 132
};


/* the following sequence will work, but requires contrast to be very high */
static const uint8_t u8x8_d_ssd1320_cs1_160x132_init_seq[] = {
    
    U8X8_DLY(1),
    U8X8_START_TRANSFER(),    /* enable chip, delay is part of the transfer start */
    U8X8_DLY(1),
    
    U8X8_C(0xae),		          /* display off */
    U8X8_CA(0xd5, 0xC2),			/* set display clock divide ratio/oscillator frequency (set clock as 80 frames/sec)  */  
    U8X8_CA(0xa8, 0x83),			/* multiplex ratio 1/132 Duty  */  
    U8X8_CA(0xa2, 0x00),			/* display start line */  

    U8X8_C(0xa0),	                /* Set Segment Re-Map: column address 0 mapped to SEG0  CS1 */ 
    // U8X8_C(0xa1),	                /* Set Segment Re-Map: column address 0 mapped to SEG0  CS2 */ 

    U8X8_C(0xc8),	             /* Set COM Output Scan Direction: normal mode CS1 */
    // U8X8_C(0xc0),			        /* Set COM Output Scan Direction: normal mode CS2 */
  
    U8X8_CA(0xd3, 0x0e),        /* CS1 */
    // U8X8_CA(0xd3, 0x92),        /* CS2 */
    
    U8X8_CA(0xda, 0x12),	    /* Set SEG Pins Hardware Configuration:  */  
    U8X8_CA(0x81, 0x5a),			/* contrast */  
    U8X8_CA(0xd9, 0x22),			/* Set Phase Length */  
    U8X8_CA(0xdb, 0x30),		  /* VCOMH Deselect Level */
    U8X8_CA(0xad, 0x10),			/* Internal IREF Enable */  
    U8X8_CA(0x20, 0x00),	    /* Memory Addressing Mode: Horizontal */  
    U8X8_CA(0x8d, 0x01),			/* disable internal charge pump 1 */  
    U8X8_CA(0xac, 0x00),			/* disable internal charge pump 2 */  
    U8X8_C(0xa4),		        	/* display on */  
    U8X8_C(0xa6),		          /* normal display */

    U8X8_DLY(1),					/* delay 2ms */

    U8X8_END_TRANSFER(),             	/* disable chip */
    U8X8_END()             			/* end of sequence */
};

/*
OLED_WR_Byte(0xae,OLED_CMD);//Display OFF
OLED_WR_Byte(0xfd,OLED_CMD);//Set Command Lock
OLED_WR_Byte(0x12,OLED_CMD);
OLED_WR_Byte(0x20,OLED_CMD);//Set Memory Addressing Mode
OLED_WR_Byte(0x00,OLED_CMD);
OLED_WR_Byte(0x25,OLED_CMD);//Set Portrait Addressing Mode
OLED_WR_Byte(0x00,OLED_CMD);//Normal Addressing Mode
OLED_WR_Byte(0x81,OLED_CMD);//Set Contrast Control
OLED_WR_Byte(0x6b,OLED_CMD);
OLED_WR_Byte1(0xa0,OLED_CMD,1);//Set Seg Remap LEFT DISPLAY
OLED_WR_Byte1(0xa1,OLED_CMD,2);//Set Seg Remap RIGHT DISPLAY
OLED_WR_Byte(0xa2,OLED_CMD);//Set Display Start Line
OLED_WR_Byte(0x00,OLED_CMD);
OLED_WR_Byte(0xa4,OLED_CMD);//Resume to RAM content display
OLED_WR_Byte(0xa6,OLED_CMD);//Set Normal Display

OLED_WR_Byte(0xa8,OLED_CMD);//Set MUX Ratio
OLED_WR_Byte(0x83,OLED_CMD);//1/132 duty

OLED_WR_Byte(0xad,OLED_CMD);//Select external or internal IREF
OLED_WR_Byte(0x10,OLED_CMD);
OLED_WR_Byte(0xbc,OLED_CMD);//Set Pre-charge voltage
OLED_WR_Byte(0x1e,OLED_CMD);//
OLED_WR_Byte(0xbf,OLED_CMD);//Linear LUT
OLED_WR_Byte1(0xc8,OLED_CMD,1);//Set COM Output Scan Direction LEFT DISPLAY
OLED_WR_Byte1(0xc0,OLED_CMD,2);//Set COM Output Scan Direction RIGHT DISPLAY
OLED_WR_Byte(0xd3,OLED_CMD);//Set Display Offset
OLED_WR_Byte1(0x0e,OLED_CMD,1); //LEFT DISPLAY
OLED_WR_Byte1(0x92,OLED_CMD,2); // RIGHT DISPLAY
OLED_WR_Byte(0xd5,OLED_CMD);//Set Display Clock Divide Ratio/Oscillator Frequency
OLED_WR_Byte(0xc2,OLED_CMD);//85Hz
OLED_WR_Byte(0xd9,OLED_CMD);//Set Pre-charge Period
OLED_WR_Byte(0x72,OLED_CMD);//
OLED_WR_Byte(0xda,OLED_CMD);//Set SEG Pins Hardware Configuration
OLED_WR_Byte(0x32,OLED_CMD);
OLED_WR_Byte(0xbd,OLED_CMD);//Set VP
OLED_WR_Byte(0x03,OLED_CMD);
OLED_WR_Byte(0xdb,OLED_CMD);//Set VCOMH
OLED_WR_Byte(0x30,OLED_CMD);
OLED_WR_Byte(0xaf,OLED_CMD);//Display on
*/
static const uint8_t u8x8_d_ssd1320_160x132_init_seq[] = {
    U8X8_DLY(1),
    U8X8_START_TRANSFER(),    /* enable chip, delay is part of the transfer start */
    U8X8_DLY(1),
    
    U8X8_C(0xae),		          /* display off */
    U8X8_CA(0xd5, 0xC2),	/* set display clock divide ratio/oscillator frequency (set clock as 80 frames/sec)  */  
    U8X8_CA(0xa8, 0x83),	/* multiplex ratio 1/132 Duty  */  
    U8X8_CA(0xa2, 0x00),	/* display start line */  

    U8X8_C(0xa0),	                /* Set Segment Re-Map: column address 0 mapped to SEG0  CS1 */ 
    // U8X8_C(0xa1),	      	/* Set Segment Re-Map: column address 0 mapped to SEG0  CS2 */ 

    U8X8_C(0xc8),	             	/* Set COM Output Scan Direction: normal mode CS1 */
    // U8X8_C(0xc0),		/* Set COM Output Scan Direction: normal mode CS2 */

    U8X8_CA(0xad, 0x10), 		/* select Iref: 0x00 external (reset default), 0x10 internal */
    U8X8_CA(0xbc, 0x1e), 		/* pre-charge voltage level 0x00..0x1f, reset default: 0x1e */
    U8X8_C(0xbf),		        	/* select linear LUT */  
    U8X8_CA(0xd5, 0xc2), 		/* Bit 0..3: clock ratio 1, 2, 4, 8, ...256, reset=0x1, Bit 4..7: F_osc 0..15 */
    U8X8_CA(0xd9, 0x72),		/* Set Phase 1&2 Length, Bit 0..3: Phase 1, Bit 4..7: Phase 2, reset default 0x72 */  
    U8X8_CA(0xbd, 0x03), 		/* from the vendor init sequence */
    U8X8_CA(0xdb, 0x30),		  /* VCOMH Deselect Level */

  
    U8X8_CA(0xd3, 0x0e),        /* CS1 */
    // U8X8_CA(0xd3, 0x92),        /* CS2 */
    
    U8X8_CA(0xda, 0x12),	/* Set SEG Pins Hardware Configuration:  */  
    U8X8_CA(0x81, 0x6b),			/* contrast */  
    //U8X8_CA(0xd9, 0x22),			/* Set Phase Length */  
    //U8X8_CA(0xdb, 0x30),		  /* VCOMH Deselect Level */
    //U8X8_CA(0xad, 0x10),			/* Internal IREF Enable */  
    U8X8_CA(0x20, 0x00),	    /* Memory Addressing Mode: Horizontal */  
    //U8X8_CA(0x8d, 0x01),			/* unknown in SSD1320 datasheet, disable internal charge pump 1 */  
    //U8X8_CA(0xac, 0x00),			/* unknown in SSD1320 datasheet, disable internal charge pump 2 */  
    U8X8_C(0xa4),		        	/* display RAM on */  
    U8X8_C(0xa6),		          /* normal display */

    U8X8_DLY(1),					/* delay 2ms */

    U8X8_END_TRANSFER(),             	/* disable chip */
    U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_ssd1320_160x132(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
        u8x8_d_helper_display_setup_memory(u8x8, &u8x8_d_ssd1320_cs1_160x132_display_info);
    
      break;

    case U8X8_MSG_DISPLAY_INIT:
        u8x8_d_helper_display_init(u8x8);
       // u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1320_cs1_160x132_init_seq);
        u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1320_160x132_init_seq);
      break;

    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 ){
        u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1320_cs1_160x132_nhd_flip0_seq);
        u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else{
        u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1320_cs1_160x132_nhd_flip1_seq);
        u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
    
    default:
      return u8x8_d_ssd1320_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}

