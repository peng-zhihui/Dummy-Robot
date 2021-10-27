/*

  u8x8_d_ssd1607_200x200.c

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
  
  SSD1607: 200x300x1
  
  command 
    0x22: assign actions
    0x20: execute actions
  
  action for command 0x022 are (more or less guessed)
    bit 7:	Enable Clock
    bit 6:	Enable Charge Pump
    bit 5:	Load Temparture Value (???)
    bit 4:	Load LUT (???)
    bit 3:	Initial Display (???)
    bit 2:	Pattern Display --> Requires about 945ms with the LUT from below
    bit 1:	Disable Charge Pump
    bit 0:	Disable Clock
    
    Disable Charge Pump and Clock require about 267ms
    Enable Charge Pump and Clock require about 10ms

  Notes:
    - Introduced a refresh display message, which copies RAM to display
    - Charge pump and clock are only enabled for the transfer RAM to display
    - U8x8 will not really work because of the two buffers in the SSD1606, however U8g2 should be ok.

*/


#include "u8x8.h"

/*=================================================*/

static const u8x8_display_info_t u8x8_ssd1607_200x200_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* values from SSD1606 */
  /* post_chip_enable_wait_ns = */ 120,
  /* pre_chip_disable_wait_ns = */ 60,
  /* reset_pulse_width_ms = */ 100, 	
  /* post_reset_wait_ms = */ 100, 
  /* sda_setup_time_ns = */ 50,		/* SSD1606: */
  /* sck_pulse_width_ns = */ 100,	/* SSD1606: 100ns */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	
  /* tile_width = */ 25,		/* 25*8 = 200 */
  /* tile_hight = */ 25,		
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 200,
  /* pixel_height = */ 200		
};


static const uint8_t u8x8_d_ssd1607_200x200_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x22, 0xc0),			/* enable clock and charge pump */
  U8X8_C(0x20),				/* execute sequence */  
  U8X8_DLY(200),				/* according to my measures it may take up to 150ms */
  U8X8_DLY(100),				/* but it might take longer */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1607_200x200_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */  
  /* disable clock and charge pump only, deep sleep is not entered, because we will loose RAM content */
  U8X8_CA(0x22, 0x02),			/* only disable charge pump, HW reset seems to be required if the clock is disabled */
  U8X8_C(0x20),				/* execute sequence */  
  U8X8_DLY(20),
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const uint8_t u8x8_d_ssd1607_200x200_exec_1000dly_seq[] = {
  // assumes, that the start transfer has happend
  U8X8_CA(0x22, 0x04),	/* display update seq. option: pattern display */
  U8X8_C(0x20),	/* execute sequence */
  U8X8_DLY(250),
  U8X8_DLY(250),
  U8X8_DLY(250),
  U8X8_DLY(250),
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static void u8x8_d_ssd1607_200x200_first_init(u8x8_t *u8x8)
{
      u8x8_ClearDisplay(u8x8);
  
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x032);		// program update sequence
      u8x8_cad_SendMultipleArg(u8x8, 8, 0x055);		// all black
      u8x8_cad_SendMultipleArg(u8x8, 12, 0x0aa);		// all white
      u8x8_cad_SendMultipleArg(u8x8, 10, 0x022);		// 830ms
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_exec_1000dly_seq);
  
}

static uint8_t *u8x8_convert_tile_for_ssd1607(uint8_t *t)
{
  uint8_t i;
  static uint8_t buf[8];
  uint8_t *pbuf = buf;

  for( i = 0; i < 8; i++ )
  {
    *pbuf++ = ~(*t++);
  }
  return buf;
}

