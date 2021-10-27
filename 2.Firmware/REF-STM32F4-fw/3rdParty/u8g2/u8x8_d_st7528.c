/*

  u8x8_d_st7528.c

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2019, olikraus@gmail.com
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
  
  ST7528: 16 Graylevel Controller
  https://github.com/olikraus/u8g2/issues/986
  I2C Address: 0x03f (0x7e)
  
  
  ERC16004
  https://www.buydisplay.com/default/2-inch-lcd-160x64-graphic-module-serial-spi-display-st7528-black-on-white
  
*/


#include "u8x8.h"


static const uint8_t u8x8_d_st7528_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x038, 0x074),		/* ext mode 0*/
  U8X8_C(0x0af),		                /* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7528_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x038, 0x074),		/* ext mode 0*/
  U8X8_C(0x0ae),		                /* display off */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7528_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a0),		                /* ADC */
  U8X8_C(0x0c8),		                /* SHL */  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7528_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a1),		                /* ADC */
  U8X8_C(0x0c0),		                /* SHL */  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


/*
  input:
    one tile (8 Bytes)
  output:
    Tile for st7528 (32 Bytes)
*/

static uint8_t u8x8_st7528_8to32_dest_buf[32];

static uint8_t *u8x8_st7528_8to32(U8X8_UNUSED u8x8_t *u8x8, uint8_t *ptr)
{
  uint8_t j;
  uint8_t *dest;
  
  dest = u8x8_st7528_8to32_dest_buf;
  for( j = 0; j < 8; j++ )
  {
    *dest++ =*ptr;
    *dest++ =*ptr;
    *dest++ =*ptr;
    *dest++ =*ptr;
    ptr++;
  }
  return u8x8_st7528_8to32_dest_buf;
}



static uint8_t u8x8_d_st7528_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x;
  uint8_t y, c;
  uint8_t *ptr;
  switch(msg)
  {
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7528_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_st7528_nhd_c160100_init_seq);    
    */
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7528_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7528_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7528_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7528_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int );	/* ssd1326 has range from 0 to 255 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;  // not clear
      
      y = (((u8x8_tile_t *)arg_ptr)->y_pos);
      
    
      
      do
      {
	c = ((u8x8_tile_t *)arg_ptr)->cnt;
	ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;

	do
	{
          u8x8_cad_SendCmd(u8x8, 0xb0 | y );	/* set page address */
          u8x8_cad_SendCmd(u8x8, 0x10| (x>>4) );	/* set col msb*/
          u8x8_cad_SendCmd(u8x8, 0x00| (x&15) );	/* set col lsb*/
          
          u8x8_cad_SendData(u8x8, 32, u8x8_st7528_8to32(u8x8, ptr));
          
	  ptr += 8;
	  x += 8;
	  c--;
	} while( c > 0 );	
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}


static void u8x8_d_st7528_graylevel_init(u8x8_t *u8x8, uint8_t mode0)
{
  uint8_t i;
  
  u8x8_cad_StartTransfer(u8x8);
  u8x8_cad_SendCmd(u8x8, 0x38 );
  u8x8_cad_SendArg(u8x8, mode0+1 );
  for( i = 0; i < 64; i++ )
  {
          u8x8_cad_SendCmd(u8x8, i+0x080 );
          u8x8_cad_SendArg(u8x8, i & 0xfc);
  }
  u8x8_cad_SendCmd(u8x8, 0x38 );
  u8x8_cad_SendArg(u8x8, mode0 );
  u8x8_cad_EndTransfer(u8x8);
}

