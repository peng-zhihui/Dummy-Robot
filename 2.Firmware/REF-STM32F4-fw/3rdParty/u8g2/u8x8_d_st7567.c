/*

  u8x8_d_st7567.c
  
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




static const uint8_t u8x8_d_st7567_132x64_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a4),		                /* all pixel off, issue 142 */
  U8X8_C(0x0af),		                /* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7567_132x64_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x0a5),		                /* enter powersafe: all pixel on, issue 142 */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7567_132x64_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a1),				/* segment remap a0/a1*/
  U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7567_132x64_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a0),				/* segment remap a0/a1*/
  U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7567_n_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a0),				/* segment remap a0/a1*/
  U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7567_n_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a1),				/* segment remap a0/a1*/
  U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};



/*=====================================================*/


static const u8x8_display_info_t u8x8_st7567_132x64_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 150,	/* */
  /* pre_chip_disable_wait_ns = */ 50,	/* */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 1, 
  /* sda_setup_time_ns = */ 50,		/* */
  /* sck_pulse_width_ns = */ 120,	/* */
  /* sck_clock_hz = */ 4000000UL,	/* */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,	/* */
  /* write_pulse_width_ns = */ 80,	/* */
  /* tile_width = */ 17,		/* width of 17*8=136 pixel */
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 132,
  /* pixel_height = */ 64
};

static const uint8_t u8x8_d_st7567_132x64_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_C(0x0e2),            			/* soft reset */
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x040),		                /* set display start line to 0 */
  
  U8X8_C(0x0a1),		                /* ADC set to reverse */
  U8X8_C(0x0c0),		                /* common output mode */
  // Flipmode
  //U8X8_C(0x0a0),		                /* ADC set to reverse */
  //U8X8_C(0x0c8),		                /* common output mode */
  
  U8X8_C(0x0a6),		                /* display normal, bit val 0: LCD pixel off. */
  U8X8_C(0x0a3),		                /* LCD bias 1/7 */
  /* power on sequence from paxinstruments */
  U8X8_C(0x028|4),		                /* all power  control circuits on */
  U8X8_DLY(50),
  U8X8_C(0x028|6),		                /* all power  control circuits on */
  U8X8_DLY(50),
  U8X8_C(0x028|7),		                /* all power  control circuits on */
  U8X8_DLY(50),
  
  U8X8_C(0x026),		                /* v0 voltage resistor ratio */
  U8X8_CA(0x081, 0x027),		/* set contrast, contrast value*/
  
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x0a5),		                /* enter powersafe: all pixel on, issue 142 */
   
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

/* pax instruments 132x64 display */
uint8_t u8x8_d_st7567_pi_132x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7567_132x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int >> 2 );	/* st7567 has range from 0 to 63 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
    
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;
      x += u8x8->x_offset;
      u8x8_cad_SendCmd(u8x8, 0x010 | (x>>4) );
      u8x8_cad_SendCmd(u8x8, 0x000 | ((x&15)));
      u8x8_cad_SendCmd(u8x8, 0x0b0 | (((u8x8_tile_t *)arg_ptr)->y_pos));
    
      c = ((u8x8_tile_t *)arg_ptr)->cnt;
      c *= 8;
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
      /* 
	The following if condition checks the hardware limits of the st7567 
	controller: It is not allowed to write beyond the display limits.
	This is in fact an issue within flip mode.
      */
      if ( c + x > 132u )
      {
	c = 132u;
	c -= x;
      }
      do
      {
	u8x8_cad_SendData(u8x8, c, ptr);	/* note: SendData can not handle more than 255 bytes */
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}




/*=====================================================*/





static const u8x8_display_info_t u8x8_st7567_jlx12864_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 150,	/* */
  /* pre_chip_disable_wait_ns = */ 50,	/* */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 1, 
  /* sda_setup_time_ns = */ 50,		/* */
  /* sck_pulse_width_ns = */ 120,	/* */
  /* sck_clock_hz = */ 4000000UL,	/* */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,	/* */
  /* write_pulse_width_ns = */ 80,	/* */
  /* tile_width = */ 16,		/* width of 16*8=128 pixel */
  /* tile_hight = */ 8,
  /* default_x_offset = */ 4,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 128,
  /* pixel_height = */ 64
};

