/*

  u8x8_d_ssd1606_172x72.c

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
  
  SSD1606: 128x180x2 
  two-bit, four graylevels
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


#define L(a,b,c,d) (((a)<<6)|((b)<<4)|((c)<<2)|(d))


/* GDE021A1, 2.1" EPD */
static const uint8_t u8x8_d_ssd1606_172x72_gde021a1_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  U8X8_CA(0x10, 0x00),	/* Deep Sleep mode Control: Disable */
  U8X8_CA(0x11, 0x03),	/* Define data entry mode, x&y inc, x first */
  U8X8_CAA(0x44, 0, 31),	/* RAM x start & end, each byte has 4 pixel, 32*4=128 */
  U8X8_CAA(0x45, 0, 179),	/* RAM y start & end, 179 MAX */
  
  U8X8_CA(0x4e, 0),	/* set x pos, 0..31 */
  U8X8_CA(0x4f, 0),	/* set y pos, 0...179 */

  U8X8_CA(0xf0, 0x1f),	/* set booster feedback to internal */
  U8X8_CA(0x22, 0xc0),	/* display update seq. option: enable clk, enable CP, .... todo: this is never activated */
  
  U8X8_C(0x32),	/* write LUT register*/

#ifdef ORIGINAL_LUT
  
  /* wavefrom part of the LUT: absolute LUT... this will always force the destination color */
  U8X8_A4(0x00,0x00,0x00,0x55),  /* step 0 */
  U8X8_A4(0x00,0x00,0x55,0x55),	/* step 1 */
  U8X8_A4(0x00,0x55,0x55,0x55),
  U8X8_A4(0xAA,0xAA,0xAA,0xAA),
  U8X8_A4(0x15,0x15,0x15,0x15),
  U8X8_A4(0x05,0x05,0x05,0x05),
  U8X8_A4(0x01,0x01,0x01,0x01),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),	/* step 19 */
  
  /* timing part of the LUT */
  U8X8_A8(0x22,0xFB,0x22,0x1B,0x00,0x00,0x00,0x00),
  U8X8_A(0x00),U8X8_A(0x00),

#else

  /* the following LUT will not change anything if the old and the new values are the same */
  /* 03 02 01 00	13 12 11 10 	23 22 21 20	33 32 31 30 				original */
  U8X8_A4(L(0, 0, 0, 0), 	L(0, 0, 0, 0), 	L(0, 0, 0, 0), 	L(0, 1, 1, 1)),		// 0x00,0x00,0x00,0x55,	step 0
  U8X8_A4(L(0, 0, 0, 0), 	L(0, 0, 0, 0), 	L(1, 0, 1, 1), 	L(0, 1, 1, 1)),		// 0x00,0x00,0x55,0x55,	step 1
  U8X8_A4(L(0, 0, 0, 0), 	L(1, 1, 0, 1), 	L(1, 0, 1, 1), 	L(0, 1, 1, 1)),		// 0x00,0x55,0x55,0x55,	step 2
  U8X8_A4(L(2, 2, 2, 0), 	L(2, 2, 0, 2), 	L(2, 0, 2, 2), 	L(0, 2, 2, 2)),		// 0xAA,0xAA,0xAA,0xAA,	step 3
  U8X8_A4(L(0, 1, 1, 0), 	L(0, 1, 0, 1), 	L(0, 0, 1, 1), 	L(0, 1, 1, 1)),		// 0x15,0x15,0x15,0x15,	step 4
  U8X8_A4(L(0, 0, 1, 0), 	L(0, 0, 0, 1), 	L(0, 0, 1, 1), 	L(0, 0, 1, 1)),		// 0x05,0x05,0x05,0x05,	step 5
  U8X8_A4(L(0, 0, 0, 0), 	L(0, 0, 0, 1), 	L(0, 0, 0, 1), 	L(0, 0, 0, 1)),		// 0x01,0x01,0x01,0x01,	step 6
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),
  U8X8_A4(0x00,0x00,0x00,0x00),	/* step 19 */
  
  /* timing part of the LUT */
  U8X8_A8(0x22,0xFB,0x22,0x1B,0x00,0x00,0x00,0x00),
  U8X8_A(0x00),U8X8_A(0x00),

#endif
  
  U8X8_CA(0x2c, 0xa0),	/* write vcom value*/
  U8X8_CA(0x3c, 0x63),	/* select boarder waveform */
  U8X8_CA(0x22, 0xc4),	/* display update seq. option: clk -> CP -> LUT -> initial display -> pattern display */
    /* 0x0c4 is mentioned in chapter 9.2 of the GDE021A1 data sheet */
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1606_to_display_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  
  //U8X8_CA(0x22, 0xc0),	/* display update seq. option: Enable clock and charge pump */
  //U8X8_C(0x20),	/* execute sequence */
  //U8X8_DLY(10),
  /* strange, splitting 0x0c0 does not work reliable */
  
  U8X8_CA(0x22, 0xc4),	/* display update seq. option: clk -> CP -> LUT -> initial display -> pattern display */
  U8X8_C(0x20),	/* execute sequence */
  U8X8_DLY(250),	/* the sequence above requires about 970ms */
  U8X8_DLY(250),
  U8X8_DLY(250),
  U8X8_DLY(230),
  
  U8X8_CA(0x22, 0x03),	/* disable clock and charge pump */
  U8X8_DLY(200),		/* this requres about 270ms */
  U8X8_DLY(90),  
  
  //U8X8_CA(0x10, 0x01), /* deep sleep mode */
  //U8X8_C(0x20), 		/* execute sequence */
  U8X8_DLY(50),  
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