/*===============================================================*/
/* NHD C160100 */
static const uint8_t u8x8_d_st7528_nhd_c160100_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  
/*
    I2C_out(0x48);//partial display duty ratio
    I2C_out(0x64);// 1/100 duty
    I2C_out(0xA0);//ADC select
    I2C_out(0xC8);//SHL select
    I2C_out(0x44);//initial Com0 register
    I2C_out(0x00);//scan from Com0
    I2C_out(0xAB);//OSC on
    I2C_out(0x26);//
    I2C_out(0x81);	//set electronic volume
    I2C_out(0x15);//vopcode=0x1C
    I2C_out(0x56);//set 1/11 bias
    I2C_out(0x64);//3x
    delay(2);
    I2C_out(0x2C);//
    I2C_out(0x66);//5x
    delay(2);
    I2C_out(0x2E);//
    delay(2);
    I2C_out(0x2F);//power control
    I2C_out(0xF3);//bias save circuit
    I2C_out(0x00);//
    I2C_out(0x96);//frc and pwm
    I2C_out(0x38);//external mode
    I2C_out(0x75);//
    I2C_out(0x97);//3frc, 45 pwm			THIS IS A MODE0 CMD, IT IS USELESS HERE
    I2C_out(0x80);//start 16-level grayscale settings
*/
  U8X8_CA(0x038, 0x064),		/* ext mode 0*/
  U8X8_CA(0x048, 0x064),		/* partial display duty ratio, 1/100 duty*/
  U8X8_C(0x0a0),		                /* ADC */
  U8X8_C(0x0c8),		                /* SHL */
  U8X8_CA(0x044, 0x000),		/* initial Com0 */
  U8X8_C(0x0ab),		                /* start oscillator */
  U8X8_C(0x026),		                /* Select the internal resistance ratio of the regulator resistor */
  U8X8_CA(0x081, 0x015),		/* volumn */
  U8X8_C(0x056),		                /* LCD Bias */
  U8X8_C(0x064),		                /* DC DC step up */
  U8X8_DLY(2),
  U8X8_C(0x02c),		                /* Power Control */
  U8X8_C(0x066),		                /* DC DC step up */
  U8X8_DLY(2),
  U8X8_C(0x02e),		                /* Power Control */
  U8X8_DLY(2),
  U8X8_C(0x02f),		                /* Power Control */
  U8X8_CA(0x0f3, 0x000),		/* bias power save */
  U8X8_C(0x096),		                /* frc and pwm */

  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};



static const u8x8_display_info_t u8x8_st7528_160x100_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 20,
  /* reset_pulse_width_ms = */ 5, 	
  /* post_reset_wait_ms = */ 5, 		/**/
  /* sda_setup_time_ns = */ 20,		/* st7528  */
  /* sck_pulse_width_ns = */ 25,	/* st7528 */
  /* sck_clock_hz = */ 8000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
      /* st7528 actually allows 20MHz according to the datasheet */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 80,	/* st7528 */
  /* tile_width = */ 20,
  /* tile_hight = */ 13,
  /* default_x_offset = */ 0,		/* x_offset is used as y offset for the ssd1326 */
  /* flipmode_x_offset = */ 0,		/* x_offset is used as y offset for the ssd1326 */
  /* pixel_width = */ 160,
  /* pixel_height = */ 100
};

uint8_t u8x8_d_st7528_nhd_c160100(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
    {
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7528_160x100_display_info);
      return 1;
    }
    if ( msg == U8X8_MSG_DISPLAY_INIT )
    {
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_st7528_nhd_c160100_init_seq);    
      u8x8_d_st7528_graylevel_init(u8x8, 0x074);
      return 1;
    }    
    return u8x8_d_st7528_generic(u8x8, msg, arg_int, arg_ptr);
}

/*===============================================================*/
/* ERC16064, https://www.buydisplay.com/2-inch-lcd-160x64-graphic-module-serial-spi-display-st7528-black-on-white */