static const uint8_t u8x8_st7567_jlx12864_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_C(0x0e2),            			/* soft reset */
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x040),		                /* set display start line to 0 */
  
  U8X8_C(0x0a1),		                /* ADC set to reverse */
  U8X8_C(0x0c0),		                /* common output mode */
  // Flipmode
  //U8X8_C(0x0a0),		                /* ADC set to reverse */
  //U8X8_C(0x0c8),		                /* common output mode */
  
  U8X8_C(0x0a6),		                /* display normal, bit val 0: LCD pixel off. */
  U8X8_C(0x0a3),		                /* LCD bias 1/7 */
  /* power on sequence from paxinstruments */
  U8X8_C(0x028|4),		                /* all power  control circuits on */
  U8X8_DLY(50),
  U8X8_C(0x028|6),		                /* all power  control circuits on */
  U8X8_DLY(50),
  U8X8_C(0x028|7),		                /* all power  control circuits on */
  U8X8_DLY(50),
  
  U8X8_C(0x023),		                /* v0 voltage resistor ratio */
  U8X8_CA(0x081, 42>>2),		/* set contrast, contrast value*/
  
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x0a5),		                /* enter powersafe: all pixel on, issue 142 */
   
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

/* JLX12864 display */
uint8_t u8x8_d_st7567_jlx12864(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7567_jlx12864_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_st7567_jlx12864_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int >> 2 );	/* st7567 has range from 0 to 63 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
    
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;
      x += u8x8->x_offset;
      u8x8_cad_SendCmd(u8x8, 0x010 | (x>>4) );
      u8x8_cad_SendCmd(u8x8, 0x000 | ((x&15)));
      u8x8_cad_SendCmd(u8x8, 0x0b0 | (((u8x8_tile_t *)arg_ptr)->y_pos));
    
      c = ((u8x8_tile_t *)arg_ptr)->cnt;
      c *= 8;
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
      /* 
	The following if condition checks the hardware limits of the st7567 
	controller: It is not allowed to write beyond the display limits.
	This is in fact an issue within flip mode.
      */
      if ( c + x > 132u )
      {
	c = 132u;
	c -= x;
      }
      do
      {
	u8x8_cad_SendData(u8x8, c, ptr);	/* note: SendData can not handle more than 255 bytes */
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}


/*=====================================================*/



static const u8x8_display_info_t u8x8_st7567_enh_dg128064_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 150,	/* */
  /* pre_chip_disable_wait_ns = */ 50,	/* */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 1, 
  /* sda_setup_time_ns = */ 50,		/* */
  /* sck_pulse_width_ns = */ 120,	/* */
  /* sck_clock_hz = */ 4000000UL,	/* */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,	/* */
  /* write_pulse_width_ns = */ 80,	/* */
  /* tile_width = */ 16,		/* width of 16*8=128 pixel */
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 4,
  /* pixel_width = */ 128,
  /* pixel_height = */ 64
};

static const u8x8_display_info_t u8x8_st7567_enh_dg128064i_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 150,	/* */
  /* pre_chip_disable_wait_ns = */ 50,	/* */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 1, 
  /* sda_setup_time_ns = */ 50,		/* */
  /* sck_pulse_width_ns = */ 120,	/* */
  /* sck_clock_hz = */ 4000000UL,	/* */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,	/* */
  /* write_pulse_width_ns = */ 80,	/* */
  /* tile_width = */ 16,		/* width of 16*8=128 pixel */
  /* tile_hight = */ 8,
  /* default_x_offset = */ 4,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 128,
  /* pixel_height = */ 64
};

