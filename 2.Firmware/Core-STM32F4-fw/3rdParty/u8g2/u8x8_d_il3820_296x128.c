/*

  u8x8_d_il3820_296x128.c

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
  
  il3820: 200x300x1
  
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
    
    Disable Charge Pump and Clock require about 10ms
    Enable Charge Pump and Clock require about 100 to 300ms

  Notes:
    - Introduced a refresh display message, which copies RAM to display
    - Charge pump is always enabled. Charge pump can be enabled/disabled via power save message
    - U8x8 will not really work because of the two buffers in the SSD1606, however U8g2 should be ok.

  LUT for the 296x128 device (IL3820)
  LUT (cmd: 0x032 has 30 bytes)
  section 6.8 of the datasheet mentions 256 bits = 32 bytes for the LUT
  chapter 7 tells 30 bytes

  according to section 6.8:
  20 bytes waveform
  10 bytes timing
  1 byte named as VSH/VSL
  1 empty byte
  according to the command table, the lut has 240 bits (=30 bytes * 8 bits)


  LUT / Refresh time
    total_refresh_time = (refresh_lines + dummy_lines*2)*TGate*TS_Sum/f_OSC

    f_OSC=1MHz (according to the datasheets)
    refreh_lines = 296 (for the waveshare display, 0x045 cmd)
    dummy_lines = 22 (for the upcoming u8g2 code, 0x03a cmd)
    TGate = 62 (POR default, 0x03b cmd)
    TS_Sum: Sum of all TS entries of the second part of the LUT
    f_OSC: 1MHz according to the datasheet.

    so we have

    total_refresh_time = 21080*TS_Sum/1000000 = 21ms * TS_Sum


  This file includes two devices:
    u8x8_d_il3820_296x128		--> includes LUT which is probably from the WaveShare 2.9 Vendor
    u8x8_d_il3820_v2_296x128		--> includes LUT which was optimized for faster speed and lesser flicker

*/
  
/* Waveform part of the LUT (20 bytes) */
/* bit 7/6: 1 - 1 transition */
/* bit 5/4: 1 - 0 transition */
/* bit 3/2: 0 - 1 transition */
/* bit 1/0: 0 - 0 transition */
/* 	00 – VSS */
/* 	01 – VSH */
/* 	10 – VSL */
/* 	11 – NA */
  

#include "u8x8.h"

/*=================================================*/
/* common code for all devices */


static const uint8_t u8x8_d_il3820_296x128_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x22, 0xc0),			/* enable clock and charge pump */
  U8X8_C(0x20),				/* execute sequence */  
  U8X8_DLY(200),				/* according to my measures it may take up to 150ms */
  U8X8_DLY(100),				/* but it might take longer */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_il3820_296x128_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */  
  /* disable clock and charge pump only, deep sleep is not entered, because we will loose RAM content */
  U8X8_CA(0x22, 0x02),			/* only disable charge pump, HW reset seems to be required if the clock is disabled */
  U8X8_C(0x20),				/* execute sequence */  
  U8X8_DLY(20),
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

// static const uint8_t u8x8_d_il3820_296x128_flip0_seq[] = {
//   U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
//   U8X8_END_TRANSFER(),             	/* disable chip */
//   U8X8_END()             			/* end of sequence */
// };

// static const uint8_t u8x8_d_il3820_296x128_flip1_seq[] = {
//   U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
//   U8X8_END_TRANSFER(),             	/* disable chip */
//   U8X8_END()             			/* end of sequence */
// };


static const u8x8_display_info_t u8x8_il3820_296x128_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 120,
  /* pre_chip_disable_wait_ns = */ 60,
  /* reset_pulse_width_ms = */ 100, 	
  /* post_reset_wait_ms = */ 100, 
  /* sda_setup_time_ns = */ 50,		/* IL3820 */
  /* sck_pulse_width_ns = */ 125,	/* IL3820: 125ns, clock cycle = 250ns */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	
  /* tile_width = */ 37,		/* 37*8 = 296 */
  /* tile_hight = */ 16,		/* 16*8 = 128 */	
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 296,
  /* pixel_height = */ 128
};


static uint8_t *u8x8_convert_tile_for_il3820(uint8_t *t)
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

static void u8x8_d_il3820_draw_tile(u8x8_t *u8x8, uint8_t arg_int, void *arg_ptr) U8X8_NOINLINE;
static void u8x8_d_il3820_draw_tile(u8x8_t *u8x8, uint8_t arg_int, void *arg_ptr)
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

  //u8x8_cad_SendCmd(u8x8, 0x011 );	/* cursor increment mode */
  //u8x8_cad_SendArg(u8x8, 7);

  u8x8_cad_SendCmd(u8x8, 0x04f );	/* set cursor column */
  u8x8_cad_SendArg(u8x8, x&255);
  u8x8_cad_SendArg(u8x8, x>>8);

  u8x8_cad_SendCmd(u8x8, 0x04e );	/* set cursor row */
  u8x8_cad_SendArg(u8x8, page);

  u8x8_cad_SendCmd(u8x8, 0x024 );
  
  do
  {
    c = ((u8x8_tile_t *)arg_ptr)->cnt;
    ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
    do
    {
      u8x8_cad_SendData(u8x8, 8, u8x8_convert_tile_for_il3820(ptr));
      ptr += 8;
      x += 8;
      c--;
    } while( c > 0 );
    
    arg_int--;
  } while( arg_int > 0 );
  
  u8x8_cad_EndTransfer(u8x8);
}