/*

#define	ModeSet				0x38
#define	ModeSetP1			0x64	//EXT=0
#define	ModeSetP2			0x65	//EXT=1

#define Display_on 			0xaf
#define Display_off 		0xae

#define Regulator			0x26
#define Contrast_level		0x81
#define PowerControll_on1	0x2c
#define PowerControll_on2	0x2e
#define PowerControll_on3	0x2f

#define	LcdBias_9			0x54
#define	InterOsc_on			0xab
#define	EntireDisp_on		0xa5
#define	EntireDisp_off		0xa4
#define BoostLevel_5		0x66
#define	Duty_set			0x48
#define	Duty_setP			0x40
#define Start_columnlsb		0x00
#define Start_columnmsb		0x10
#define Start_page			0xb0
#define	StartLine_set		0x40
//#define	StartLine_setP		0x00
#define Set_Initial_COM0	0x44
#define Set_Initial_COM0P	0x12


#define Entire_Displa_ON	0xA5
#define Entire_Displa_OFF	0xA4

#define FrcPwm_set			0x92
#define NLineInversion_on	0x4c
#define NLineInversion_onP	0x1f
#define NLineInversion_off	0xe4	
#define ReverseDisp_on		0xa7
#define ReverseDisp_off		0xa6
#define AdcSelect			0xa0
#define ComScanDirection	0xc8

	Write_command(ModeSet);		0x38, 0x64
	Write_command(ModeSetP1);	
	Write_command(InterOsc_on);	0xab
	Write_command(Set_Initial_COM0);	0x44, 0x12
	Write_command(Set_Initial_COM0P);

	Write_command(AdcSelect);		0xa0
	Write_command(ComScanDirection);	0xc8
	Write_command(BoostLevel_5);		0x66
	Delay(1000);
	Write_command(LcdBias_9);	0x54
	Write_command(Duty_set);		0x48, 0x40
	Write_command(Duty_setP);
	Write_command(Regulator);	0x26
	Write_command(Contrast_level);		0x81, 0x0b
	Write_command(Contrast_levelP);
	Write_command(PowerControll_on1);	0x2c
	Delay(10000);
	Write_command(PowerControll_on2);	0x2e
	Delay(1000);
	Write_command(PowerControll_on3);	0x2f
	Delay(1000);	
	Write_command(FrcPwm_set);	0x92

	Write_command(ModeSet);
	Write_command(ModeSetP2);
	for(i=0;i<64;i++)
		{
		Write_command(0x80+i);
		Write_command(Gray_Parameters[i]);
		}
	Write_command(ModeSet);
	Write_command(ModeSetP1);
*/

static const uint8_t u8x8_d_st7528_erc16064_init_seq[] = {    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x038, 0x064),		/* ext mode 0*/
  U8X8_C(0x0ab),		                /* start oscillator */
  U8X8_CA(0x044, 0x012),		/* initial Com0 */
  U8X8_C(0x0a0),		                /* ADC */
  U8X8_C(0x0c8),		                /* SHL */  
  U8X8_C(0x066),		                /* boost level, ERC16064: 0x066 */
  U8X8_DLY(1),
  U8X8_C(0x054),		                /* LCD Bias, ERC16064: 0x054 */
  U8X8_CA(0x048, 0x040),		/* partial display duty ratio */  
  U8X8_C(0x026),		                /* Select the internal resistance ratio of the regulator resistor */  
  U8X8_CA(0x081, 0x00b),		/* contrast,  ERC16064: 0x00b */
  
  U8X8_C(0x02c),		                /* Power Control */
  U8X8_DLY(2),
  U8X8_C(0x02e),		                /* Power Control */
  U8X8_DLY(2),
  U8X8_C(0x02f),		                /* Power Control */
  U8X8_DLY(2),
  U8X8_C(0x092),		                /* frc and pwm, ERC160624: 0x092 */

  

  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const u8x8_display_info_t u8x8_st7528_erc16064_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 20,
  /* reset_pulse_width_ms = */ 5, 	
  /* post_reset_wait_ms = */ 5, 		/**/
  /* sda_setup_time_ns = */ 20,		/* st7528  */
  /* sck_pulse_width_ns = */ 25,	/* st7528 */
  /* sck_clock_hz = */ 8000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
      /* st7528 actually allows 20MHz according to the datasheet */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 80,	/* st7528 */
  /* tile_width = */ 20,
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,		/* x_offset is used as y offset for the ssd1326 */
  /* flipmode_x_offset = */ 0,		/* x_offset is used as y offset for the ssd1326 */
  /* pixel_width = */ 160,
  /* pixel_height = */ 64
};


uint8_t u8x8_d_st7528_erc16064(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
    {
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7528_erc16064_display_info);
      return 1;
    }
    if ( msg == U8X8_MSG_DISPLAY_INIT )
    {
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_st7528_erc16064_init_seq);    
      u8x8_d_st7528_graylevel_init(u8x8, 0x064);
      return 1;
    }    
    return u8x8_d_st7528_generic(u8x8, msg, arg_int, arg_ptr);
}