static const uint8_t u8x8_st7567_enh_dg128064_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_C(0x0e2),            			/* soft reset */
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x040),		                /* set display start line to 0 */
  
  U8X8_C(0x0a1),		                /* ADC set to reverse */
  U8X8_C(0x0c0),		                /* common output mode */
  // Flipmode
  //U8X8_C(0x0a0),		                /* ADC set to reverse */
  //U8X8_C(0x0c8),		                /* common output mode */
  
  U8X8_C(0x0a6),		                /* display normal, bit val 0: LCD pixel off. */
  U8X8_C(0x0a2),		                /* LCD bias 1/9 */
  /* power on sequence from paxinstruments */
  U8X8_C(0x028|4),		                /* all power  control circuits on */
  U8X8_DLY(50),
  U8X8_C(0x028|6),		                /* all power  control circuits on */
  U8X8_DLY(50),
  U8X8_C(0x028|7),		                /* all power  control circuits on */
  U8X8_DLY(50),
  
  U8X8_C(0x023),		                /* v0 voltage resistor ratio */
  U8X8_CA(0x081, 200>>2),		/* set contrast, contrast value*/
  
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x0a5),		                /* enter powersafe: all pixel on, issue 142 */
   
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

/* ENH-DG128064 transparent display */
static uint8_t u8x8_d_st7567_enh_dg128064_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7567_enh_dg128064_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_st7567_enh_dg128064_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_powersave1_seq);
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int >> 2 );	/* st7567 has range from 0 to 63 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
    
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;
      x += u8x8->x_offset;
      u8x8_cad_SendCmd(u8x8, 0x010 | (x>>4) );
      u8x8_cad_SendCmd(u8x8, 0x000 | ((x&15)));
      u8x8_cad_SendCmd(u8x8, 0x0b0 | (((u8x8_tile_t *)arg_ptr)->y_pos));
    
      c = ((u8x8_tile_t *)arg_ptr)->cnt;
      c *= 8;
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
      /* 
	The following if condition checks the hardware limits of the st7567 
	controller: It is not allowed to write beyond the display limits.
	This is in fact an issue within flip mode.
      */
      if ( c + x > 132u )
      {
	c = 132u;
	c -= x;
      }
      do
      {
	u8x8_cad_SendData(u8x8, c, ptr);	/* note: SendData can not handle more than 255 bytes */
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}

uint8_t u8x8_d_st7567_enh_dg128064(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7567_enh_dg128064_display_info);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_n_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_n_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
    default:
      return u8x8_d_st7567_enh_dg128064_generic(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}

uint8_t u8x8_d_st7567_enh_dg128064i(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7567_enh_dg128064i_display_info);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
    default:
      return u8x8_d_st7567_enh_dg128064_generic(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}


/*=====================================================*/
/* issue 657 */

static const u8x8_display_info_t u8x8_st7567_64x32_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 150,	/* */
  /* pre_chip_disable_wait_ns = */ 50,	/* */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 1, 
  /* sda_setup_time_ns = */ 50,		/* */
  /* sck_pulse_width_ns = */ 120,	/* */
  /* sck_clock_hz = */ 4000000UL,	/* */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,	/* */
  /* write_pulse_width_ns = */ 80,	/* */
  /* tile_width = */ 8,		
  /* tile_hight = */ 4,
  /* default_x_offset = */ 32,
  /* flipmode_x_offset = */ 32,
  /* pixel_width = */ 64,
  /* pixel_height = */ 32
};