static void u8x8_d_ssd1607_draw_tile(u8x8_t *u8x8, uint8_t arg_int, void *arg_ptr) U8X8_NOINLINE;
static void u8x8_d_ssd1607_draw_tile(u8x8_t *u8x8, uint8_t arg_int, void *arg_ptr)
{
  uint16_t x;
  uint8_t c, page;
  uint8_t *ptr;
  u8x8_cad_StartTransfer(u8x8);

  page = u8x8->display_info->tile_height;
  page --;
  page -= (((u8x8_tile_t *)arg_ptr)->y_pos);
  
  x = ((u8x8_tile_t *)arg_ptr)->x_pos;
  x *= 8;
  x += u8x8->x_offset;
  
  

  u8x8_cad_SendCmd(u8x8, 0x045 );	/* window start column */
  u8x8_cad_SendArg(u8x8, x&255);
  u8x8_cad_SendArg(u8x8, x>>8);
  u8x8_cad_SendArg(u8x8, 199);		/* end of display */
  u8x8_cad_SendArg(u8x8, 0);

  u8x8_cad_SendCmd(u8x8, 0x044 );	/* window end page */
  u8x8_cad_SendArg(u8x8, page);
  u8x8_cad_SendArg(u8x8, page);

  u8x8_cad_SendCmd(u8x8, 0x04f );	/* window column */
  u8x8_cad_SendArg(u8x8, x&255);
  u8x8_cad_SendArg(u8x8, x>>8);

  u8x8_cad_SendCmd(u8x8, 0x04e );	/* window row */
  u8x8_cad_SendArg(u8x8, page);

  u8x8_cad_SendCmd(u8x8, 0x024 );
  
  do
  {
    c = ((u8x8_tile_t *)arg_ptr)->cnt;
    ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
    do
    {
      u8x8_cad_SendData(u8x8, 8, u8x8_convert_tile_for_ssd1607(ptr));
      ptr += 8;
      x += 8;
      c--;
    } while( c > 0 );
    
    arg_int--;
  } while( arg_int > 0 );
  
  u8x8_cad_EndTransfer(u8x8);
}



/*=================================================*/


#define L(a,b,c,d) (((a)<<6)|((b)<<4)|((c)<<2)|(d))