static const uint8_t u8x8_d_il3820_exec_1000dly_seq[] = {
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

static void u8x8_d_il3820_first_init(u8x8_t *u8x8)
{
      u8x8_ClearDisplay(u8x8);
  
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x032);		// program update sequence
      u8x8_cad_SendMultipleArg(u8x8, 8, 0x055);		// all black
      u8x8_cad_SendMultipleArg(u8x8, 12, 0x0aa);		// all white
      u8x8_cad_SendMultipleArg(u8x8, 10, 0x022);		// 830ms
      u8x8_cad_SendSequence(u8x8, u8x8_d_il3820_exec_1000dly_seq);
  
}

#ifdef OBSOLETE
static void u8x8_d_il3820_second_init(u8x8_t *u8x8)
{
      u8x8_ClearDisplay(u8x8);
  
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x032);		// program update sequence
      u8x8_cad_SendMultipleArg(u8x8, 20, 0x000);		// do nothing
      u8x8_cad_SendMultipleArg(u8x8, 10, 0x011);		// 414ms dly
      /* reuse sequence from above, ok some time is wasted here, */
      /* delay could be lesser */
      u8x8_cad_SendSequence(u8x8, u8x8_d_il3820_exec_1000dly_seq);  
}
#endif


/*=================================================*/
/* first version, LUT from WaveShare */


/* http://www.waveshare.com/wiki/File:2.9inch_e-Paper_Module_code.7z */
static const uint8_t u8x8_d_il3820_296x128_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  U8X8_CA(0x10, 0x00),	/* Deep Sleep mode Control: Disable */
  U8X8_C(0x01),
  U8X8_A(295 % 256), U8X8_A(295/256), U8X8_A(0),
  
  
  U8X8_CA(0x03, 0x00), 	/* Gate Driving voltage: 15V (lowest value)*/
  U8X8_CA(0x04, 0x0a), 	/* Source Driving voltage: 15V (mid value and POR)*/
  
  //U8X8_CA(0x22, 0xc0),	/* display update seq. option: enable clk, enable CP, .... todo: this is never activated */

  //U8X8_CA(0x0b, 7),	/* Set Delay of gate and source non overlap period, POR = 7 */
  U8X8_CA(0x2c, 0xa8),	/* write vcom value*/
  U8X8_CA(0x3a, 0x16),	/* dummy lines POR=22 (0x016) */
  U8X8_CA(0x3b, 0x08),	/* gate time POR=0x08*/
  U8X8_CA(0x3c, 0x33),	/* select boarder waveform */
  //U8X8_CA(0x22, 0xc4),	/* display update seq. option: clk -> CP -> LUT -> initial display -> pattern display */


  U8X8_CA(0x11, 0x07),	/* Define data entry mode, x&y inc, x first*/

  U8X8_CAA(0x44, 0, 29),	/* RAM x start & end, issue 920: end should be (128/8)-1=15. */
  U8X8_CAAAA(0x45, 0, 0, 295&255, 295>>8),	/* RAM y start & end */
  
  //U8X8_CA(0x4e, 0),	/* set x pos, 0..29? */
  //U8X8_CAA(0x4f, 0, 0),	/* set y pos, 0...320??? */


  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const uint8_t u8x8_d_il3820_to_display_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
/*
0x50, 0xAA, 0x55, 0xAA, 0x11, 	0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 	0x00, 0x00, 0x00, 0x00, 0x00, 
0xFF, 0xFF, 0x1F, 0x00, 0x00, 		0x00, 0x00, 0x00, 0x00, 0x00
measured 1582 ms
*/
  U8X8_C(0x32),	/* write LUT register*/
  /* original values */
  U8X8_A(0x50),
  U8X8_A(0xaa),
  U8X8_A(0x55),
  U8X8_A(0xaa),  
  U8X8_A(0x11),
  
  U8X8_A(0x11),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),  
  U8X8_A(0x00),
  
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  
  /* Timing part of the LUT, 20 Phases with 4 bit each: 10 bytes */
  U8X8_A(0xff),
  U8X8_A(0xff),
  U8X8_A(0x3f),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),

  U8X8_CA(0x22, 0x04),	/* display update seq. option: pattern display, assumes clk and charge pump are enabled  */
  U8X8_C(0x20),	/* execute sequence */
  
  U8X8_DLY(250),	/* delay for 1620ms. The current sequence takes 1582ms */
  U8X8_DLY(250),
  U8X8_DLY(250),
  U8X8_DLY(250),
  
  U8X8_DLY(250),
  U8X8_DLY(250),
  U8X8_DLY(120),
   
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_il3820_296x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_il3820_296x128_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:

      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_il3820_296x128_init_seq);    

      u8x8_cad_SendSequence(u8x8, u8x8_d_il3820_296x128_powersave0_seq);
      u8x8_d_il3820_first_init(u8x8);

      /* usually the DISPLAY_INIT message leaves the display in power save state */
      /* however this is not done for e-paper devices, see: */
      /* https://github.com/olikraus/u8g2/wiki/internal#powersave-mode */
    
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_il3820_296x128_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_il3820_296x128_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_d_il3820_draw_tile(u8x8, arg_int, arg_ptr);
      break;
    case U8X8_MSG_DISPLAY_REFRESH:
      u8x8_cad_SendSequence(u8x8, u8x8_d_il3820_to_display_seq);
      break;
    default:
      return 0;
  }
  return 1;
}