static const uint8_t u8x8_st7567_64x32_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_C(0x0e2),            			/* soft reset */
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x040),		                /* set display start line to 0 */
  
  U8X8_C(0x0a1),		                /* ADC */
  U8X8_C(0x0c0),		                /* common output mode */
  // Flipmode
  //U8X8_C(0x0a0),		                /* ADC  */
  //U8X8_C(0x0c8),		                /* common output mode */
  
  U8X8_C(0x0a6),		                /* display normal, bit val 0: LCD pixel off. */
  U8X8_C(0x0a2),		                /* LCD bias 1/9 */
  U8X8_C(0x028|4),		                /* all power  control circuits on */
  U8X8_DLY(50),
  U8X8_C(0x028|6),		                /* all power  control circuits on */
  U8X8_DLY(50),
  U8X8_C(0x028|7),		                /* all power  control circuits on */
  U8X8_DLY(50),
  
  U8X8_C(0x024),		                /* v0 voltage resistor ratio, taken from issue 657 */
  U8X8_CA(0x081, 0x020),		/* set contrast, contrast value*/
  /* 18 Apr 2020: the value 0x080 does not make sense, only 6 bit are supported
  for contrast, changed to 0x040 */
  
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x0a5),		                /* enter powersafe: all pixel on, issue 142 */
   
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_st7567_64x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7567_64x32_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_st7567_64x32_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int >> 2 );	/* st7567 has range from 0 to 63 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
    
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;
      x += u8x8->x_offset;
      u8x8_cad_SendCmd(u8x8, 0x010 | (x>>4) );
      u8x8_cad_SendCmd(u8x8, 0x000 | ((x&15)));
      u8x8_cad_SendCmd(u8x8, 0x0b0 | (((u8x8_tile_t *)arg_ptr)->y_pos));
    
      c = ((u8x8_tile_t *)arg_ptr)->cnt;
      c *= 8;
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
      /* 
	The following if condition checks the hardware limits of the st7567 
	controller: It is not allowed to write beyond the display limits.
	This is in fact an issue within flip mode.
      */
      if ( c + x > 132u )
      {
	c = 132u;
	c -= x;
      }
      do
      {
	u8x8_cad_SendData(u8x8, c, ptr);	/* note: SendData can not handle more than 255 bytes */
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}

/*=====================================================*/
/* issue 1159, Lummax HEM6432-03 */


static const u8x8_display_info_t u8x8_st7567_hem6432_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 150,	/* */
  /* pre_chip_disable_wait_ns = */ 50,	/* */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 1, 
  /* sda_setup_time_ns = */ 50,		/* */
  /* sck_pulse_width_ns = */ 120,	/* */
  /* sck_clock_hz = */ 4000000UL,	/* */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,	/* */
  /* write_pulse_width_ns = */ 80,	/* */
  /* tile_width = */ 8,		
  /* tile_hight = */ 4,
  /* default_x_offset = */ 36,		/* issue 1159 */
  /* flipmode_x_offset = */ 32,		/* issue 1159 */
  /* pixel_width = */ 64,
  /* pixel_height = */ 32
};

static const uint8_t u8x8_st7567_hem6432_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_C(0x0e2),            			/* soft reset */
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x040),		                /* set display start line to 0 */
  
  U8X8_C(0x0a1),		                /* ADC */
  U8X8_C(0x0c0),		                /* common output mode */
  // Flipmode
  //U8X8_C(0x0a0),		                /* ADC  */
  //U8X8_C(0x0c8),		                /* common output mode */
  
  U8X8_C(0x0a6),		                /* display normal, bit val 0: LCD pixel off. */
  U8X8_C(0x0a2),		                /* LCD bias 1/9 */
  U8X8_C(0x028|4),		                /* all power  control circuits on */
  U8X8_DLY(50),
  U8X8_C(0x028|6),		                /* all power  control circuits on */
  U8X8_DLY(50),
  U8X8_C(0x028|7),		                /* all power  control circuits on */
  U8X8_DLY(50),
  
  U8X8_C(0x024),		                /* v0 voltage resistor ratio, taken from issue 657 */
  U8X8_CA(0x081, 225/4),		/* set contrast, contrast value as suggested inissue 1159 */
  
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x0a5),		                /* enter powersafe: all pixel on, issue 142 */
   
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_st7567_hem6432(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7567_hem6432_display_info);
      u8x8->i2c_address = 0x07e;  /* issue 1159, use different i2c address */
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_st7567_hem6432_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int >> 2 );	/* st7567 has range from 0 to 63 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
    
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;
      x += u8x8->x_offset;
      u8x8_cad_SendCmd(u8x8, 0x010 | (x>>4) );
      u8x8_cad_SendCmd(u8x8, 0x000 | ((x&15)));
      u8x8_cad_SendCmd(u8x8, 0x0b0 | (((u8x8_tile_t *)arg_ptr)->y_pos));
    
      c = ((u8x8_tile_t *)arg_ptr)->cnt;
      c *= 8;
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
      /* 
	The following if condition checks the hardware limits of the st7567 
	controller: It is not allowed to write beyond the display limits.
	This is in fact an issue within flip mode.
      */
      if ( c + x > 132u )
      {
	c = 132u;
	c -= x;
      }
      do
      {
	u8x8_cad_SendData(u8x8, c, ptr);	/* note: SendData can not handle more than 255 bytes */
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}