/* https://github.com/embeddedadventures/SSD1607/blob/master/SSD1607.cpp */
static const uint8_t u8x8_d_ssd1607_200x200_init_seq[] = {    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  //U8X8_CA(0x10, 0x00),	/* Deep Sleep mode Control: Disable */
  U8X8_C(0x01),
  U8X8_A(199),U8X8_A(0),U8X8_A(0),
  
  
  U8X8_CA(0x03, 0x00), 	/* Gate Driving voltage: 15V (lowest value)*/
  U8X8_CA(0x04, 0x0a), 	/* Source Driving voltage: 15V (mid value and POR)*/
  
  U8X8_CA(0x0f, 0x00),		/* scan start ? */
  
  U8X8_CA(0xf0, 0x1f),	/* set booster feedback to internal */

  U8X8_CA(0x2c, 0xa8),	/* write vcom value*/
  U8X8_CA(0x3a, 0x1a),	/* dummy lines */
  U8X8_CA(0x3b, 0x08),	/* gate time */
  U8X8_CA(0x3c, 0x33),	/* select boarder waveform */
  
  U8X8_CA(0x11, 0x03),		/* cursor increment mode */
  U8X8_CAA(0x44, 0, 24),	/* RAM x start & end, each byte has 8 pixel, 25*4=200 */
  U8X8_CAAAA(0x45, 0, 0, 299&255, 299>>8),	/* RAM y start & end, 0..299 */
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1607_to_display_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  U8X8_C(0x32),	/* write LUT register*/

  
  /* according to the command table, the lut has 240 bits (=30 bytes * 8 bits) */
  
  /* Waveform part of the LUT (20 bytes) */
  /* bit 7/6: 1 - 1 transition */
  /* bit 5/4: 1 - 0 transition */
  /* bit 3/2: 0 - 1 transition */
  /* bit 1/0: 0 - 0 transition */
  /* 	00 – VSS */
  /* 	01 – VSH */
  /* 	10 – VSL */
  /* 	11 – NA */
  
  /* original values */
  /*
  U8X8_A(0x02),
  U8X8_A(0x02),
  U8X8_A(0x01),
  U8X8_A(0x11),
  U8X8_A(0x12),
  U8X8_A(0x12),
  U8X8_A(0x22),
  U8X8_A(0x22),
  U8X8_A(0x66),
  U8X8_A(0x69),
  U8X8_A(0x69),
  U8X8_A(0x59),
  U8X8_A(0x58),
  U8X8_A(0x99),
  U8X8_A(0x99),
  U8X8_A(0x88),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  */
  
  /* original values, L-macro */
  U8X8_A(L(0,0,0,2)), // 0x02
  U8X8_A(L(0,0,0,2)), // 0x02
  U8X8_A(L(0,0,0,1)), // 0x01
  U8X8_A(L(0,1,0,1)), // 0x11
  U8X8_A(L(0,1,0,2)), // 0x12
  U8X8_A(L(0,1,0,2)), // 0x12
  U8X8_A(L(0,2,0,2)), // 0x22
  U8X8_A(L(0,2,0,2)), // 0x22
  U8X8_A(L(1,2,1,2)), // 0x66
  U8X8_A(L(1,2,2,1)), // 0x69
  U8X8_A(L(1,2,2,1)), // 0x69
  U8X8_A(L(1,1,2,1)), // 0x59
  U8X8_A(L(1,1,2,0)), // 0x58
  U8X8_A(L(2,1,2,1)), // 0x99
  U8X8_A(L(2,1,2,1)), // 0x99
  U8X8_A(L(2,0,2,0)), // 0x88
  U8X8_A(L(0,0,0,0)), // 0x00
  U8X8_A(L(0,0,0,0)), // 0x00
  U8X8_A(L(0,0,0,0)), // 0x00
  U8X8_A(L(0,0,0,0)), // 0x00


  /* orginal values without 0-0 and 1-1 transition */
  /*
  U8X8_A(L(3,0,0,3)), // 0x02
  U8X8_A(L(3,0,0,3)), // 0x02
  U8X8_A(L(3,0,0,3)), // 0x01
  U8X8_A(L(3,1,0,3)), // 0x11
  U8X8_A(L(3,1,0,3)), // 0x12
  U8X8_A(L(3,1,0,3)), // 0x12
  U8X8_A(L(3,2,0,3)), // 0x22
  U8X8_A(L(3,2,0,3)), // 0x22
  U8X8_A(L(3,2,1,3)), // 0x66
  U8X8_A(L(3,2,2,3)), // 0x69
  U8X8_A(L(3,2,2,3)), // 0x69
  U8X8_A(L(3,1,2,3)), // 0x59
  U8X8_A(L(3,1,2,3)), // 0x58
  U8X8_A(L(3,1,2,3)), // 0x99
  U8X8_A(L(3,1,2,3)), // 0x99
  U8X8_A(L(3,0,2,3)), // 0x88
  U8X8_A(L(3,0,0,3)), // 0x00
  U8X8_A(L(3,0,0,3)), // 0x00
  U8X8_A(L(3,0,0,3)), // 0x00
  U8X8_A(L(3,0,0,3)), // 0x00
  */
  
  
  /* Timing part of the LUT, 20 Phases with 4 bit each: 10 bytes */
  U8X8_A(0xF8),
  U8X8_A(0xB4),
  U8X8_A(0x13),
  U8X8_A(0x51),
  U8X8_A(0x35),
  U8X8_A(0x51),
  U8X8_A(0x51),
  U8X8_A(0x19),
  U8X8_A(0x01),
  U8X8_A(0x00),
  
  
  U8X8_CA(0x22, 0x04),	/* display update seq. option: clk -> CP -> LUT -> initial display -> pattern display */
  U8X8_C(0x20),	/* execute sequence */
  U8X8_DLY(250),	/* the sequence above requires about 1200ms for the 200x200 display*/
  U8X8_DLY(250),
  U8X8_DLY(250),
  U8X8_DLY(250),
  U8X8_DLY(250),
  U8X8_DLY(250),
  U8X8_DLY(250),
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_ssd1607_200x200(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1607_200x200_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_init_seq);    
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave0_seq);
      u8x8_d_ssd1607_200x200_first_init(u8x8);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      break;
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_d_ssd1607_draw_tile(u8x8, arg_int, arg_ptr);
      break;
    case U8X8_MSG_DISPLAY_REFRESH:
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_to_display_seq);
      break;
    default:
      return 0;
  }
  return 1;
}


/*=================================================*/
/* there is no improvement possible... so i consider the v2 version as obsolete */