// static const uint8_t u8x8_d_ssd1606_172x72_powersave0_seq[] = {
//   U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
//   U8X8_END_TRANSFER(),             	/* disable chip */
//   U8X8_END()             			/* end of sequence */
// };


// static const uint8_t u8x8_d_ssd1606_172x72_powersave1_seq[] = {
//   U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
//   U8X8_END_TRANSFER(),             	/* disable chip */
//   U8X8_END()             			/* end of sequence */
// };

// static const uint8_t u8x8_d_ssd1606_172x72_flip0_seq[] = {
//   U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
//   U8X8_END_TRANSFER(),             	/* disable chip */
//   U8X8_END()             			/* end of sequence */
// };

// static const uint8_t u8x8_d_ssd1606_172x72_flip1_seq[] = {
//   U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
//   U8X8_END_TRANSFER(),             	/* disable chip */
//   U8X8_END()             			/* end of sequence */
// };


static uint8_t *u8x8_convert_tile_for_ssd1606(uint8_t *t)
{
  uint8_t i;
  uint16_t r;
  static uint8_t buf[16];
  uint8_t *pbuf = buf;

  for( i = 0; i < 8; i++ )
  {
    r = u8x8_upscale_byte(~(*t++));
    *pbuf++ = (r>>8) & 255;
    *pbuf++ = r & 255;
  }
  return buf;
}

static void u8x8_d_ssd1606_draw_tile(u8x8_t *u8x8, uint8_t arg_int, void *arg_ptr) U8X8_NOINLINE;
static void u8x8_d_ssd1606_draw_tile(u8x8_t *u8x8, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c, page;
  uint8_t *ptr;
  u8x8_cad_StartTransfer(u8x8);

  page = u8x8->display_info->tile_height;
  page --;
  page -= (((u8x8_tile_t *)arg_ptr)->y_pos);
  page *= 2;


  x = ((u8x8_tile_t *)arg_ptr)->x_pos;
  x *= 8;
  x += u8x8->x_offset;

  u8x8_cad_SendCmd(u8x8, 0x00f );	/* scan start */
  u8x8_cad_SendArg(u8x8, 0);

  u8x8_cad_SendCmd(u8x8, 0x011 );	/* cursor increment mode */
  u8x8_cad_SendArg(u8x8, 3);

  u8x8_cad_SendCmd(u8x8, 0x045 );	/* window start column */
  u8x8_cad_SendArg(u8x8, 0);
  u8x8_cad_SendArg(u8x8, 179);		/* end of display */

  u8x8_cad_SendCmd(u8x8, 0x044 );	/* window end page */
  u8x8_cad_SendArg(u8x8, page);
  u8x8_cad_SendArg(u8x8, page+1);

  u8x8_cad_SendCmd(u8x8, 0x04f );	/* window column */
  u8x8_cad_SendArg(u8x8, x);

  u8x8_cad_SendCmd(u8x8, 0x04e );	/* window row */
  u8x8_cad_SendArg(u8x8, page);

  u8x8_cad_SendCmd(u8x8, 0x024 );
  
  do
  {
    c = ((u8x8_tile_t *)arg_ptr)->cnt;
    ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
    do
    {
      u8x8_cad_SendData(u8x8, 16, u8x8_convert_tile_for_ssd1606(ptr));
      ptr += 8;
      x += 8;
      c--;
    } while( c > 0 );
    
    arg_int--;
  } while( arg_int > 0 );
  
  u8x8_cad_EndTransfer(u8x8);
}


static uint8_t u8x8_d_ssd1606_172x72_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1606_172x72_display_info);
      break;
    */
    case U8X8_MSG_DISPLAY_INIT:

      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1606_172x72_gde021a1_init_seq);    
    
      /* special code for the SSD1606... */
      /* ensure that the initial buffer is clear and all eInk is set to white */
      /* this is done here, because the LUT will be of that kind, that it uses the previous color */
      /* make everything black */
      u8x8_FillDisplay(u8x8);		
      /* write content to the display */
      u8x8_RefreshDisplay(u8x8);
      /* now make everything clear */
      u8x8_FillDisplay(u8x8);		
      /* write content to the display */
      u8x8_RefreshDisplay(u8x8);
      /* now make everything clear */
      u8x8_ClearDisplay(u8x8);		
      /* write content to the display */
      u8x8_RefreshDisplay(u8x8);

      u8x8_ClearDisplay(u8x8);		
      /* write content to the display */
      u8x8_RefreshDisplay(u8x8);
    
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
/*
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1606_172x72_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1606_172x72_powersave1_seq);
*/
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
/*
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1606_172x72_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1606_172x72_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
*/
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
/*
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_EndTransfer(u8x8);
*/
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_d_ssd1606_draw_tile(u8x8, arg_int, arg_ptr);
      break;
    case U8X8_MSG_DISPLAY_REFRESH:
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1606_to_display_seq);    
      break;
    default:
      return 0;
  }
  return 1;
}


static const u8x8_display_info_t u8x8_ssd1606_172x72_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
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
  /* tile_width = */ 22,		/* 22*8 = 176 */
  /* tile_hight = */ 9,		/* 9*8 = 72 */
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 172,
  /* pixel_height = */ 72		
};

uint8_t u8x8_d_ssd1606_172x72(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
    {
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1606_172x72_display_info);
      return 1;
    }
    return u8x8_d_ssd1606_172x72_generic(u8x8, msg, arg_int, arg_ptr);
}