/*=====================================================*/
/*
  https://github.com/olikraus/u8g2/issues/1088 
  https://www.dx.com/p/opensmart-33v-26-inch-128x64-serial-spi-monochrome-lcd-breakout-board-module-with-backlight-for-arduino-nano-pro-mini-2710499.html
*/





static const u8x8_display_info_t u8x8_st7567_os12864_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 150,	/* */
  /* pre_chip_disable_wait_ns = */ 50,	/* */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 1, 
  /* sda_setup_time_ns = */ 50,		/* */
  /* sck_pulse_width_ns = */ 120,	/* */
  /* sck_clock_hz = */ 4000000UL,	/* */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,	/* */
  /* write_pulse_width_ns = */ 80,	/* */
  /* tile_width = */ 16,		/* width of 16*8=128 pixel */
  /* tile_hight = */ 8,
  /* default_x_offset = */ 4,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 128,
  /* pixel_height = */ 64
};

static const uint8_t u8x8_st7567_os12864_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_C(0x0e2),            			/* soft reset */
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x040),		                /* set display start line to 0 */
  
  U8X8_C(0x0a1),		                /* ADC set to reverse */
  U8X8_C(0x0c0),		                /* common output mode */
  // Flipmode
  //U8X8_C(0x0a0),		                /* ADC set to reverse */
  //U8X8_C(0x0c8),		                /* common output mode */
  
  U8X8_C(0x0a6),		                /* display normal, bit val 0: LCD pixel off. */
  U8X8_C(0x0a3),		                /* LCD bias 1/7 */
  /* power on sequence from paxinstruments */
  U8X8_C(0x028|4),		                /* all power  control circuits on */
  U8X8_DLY(50),
  U8X8_C(0x028|6),		                /* all power  control circuits on */
  U8X8_DLY(50),
  U8X8_C(0x028|7),		                /* all power  control circuits on */
  U8X8_DLY(50),
  
  U8X8_C(0x026),		                /* v0 voltage resistor ratio */
  U8X8_CA(0x081, 50>>2),		/* set contrast, contrast value*/
  
  U8X8_C(0x0ae),		                /* display off */
  U8X8_C(0x0a5),		                /* enter powersafe: all pixel on, issue 142 */
   
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

/* open-smart 12864 display */
uint8_t u8x8_d_st7567_os12864(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7567_os12864_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_st7567_os12864_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7567_132x64_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int >> 2 );	/* st7567 has range from 0 to 63 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
    
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;
      x += u8x8->x_offset;
      u8x8_cad_SendCmd(u8x8, 0x010 | (x>>4) );
      u8x8_cad_SendCmd(u8x8, 0x000 | ((x&15)));
      u8x8_cad_SendCmd(u8x8, 0x0b0 | (((u8x8_tile_t *)arg_ptr)->y_pos));
    
      c = ((u8x8_tile_t *)arg_ptr)->cnt;
      c *= 8;
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
      /* 
	The following if condition checks the hardware limits of the st7567 
	controller: It is not allowed to write beyond the display limits.
	This is in fact an issue within flip mode.
      */
      if ( c + x > 132u )
      {
	c = 132u;
	c -= x;
      }
      do
      {
	u8x8_cad_SendData(u8x8, c, ptr);	/* note: SendData can not handle more than 255 bytes */
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}