static const uint8_t u8x8_d_ssd1607_v2_to_display_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  /*
0xaa, 0x09, 0x09, 0x19, 0x19, 
0x11, 0x11, 0x11, 0x11, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 

0x75, 0x77, 0x77, 0x77, 0x07, 
0x00, 0x00, 0x00, 0x00, 0x00
measured 1240 ms with IL3830 196x128
  
  
0x02, 0x02, 0x01, 0x11, 0x12, 
0x12, 0x12, 0x22, 0x22, 0x66, 
0x69, 0x59, 0x58, 0x99, 0x99, 
0x88, 0x00, 0x00, 0x00, 0x00, 

0xf8, 0xb4, 0x13, 0x51, 0x35, 
0x51, 0x51, 0xe9, 0x04, 0x00
  
*/

  U8X8_C(0x32),	/* write LUT register*/

  /* https://github.com/olikraus/u8g2/issues/347 */
  U8X8_A(0x02),
  U8X8_A(0x02),
  U8X8_A(0x01),
  U8X8_A(0x11),
  U8X8_A(0x12),
  U8X8_A(0x12),
  U8X8_A(0x22),
  U8X8_A(0x22),
  U8X8_A(0x66),
  U8X8_A(0x69),
  U8X8_A(0x69),
  U8X8_A(0x59),
  U8X8_A(0x58),
  U8X8_A(0x99),
  U8X8_A(0x99),
  
  U8X8_A(0x88),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  
  /* Timing part of the LUT, 20 Phases with 4 bit each: 10 bytes */
  U8X8_A(0xF8),
  U8X8_A(0xB4),
  U8X8_A(0x13),
  U8X8_A(0x51),
  U8X8_A(0x35),
  
  U8X8_A(0x51),
  U8X8_A(0x51),
  U8X8_A(0xe9),
  U8X8_A(0x04),
  U8X8_A(0x00),
  
  U8X8_CA(0x22, 0x04),	/* display update seq. option: clk -> CP -> LUT -> initial display -> pattern display */
  U8X8_C(0x20),	/* execute sequence */
  
  U8X8_DLY(250),	/* delay for 1500ms. The current sequence takes 1300ms */
  U8X8_DLY(250),
  U8X8_DLY(250),
  U8X8_DLY(250),
  
  U8X8_DLY(250),
  U8X8_DLY(250),
  
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_ssd1607_v2_200x200(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1607_200x200_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_init_seq);    
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave0_seq);
      u8x8_d_ssd1607_200x200_first_init(u8x8);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      break;
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_d_ssd1607_draw_tile(u8x8, arg_int, arg_ptr);
      break;
    case U8X8_MSG_DISPLAY_REFRESH:
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_v2_to_display_seq);
      break;
    default:
      return 0;
  }
  return 1;
}


/*=================================================*/
/* GDEP015OC1 */
/* https://github.com/olikraus/u8g2/issues/454 */


static const uint8_t u8x8_d_ssd1607_gd_to_display_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  /*
0xaa, 0x09, 0x09, 0x19, 0x19, 
0x11, 0x11, 0x11, 0x11, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 

0x75, 0x77, 0x77, 0x77, 0x07, 
0x00, 0x00, 0x00, 0x00, 0x00
measured 1240 ms with IL3830 196x128
  
  
0x02, 0x02, 0x01, 0x11, 0x12, 
0x12, 0x12, 0x22, 0x22, 0x66, 
0x69, 0x59, 0x58, 0x99, 0x99, 
0x88, 0x00, 0x00, 0x00, 0x00, 

0xf8, 0xb4, 0x13, 0x51, 0x35, 
0x51, 0x51, 0xe9, 0x04, 0x00
  
*/

  U8X8_C(0x32),	/* write LUT register*/

/*
  U8X8_A(0x50), U8X8_A(0xAA), U8X8_A(0x55), U8X8_A(0xAA), U8X8_A(0x11), 
  U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), 
  U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), 
  U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), 
  
  U8X8_A(0xFF), U8X8_A(0xFF), U8X8_A(0x1F), U8X8_A(0x00), U8X8_A(0x00), 
  U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), 
*/
  U8X8_A(0x10), U8X8_A(0x18), U8X8_A(0x18), U8X8_A(0x08), U8X8_A(0x18),   // numbers based on Waveshare demo code
  U8X8_A(0x18), U8X8_A(0x08), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), 
  U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), 
  U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), 
  
  U8X8_A(0x13), U8X8_A(0x14), U8X8_A(0x44), U8X8_A(0x12), U8X8_A(0x00), 
  U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),

  U8X8_CA(0x22, 0xc4),	/* display update seq. option: clk -> CP -> LUT -> initial display -> pattern display */
  U8X8_C(0x20),	/* execute sequence */
  
  U8X8_DLY(250),	/* delay for 1500ms. The current sequence takes 1300ms */
  U8X8_DLY(250),
  U8X8_DLY(250),