/*=================================================*/
/* second version for the IL3820 display */


/* http://www.waveshare.com/wiki/File:2.9inch_e-Paper_Module_code.7z */
static const uint8_t u8x8_d_il3820_v2_296x128_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  // U8X8_CA(0x10, 0x00),	/* Deep Sleep mode Control: POR: Normal mode */
  U8X8_C(0x01),
  U8X8_A(295 % 256), U8X8_A(295/256), U8X8_A(0),
  
  /* the driving voltagesmust not be that high, in order to aviod level change after */
  /* some seconds (which happens with 0xea */
  U8X8_CA(0x03, 0x75), 	/* Gate Driving voltage: +/-15V =0x00 POR (+22/-20V) = 0x0ea*/
  U8X8_CA(0x04, 0x0a), 	/* Source Driving voltage:  (POR=0x0a=15V), max=0x0e*/
  
  U8X8_CA(0x0b, 7),	/* Set Delay of gate and source non overlap period, POR = 7 */
  U8X8_CA(0x2c, 0xa8),	/* write vcom value*/
  U8X8_CA(0x3a, 0x16),	/* dummy lines POR=22 (0x016) */
  U8X8_CA(0x3b, 0x08),	/* gate time POR=0x08*/
  U8X8_CA(0x3c, 0x33),	/* select boarder waveform */

  U8X8_CA(0x11, 0x07),	/* Define data entry mode, x&y inc, x first*/
  U8X8_CAA(0x44, 0, 29),	/* RAM x start & end, 32*4=128 */
  U8X8_CAAAA(0x45, 0, 0, 295&255, 295>>8),	/* RAM y start & end, 0..295 */
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const uint8_t u8x8_d_il3820_v2_to_display_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

/*
0xaa, 0x09, 0x09, 0x19, 0x19, 
0x11, 0x11, 0x11, 0x11, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 

0x75, 0x77, 0x77, 0x77, 0x07, 
0x00, 0x00, 0x00, 0x00, 0x00
measured 1240 ms
*/
  U8X8_C(0x32),	/* write LUT register*/
  /* https://github.com/olikraus/u8g2/issues/347 */
  U8X8_A(0xaa),
  U8X8_A(0x09),
  U8X8_A(0x09),
  U8X8_A(0x19),  
  U8X8_A(0x19),
  
  U8X8_A(0x11),
  U8X8_A(0x11),
  U8X8_A(0x11),
  U8X8_A(0x11),  
  U8X8_A(0x00),
  
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  
  /* Timing part of the LUT, 20 Phases with 4 bit each: 10 bytes */
  U8X8_A(0x75),
  U8X8_A(0x77),
  U8X8_A(0x77),
  U8X8_A(0x77),
  U8X8_A(0x07),
  
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  U8X8_A(0x00),
  
  U8X8_CA(0x22, 0x04),	/* display update seq. option: pattern display */
  U8X8_C(0x20),	/* execute sequence */
  
  U8X8_DLY(250),	/* delay for 1400ms. The current sequence takes 1240ms, it was reported, that longer delays are better */
  U8X8_DLY(250),
  U8X8_DLY(250),
  U8X8_DLY(250),
  
  U8X8_DLY(250),
  U8X8_DLY(150),	/* extended, #318 */
 
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_il3820_v2_296x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_il3820_296x128_display_info);
      break;    
    case U8X8_MSG_DISPLAY_INIT:

      u8x8_d_helper_display_init(u8x8);
    
      u8x8_cad_SendSequence(u8x8, u8x8_d_il3820_v2_296x128_init_seq);    

      u8x8_cad_SendSequence(u8x8, u8x8_d_il3820_296x128_powersave0_seq);
      u8x8_d_il3820_first_init(u8x8);
      /* u8x8_d_il3820_second_init(u8x8); */  /* not required, u8g2.begin() will also clear the display once more */
          
      /* usually the DISPLAY_INIT message leaves the display in power save state */
      /* however this is not done for e-paper devices, see: */
      /* https://github.com/olikraus/u8g2/wiki/internal#powersave-mode */
    
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_il3820_296x128_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_il3820_296x128_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_d_il3820_draw_tile(u8x8, arg_int, arg_ptr);
      break;
    case U8X8_MSG_DISPLAY_REFRESH:
      u8x8_cad_SendSequence(u8x8, u8x8_d_il3820_v2_to_display_seq);
      break;
    default:
      return 0;
  }
  return 1;
}