//  U8X8_DLY(250),
  
//  U8X8_DLY(250),
//  U8X8_DLY(250),
  
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_ssd1607_gd_200x200(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1607_200x200_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_init_seq);    
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave0_seq);
      u8x8_d_ssd1607_200x200_first_init(u8x8);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      break;
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_d_ssd1607_draw_tile(u8x8, arg_int, arg_ptr);
      break;
    case U8X8_MSG_DISPLAY_REFRESH:
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_gd_to_display_seq);
      break;
    default:
      return 0;
  }
  return 1;
}



/*=================================================*/

static const uint8_t u8x8_d_ssd1607_ws_to_display_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */


  U8X8_C(0x32),	/* write LUT register*/

  U8X8_A(0x10), U8X8_A(0x18), U8X8_A(0x18), U8X8_A(0x08), U8X8_A(0x18),   // numbers based on Waveshare demo code
  U8X8_A(0x18), U8X8_A(0x08), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), 
  U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), 
  U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), 
  
  U8X8_A(0x13), U8X8_A(0x14), U8X8_A(0x44), U8X8_A(0x12), U8X8_A(0x00), 
  U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00), U8X8_A(0x00),

  U8X8_CA(0x22, 0xc4),	/* display update seq. option: clk -> CP -> LUT -> initial display -> pattern display */
  U8X8_C(0x20),	/* execute sequence */
  
  U8X8_DLY(250),	/* delay for 1250ms.  */
  U8X8_DLY(250),
  U8X8_DLY(250),
  U8X8_DLY(250),
  U8X8_DLY(250),
  
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const uint8_t u8x8_d_ssd1607_ws_to_refresh_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */


  U8X8_CA(0x22, 0x04),	/* display update seq. option: clk -> CP -> LUT -> initial display -> pattern display */
  U8X8_C(0x20),	/* execute sequence */
  
//  U8X8_DLY(250),
//  U8X8_DLY(250),
  
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


/* waveshare 200x200 */
static const uint8_t u8x8_d_ssd1607_ws_200x200_init_seq[] = {    
  // suggested code from https://github.com/olikraus/u8g2/issues/637
  
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
 
  U8X8_C(0x01), /* DRIVER_OUTPUT_CONTROL: LO(EPD_HEIGHT-1), HI(EPD_HEIGHT-1). GD = 0; SM = 0; TB = 0; */
  U8X8_A(199),U8X8_A(0),U8X8_A(0),
  
  U8X8_C(0x0C), /* BOOSTER_SOFT_START_CONTROL */
  U8X8_A(0xd7),U8X8_A(0xd6),U8X8_A(0x9d),
  
  U8X8_CA(0x2c, 0xa8), /* WRITE_VCOM_REGISTER: VCOM 7C */
  U8X8_CA(0x3a, 0x1a), /* SET_DUMMY_LINE_PERIOD: 4 dummy lines per gate */
  U8X8_CA(0x3b, 0x08), /* SET_GATE_TIME: 2us per line */
  U8X8_CA(0x11, 0x03), /* DATA_ENTRY_MODE_SETTING: X increment; Y increment */
  U8X8_CAA(0x44, 0, 24), /* SET_RAM_X_ADDRESS_START_END_POSITION: LO(x >> 3), LO((w-1) >> 3) */
  U8X8_CAAAA(0x45, 0, 0, 199&255, 199>>8), /* SET_RAM_Y_ADDRESS_START_END_POSITION: LO(y), HI(y), LO(h - 1), HI(h - 1) */
  U8X8_CA(0x4e, 0), /* LO(x >> 3) */
  U8X8_CAA(0x4f, 0, 0), /* LO(y), HI(y >> 8) */
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */  
  
};


uint8_t u8x8_d_ssd1607_ws_200x200(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1607_200x200_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_ws_200x200_init_seq);    
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave0_seq);
      u8x8_d_ssd1607_200x200_first_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_ws_to_display_seq);; // to setup LUT
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_200x200_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      break;
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_d_ssd1607_draw_tile(u8x8, arg_int, arg_ptr);
      break;
    case U8X8_MSG_DISPLAY_REFRESH:
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1607_ws_to_refresh_seq);
      break;
    default:
      return 0;
  }
  return 1;
}

