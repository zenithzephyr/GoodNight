/****************************************Copyright (c)**************************************************
**
**                                 http://www.powermcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			GLCD.h
** Descriptions:		TFT (IO)
**
**------------------------------------------------------------------------------------------------------
** Created by:			AVRman
** Created date:		2011-1-26
** Version:				1.0
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <asf.h>
#include "GLCD.h"
#include "AsciiLib.h"

/* Private variables ---------------------------------------------------------*/
static uint8_t LCD_Code;

/* Private define ------------------------------------------------------------*/
#define  ILI9320    0  /* 0x9320 */
#define  ILI9325    1  /* 0x9325 */
#define  ST7781     3  /* 0x7783 */
#define  LGDP4531   4  /* 0x4531 */
#define  SPFD5408B  5  /* 0x5408 */
#define  R61505U    6  /* 0x1505 0x0505 */
#define  HX8347A    7  /* 0x0047 */
#define  LGDP4535   8  /* 0x4535 */

/*******************************************************************************
* Function Name  : Lcd_Configuration
* Description    : Configures LCD Control lines
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_Configuration(void)
{

}

/*******************************************************************************
* Function Name  : LCD_Send
* Description    : LCD write data
* Input          : - byte: byte to be sent
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_Send (uint8_t byte)
{
  int i;

  //printf("Send 0x%02X\r\n",byte);
  //FIXME : change single operation
  for(i=0;i<8;i++) {
    gpio_configure_pin(LCD_DB10_GPIO+i, PIO_OUTPUT_1|PIO_DEFAULT);
    if(byte & 0x01)
    gpio_set_pin_high(LCD_DB10_GPIO+i);
    else
    gpio_set_pin_low(LCD_DB10_GPIO+i);
    byte = byte >> 1;
  }
}

/*******************************************************************************
* Function Name  : LCD_Read
* Description    : LCD read data
* Input          : - byte: byte to be read
* Output         : None
* Return         : return data
* Attention		 : None
*******************************************************************************/
static uint8_t LCD_Read (void)
{
  uint8_t data = 0;
  int i;

   //FIXME : change single operation
  for(i=0;i<8;i++) {
      gpio_configure_pin(LCD_DB10_GPIO+i, PIO_INPUT|PIO_DEFAULT);
      data |= gpio_pin_is_high(LCD_DB10_GPIO+i) << i;
  }

  printf("Read 0x%02X\r\n",data);
	return data;
}

/*******************************************************************************
* Function Name  : LCD_WriteIndex
* Description    : LCD write register address
* Input          : - index: register address
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_WriteIndex(uint16_t index)
{
	gpio_set_pin_high(LCD_RD_GPIO);  /* RD high */
	gpio_set_pin_low(LCD_CS_GPIO);/* CS low */
  delay_us(1); //delay
	gpio_set_pin_low(LCD_RS_GPIO);   /* RS low */
  delay_us(1); //delay

  /* write data */
	LCD_Send(0x00);
	gpio_set_pin_low(LCD_WR_GPIO);   /* Wr low */
  delay_us(1); //delay
	gpio_set_pin_high(LCD_WR_GPIO);   /* Wr high */
  delay_us(1); //delay

	LCD_Send(index & 0xFF);
	gpio_set_pin_low(LCD_WR_GPIO);   /* Wr low */
  delay_us(1); //delay
	gpio_set_pin_high(LCD_WR_GPIO);   /* Wr high */
  delay_us(1); //delay

	gpio_set_pin_high(LCD_CS_GPIO);/* CS high */

  delay_us(1); //delay
}

/*******************************************************************************
* Function Name  : LCD_WriteData
* Description    : LCD write register data
* Input          : - index: register data
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_WriteData(uint16_t data)
{
	gpio_set_pin_low(LCD_CS_GPIO);/* CS low */
  delay_us(1); //delay
	gpio_set_pin_high(LCD_RS_GPIO);   /* RS high */
	gpio_set_pin_high(LCD_RD_GPIO);   /* RD high */
  delay_us(1); //delay

  /* write data */
	LCD_Send(data >> 8);
	gpio_set_pin_low(LCD_WR_GPIO);   /* Wr low */
  delay_us(1); //delay
	gpio_set_pin_high(LCD_WR_GPIO);   /* Wr high */
  delay_us(1); //delay

	LCD_Send(data & 0xFF);
	gpio_set_pin_low(LCD_WR_GPIO);   /* Wr low */
  delay_us(1); //delay
	gpio_set_pin_high(LCD_WR_GPIO);   /* Wr high */
  delay_us(1); //delay

	gpio_set_pin_high(LCD_CS_GPIO);/* CS high */

  delay_us(1); //delay
}

/*******************************************************************************
* Function Name  : LCD_ReadData
* Description    : LCD read data
* Input          : None
* Output         : None
* Return         : return data
* Attention		 : None
*******************************************************************************/
static uint16_t LCD_ReadData(void)
{
	uint16_t value;

	gpio_set_pin_low(LCD_CS_GPIO);  /* CS low */
  delay_us(1); //delay
	gpio_set_pin_high(LCD_RS_GPIO); /* RS high */
	gpio_set_pin_high(LCD_WR_GPIO); /* Wr high */
  delay_us(1); //delay

	gpio_set_pin_low(LCD_RD_GPIO);  /* RD low */
  /* read data */
  delay_us(1);
	value = LCD_Read() << 8;
	gpio_set_pin_high(LCD_RD_GPIO);  /* RD high */
  delay_us(1);

	gpio_set_pin_low(LCD_RD_GPIO);  /* RD low */
  /* read data */
  delay_us(1);
	value |= LCD_Read();
	gpio_set_pin_high(LCD_RD_GPIO);  /* RD high */

	gpio_set_pin_high(LCD_CS_GPIO);  /* CS high */

  delay_us(1);
	return value;
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Writes to the selected LCD register.
* Input          : - LCD_Reg: address of the selected register.
*                  - LCD_RegValue: value to write to the selected register.
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{
	/* Write 16-bit Index, then Write Reg */
	LCD_WriteIndex(LCD_Reg);
	/* Write 16-bit Reg */
	LCD_WriteData(LCD_RegValue);
}

/*******************************************************************************
* Function Name  : LCD_ReadReg
* Description    : Reads the selected LCD register data.
* Input          : - LCD_Reg: address of the selected register.
* Output         : None
* Return         : return Register data.
* Attention	 : None
*******************************************************************************/
static uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
	uint16_t LCD_RAM;

	/* Write 16-bit Index (then Read Reg) */
	LCD_WriteIndex(LCD_Reg);
	/* Read 16-bit Reg */
	LCD_RAM = LCD_ReadData();
	return LCD_RAM;
}

/*******************************************************************************
* Function Name  : LCD_SetCursor
* Description    : Sets the cursor position.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position.
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_SetCursor(uint16_t Xpos,uint16_t Ypos)
{
    #if  ( DISP_ORIENTATION == 90 ) || ( DISP_ORIENTATION == 270 )

 	uint16_t temp = Xpos;

			 Xpos = Ypos;
			 Ypos = ( MAX_X - 1 ) - temp;

	#elif  ( DISP_ORIENTATION == 0 ) || ( DISP_ORIENTATION == 180 )

	#endif

  switch( LCD_Code )
  {
     default:		 /* 0x9320 0x9325 0x5408 0x1505 0x0505 0x7783 0x4531 0x4535 */
          LCD_WriteReg(0x0020, Xpos );
          LCD_WriteReg(0x0021, Ypos );
	      break;

     case HX8347A: 	 /* 0x0047 */
	      LCD_WriteReg(0x02, Xpos>>8 );
	      LCD_WriteReg(0x03, Xpos );

	      LCD_WriteReg(0x06, Ypos>>8 );
	      LCD_WriteReg(0x07, Ypos );

	      break;
  }
}

/*******************************************************************************
* Function Name  : LCD_Initializtion
* Description    : Initialize TFT Controller.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Initializtion(void)
{
	uint16_t DeviceCode;

	LCD_Configuration();
	delay_ms(100);
	DeviceCode = LCD_ReadReg(0x0000);	/* read the LCD ID, if Controller is ILI9320, The ID value is 0x9320 */

	if( DeviceCode == 0x9325 || DeviceCode == 0x9328 )
	{
		LCD_Code = ILI9325;
		LCD_WriteReg(0x00e5,0x78f0);
		LCD_WriteReg(0x0001,0x0100);
		LCD_WriteReg(0x0002,0x0700); 	/* power on sequence */
		LCD_WriteReg(0x0003,0x1030);
		LCD_WriteReg(0x0004,0x0000);
		LCD_WriteReg(0x0008,0x0207);
		LCD_WriteReg(0x0009,0x0000);
		LCD_WriteReg(0x000a,0x0000); 	/* display setting */
		LCD_WriteReg(0x000c,0x0000);	/* display setting */
    //FRAME MARKER POS
		LCD_WriteReg(0x000d,0x0000);
		LCD_WriteReg(0x000f,0x0000);

		/* Power On sequence */
		LCD_WriteReg(0x0010,0x0000);
		LCD_WriteReg(0x0011,0x0007);
		LCD_WriteReg(0x0012,0x0000);
		LCD_WriteReg(0x0013,0x0000);
		LCD_WriteReg(0x0007,0x0001);
		delay_ms(200);  /* delay 50 ms */
		LCD_WriteReg(0x0010,0x1090);
		LCD_WriteReg(0x0011,0x0227);
		delay_ms(50);  /* delay 50 ms */
		LCD_WriteReg(0x0012,0x001f);
		delay_ms(50);  /* delay 50 ms */
		LCD_WriteReg(0x0013,0x1500);
		LCD_WriteReg(0x0029,0x0027);
		LCD_WriteReg(0x002b,0x000d);
		delay_ms(50);  /* delay 50 ms */
		LCD_WriteReg(0x0020,0x0000);
		LCD_WriteReg(0x0021,0x0000);
		delay_ms(50);  /* delay 50 ms */
    //GAMMA CONTROL
		LCD_WriteReg(0x0030,0x0007);
		LCD_WriteReg(0x0031,0x0707);
		LCD_WriteReg(0x0032,0x0307);
		LCD_WriteReg(0x0035,0x0200);
		LCD_WriteReg(0x0036,0x0008);
		LCD_WriteReg(0x0037,0x0004);
		LCD_WriteReg(0x0038,0x0000);
		LCD_WriteReg(0x0039,0x0707);
		LCD_WriteReg(0x003c,0x0002);
		LCD_WriteReg(0x003d,0x1d04);
		delay_ms(50);  /* delay 50 ms */

    //HOR, VER START,END
		LCD_WriteReg(0x0050,0x0000);
		LCD_WriteReg(0x0051,0x00ef);
		LCD_WriteReg(0x0052,0x0000);
		LCD_WriteReg(0x0053,0x013f);

    //GATE SCAN CTRL
		LCD_WriteReg(0x0060,0xa700);
		LCD_WriteReg(0x0061,0x0001);
		LCD_WriteReg(0x006a,0x0000);

		LCD_WriteReg(0x0080,0x0000);
		LCD_WriteReg(0x0081,0x0000);
		LCD_WriteReg(0x0082,0x0000);
		LCD_WriteReg(0x0083,0x0000);
		LCD_WriteReg(0x0084,0x0000);
		LCD_WriteReg(0x0085,0x0000);

    //PANEL_IF_CTRL
		LCD_WriteReg(0x0090,0x0010);
		LCD_WriteReg(0x0092,0x0600);
		/* display on sequence */
		LCD_WriteReg(0x0007,0x0133);

		//LCD_WriteReg(0x0020,0x0000);  /* Line first address 0 */
		//LCD_WriteReg(0x0021,0x0000);  /* Column first site 0 */
	}
	else if( DeviceCode == 0x9320 || DeviceCode == 0x9300 )
	{
	    LCD_Code = ILI9320;
	    LCD_WriteReg(0x00,0x0000);
		LCD_WriteReg(0x01,0x0100);	/* Driver Output Contral */
		LCD_WriteReg(0x02,0x0700);	/* LCD Driver Waveform Contral */
		LCD_WriteReg(0x03,0x1018);	/* Entry Mode Set */

		LCD_WriteReg(0x04,0x0000);	/* Scalling Contral */
	    LCD_WriteReg(0x08,0x0202);	/* Display Contral */
		LCD_WriteReg(0x09,0x0000);	/* Display Contral 3.(0x0000) */
		LCD_WriteReg(0x0a,0x0000);	/* Frame Cycle Contal.(0x0000) */
	    LCD_WriteReg(0x0c,(1<<0));	/* Extern Display Interface Contral */
		LCD_WriteReg(0x0d,0x0000);	/* Frame Maker Position */
		LCD_WriteReg(0x0f,0x0000);	/* Extern Display Interface Contral 2. */

	    delay_ms(100);  /* delay 100 ms */
		LCD_WriteReg(0x07,0x0101);	/* Display Contral */
	    delay_ms(100);  /* delay 100 ms */

		LCD_WriteReg(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	/* Power Control 1.(0x16b0)	*/
		LCD_WriteReg(0x11,0x0007);								/* Power Control 2 */
		LCD_WriteReg(0x12,(1<<8)|(1<<4)|(0<<0));				/* Power Control 3.(0x0138)	*/
		LCD_WriteReg(0x13,0x0b00);								/* Power Control 4 */
		LCD_WriteReg(0x29,0x0000);								/* Power Control 7 */

		LCD_WriteReg(0x2b,(1<<14)|(1<<4));

		LCD_WriteReg(0x50,0);       /* Set X Start */
		LCD_WriteReg(0x51,239);	    /* Set X End */
		LCD_WriteReg(0x52,0);	    /* Set Y Start */
		LCD_WriteReg(0x53,319);	    /* Set Y End */

		LCD_WriteReg(0x60,0x2700);	/* Driver Output Control */
		LCD_WriteReg(0x61,0x0001);	/* Driver Output Control */
		LCD_WriteReg(0x6a,0x0000);	/* Vertical Srcoll Control */

		LCD_WriteReg(0x80,0x0000);	/* Display Position? Partial Display 1 */
		LCD_WriteReg(0x81,0x0000);	/* RAM Address Start? Partial Display 1 */
		LCD_WriteReg(0x82,0x0000);	/* RAM Address End-Partial Display 1 */
		LCD_WriteReg(0x83,0x0000);	/* Displsy Position? Partial Display 2 */
		LCD_WriteReg(0x84,0x0000);	/* RAM Address Start? Partial Display 2 */
		LCD_WriteReg(0x85,0x0000);	/* RAM Address End? Partial Display 2 */

	    LCD_WriteReg(0x90,(0<<7)|(16<<0));	/* Frame Cycle Contral.(0x0013)	*/
		LCD_WriteReg(0x92,0x0000);	/* Panel Interface Contral 2.(0x0000) */
		LCD_WriteReg(0x93,0x0001);	/* Panel Interface Contral 3. */
	    LCD_WriteReg(0x95,0x0110);	/* Frame Cycle Contral.(0x0110)	*/
		LCD_WriteReg(0x97,(0<<8));
		LCD_WriteReg(0x98,0x0000);	/* Frame Cycle Contral */

	    LCD_WriteReg(0x07,0x0173);
	}
	else if( DeviceCode == 0x1505 || DeviceCode == 0x0505 )
	{
		LCD_Code = R61505U;
		/* initializing funciton */
		LCD_WriteReg(0xe5,0x8000);  /* Set the internal vcore voltage */
		LCD_WriteReg(0x00,0x0001);  /* start OSC */
		LCD_WriteReg(0x2b,0x0010);  /* Set the frame rate as 80 when the internal resistor is used for oscillator circuit */
		LCD_WriteReg(0x01,0x0100);  /* s720  to  s1 ; G1 to G320 */
		LCD_WriteReg(0x02,0x0700);  /* set the line inversion */
		LCD_WriteReg(0x03,0x1018);  /* 65536 colors */
		LCD_WriteReg(0x04,0x0000);
		LCD_WriteReg(0x08,0x0202);  /* specify the line number of front and back porch periods respectively */
		LCD_WriteReg(0x09,0x0000);
		LCD_WriteReg(0x0a,0x0000);
		LCD_WriteReg(0x0c,0x0000);  /* select  internal system clock */
		LCD_WriteReg(0x0d,0x0000);
		LCD_WriteReg(0x0f,0x0000);
		LCD_WriteReg(0x50,0x0000);  /* set windows adress */
		LCD_WriteReg(0x51,0x00ef);
		LCD_WriteReg(0x52,0x0000);
		LCD_WriteReg(0x53,0x013f);
		LCD_WriteReg(0x60,0x2700);
		LCD_WriteReg(0x61,0x0001);
		LCD_WriteReg(0x6a,0x0000);
		LCD_WriteReg(0x80,0x0000);
		LCD_WriteReg(0x81,0x0000);
		LCD_WriteReg(0x82,0x0000);
		LCD_WriteReg(0x83,0x0000);
		LCD_WriteReg(0x84,0x0000);
		LCD_WriteReg(0x85,0x0000);
		LCD_WriteReg(0x90,0x0010);
		LCD_WriteReg(0x92,0x0000);
		LCD_WriteReg(0x93,0x0003);
		LCD_WriteReg(0x95,0x0110);
		LCD_WriteReg(0x97,0x0000);
		LCD_WriteReg(0x98,0x0000);
		/* power setting function */
		LCD_WriteReg(0x10,0x0000);
		LCD_WriteReg(0x11,0x0000);
		LCD_WriteReg(0x12,0x0000);
		LCD_WriteReg(0x13,0x0000);
		delay_ms(100);
		LCD_WriteReg(0x10,0x17b0);
		LCD_WriteReg(0x11,0x0004);
		delay_ms(50);
		LCD_WriteReg(0x12,0x013e);
		delay_ms(50);
		LCD_WriteReg(0x13,0x1f00);
		LCD_WriteReg(0x29,0x000f);
		delay_ms(50);
		LCD_WriteReg(0x20,0x0000);
		LCD_WriteReg(0x21,0x0000);

		/* initializing function */
		LCD_WriteReg(0x30,0x0204);
		LCD_WriteReg(0x31,0x0001);
		LCD_WriteReg(0x32,0x0000);
		LCD_WriteReg(0x35,0x0206);
		LCD_WriteReg(0x36,0x0600);
		LCD_WriteReg(0x37,0x0500);
		LCD_WriteReg(0x38,0x0505);
		LCD_WriteReg(0x39,0x0407);
		LCD_WriteReg(0x3c,0x0500);
		LCD_WriteReg(0x3d,0x0503);

		/* display on */
		LCD_WriteReg(0x07,0x0173);
	}
	else if( DeviceCode == 0x5408 )
	{
		LCD_Code = SPFD5408B;

		LCD_WriteReg(0x0001,0x0100);	  /* Driver Output Contral Register */
		LCD_WriteReg(0x0002,0x0700);      /* LCD Driving Waveform Contral */
		LCD_WriteReg(0x0003,0x1030);	  /* Entry Mode���� */

		LCD_WriteReg(0x0004,0x0000);	  /* Scalling Control register */
		LCD_WriteReg(0x0008,0x0207);	  /* Display Control 2 */
		LCD_WriteReg(0x0009,0x0000);	  /* Display Control 3 */
		LCD_WriteReg(0x000A,0x0000);	  /* Frame Cycle Control */
		LCD_WriteReg(0x000C,0x0000);	  /* External Display Interface Control 1 */
		LCD_WriteReg(0x000D,0x0000);      /* Frame Maker Position */
		LCD_WriteReg(0x000F,0x0000);	  /* External Display Interface Control 2 */
		delay_ms(50);
		LCD_WriteReg(0x0007,0x0101);	  /* Display Control */
		delay_ms(50);
		LCD_WriteReg(0x0010,0x16B0);      /* Power Control 1 */
		LCD_WriteReg(0x0011,0x0001);      /* Power Control 2 */
		LCD_WriteReg(0x0017,0x0001);      /* Power Control 3 */
		LCD_WriteReg(0x0012,0x0138);      /* Power Control 4 */
		LCD_WriteReg(0x0013,0x0800);      /* Power Control 5 */
		LCD_WriteReg(0x0029,0x0009);	  /* NVM read data 2 */
		LCD_WriteReg(0x002a,0x0009);	  /* NVM read data 3 */
		LCD_WriteReg(0x00a4,0x0000);
		LCD_WriteReg(0x0050,0x0000);
		LCD_WriteReg(0x0051,0x00EF);
		LCD_WriteReg(0x0052,0x0000);
		LCD_WriteReg(0x0053,0x013F);

		LCD_WriteReg(0x0060,0x2700);	  /* Driver Output Control */

		LCD_WriteReg(0x0061,0x0003);	  /* Driver Output Control */
		LCD_WriteReg(0x006A,0x0000);	  /* Vertical Scroll Control */

		LCD_WriteReg(0x0080,0x0000);	  /* Display Position �C Partial Display 1 */
		LCD_WriteReg(0x0081,0x0000);	  /* RAM Address Start �C Partial Display 1 */
		LCD_WriteReg(0x0082,0x0000);	  /* RAM address End - Partial Display 1 */
		LCD_WriteReg(0x0083,0x0000);	  /* Display Position �C Partial Display 2 */
		LCD_WriteReg(0x0084,0x0000);	  /* RAM Address Start �C Partial Display 2 */
		LCD_WriteReg(0x0085,0x0000);	  /* RAM address End �C Partail Display2 */
		LCD_WriteReg(0x0090,0x0013);	  /* Frame Cycle Control */
		LCD_WriteReg(0x0092,0x0000); 	  /* Panel Interface Control 2 */
		LCD_WriteReg(0x0093,0x0003);	  /* Panel Interface control 3 */
		LCD_WriteReg(0x0095,0x0110);	  /* Frame Cycle Control */
		LCD_WriteReg(0x0007,0x0173);
	}
	else if( DeviceCode == 0x4531 )
	{
		LCD_Code = LGDP4531;
		/* Setup display */
		LCD_WriteReg(0x00,0x0001);
		LCD_WriteReg(0x10,0x0628);
		LCD_WriteReg(0x12,0x0006);
		LCD_WriteReg(0x13,0x0A32);
		LCD_WriteReg(0x11,0x0040);
		LCD_WriteReg(0x15,0x0050);
		LCD_WriteReg(0x12,0x0016);
		delay_ms(50);
		LCD_WriteReg(0x10,0x5660);
		delay_ms(50);
		LCD_WriteReg(0x13,0x2A4E);
		LCD_WriteReg(0x01,0x0100);
		LCD_WriteReg(0x02,0x0300);
		LCD_WriteReg(0x03,0x1030);
		LCD_WriteReg(0x08,0x0202);
		LCD_WriteReg(0x0A,0x0000);
		LCD_WriteReg(0x30,0x0000);
		LCD_WriteReg(0x31,0x0402);
		LCD_WriteReg(0x32,0x0106);
		LCD_WriteReg(0x33,0x0700);
		LCD_WriteReg(0x34,0x0104);
		LCD_WriteReg(0x35,0x0301);
		LCD_WriteReg(0x36,0x0707);
		LCD_WriteReg(0x37,0x0305);
		LCD_WriteReg(0x38,0x0208);
		LCD_WriteReg(0x39,0x0F0B);
		delay_ms(50);
		LCD_WriteReg(0x41,0x0002);
		LCD_WriteReg(0x60,0x2700);
		LCD_WriteReg(0x61,0x0001);
		LCD_WriteReg(0x90,0x0119);
		LCD_WriteReg(0x92,0x010A);
		LCD_WriteReg(0x93,0x0004);
		LCD_WriteReg(0xA0,0x0100);
		delay_ms(50);
		LCD_WriteReg(0x07,0x0133);
		delay_ms(50);
		LCD_WriteReg(0xA0,0x0000);
	}
	else if( DeviceCode == 0x4535 )
	{
		LCD_Code = LGDP4535;
		LCD_WriteReg(0x15, 0x0030); 	 /* Set the internal vcore voltage */
		LCD_WriteReg(0x9A, 0x0010); 	 /* Start internal OSC */
		LCD_WriteReg(0x11, 0x0020);	     /* set SS and SM bit */
		LCD_WriteReg(0x10, 0x3428);	     /* set 1 line inversion */
		LCD_WriteReg(0x12, 0x0002);	     /* set GRAM write direction and BGR=1 */
		LCD_WriteReg(0x13, 0x1038);	     /* Resize register */
		delay_ms(40);
		LCD_WriteReg(0x12, 0x0012);	     /* set the back porch and front porch */
		delay_ms(40);
		LCD_WriteReg(0x10, 0x3420);	     /* set non-display area refresh cycle ISC[3:0] */
		LCD_WriteReg(0x13, 0x3045);	     /* FMARK function */
		delay_ms(70);
		LCD_WriteReg(0x30, 0x0000);      /* RGB interface setting */
		LCD_WriteReg(0x31, 0x0402);	     /* Frame marker Position */
		LCD_WriteReg(0x32, 0x0307);      /* RGB interface polarity */
		LCD_WriteReg(0x33, 0x0304);      /* SAP, BT[3:0], AP, DSTB, SLP, STB */
		LCD_WriteReg(0x34, 0x0004);      /* DC1[2:0], DC0[2:0], VC[2:0] */
		LCD_WriteReg(0x35, 0x0401);      /* VREG1OUT voltage */
		LCD_WriteReg(0x36, 0x0707);      /* VDV[4:0] for VCOM amplitude */
		LCD_WriteReg(0x37, 0x0305);      /* SAP, BT[3:0], AP, DSTB, SLP, STB */
		LCD_WriteReg(0x38, 0x0610);      /* DC1[2:0], DC0[2:0], VC[2:0] */
		LCD_WriteReg(0x39, 0x0610);      /* VREG1OUT voltage */
		LCD_WriteReg(0x01, 0x0100);      /* VDV[4:0] for VCOM amplitude */
		LCD_WriteReg(0x02, 0x0300);      /* VCM[4:0] for VCOMH */
		LCD_WriteReg(0x03, 0x1030);      /* GRAM horizontal Address */
		LCD_WriteReg(0x08, 0x0808);      /* GRAM Vertical Address */
		LCD_WriteReg(0x0A, 0x0008);
		LCD_WriteReg(0x60, 0x2700);	     /* Gate Scan Line */
		LCD_WriteReg(0x61, 0x0001);	     /* NDL,VLE, REV */
		LCD_WriteReg(0x90, 0x013E);
		LCD_WriteReg(0x92, 0x0100);
		LCD_WriteReg(0x93, 0x0100);
		LCD_WriteReg(0xA0, 0x3000);
		LCD_WriteReg(0xA3, 0x0010);
		LCD_WriteReg(0x07, 0x0001);
		LCD_WriteReg(0x07, 0x0021);
		LCD_WriteReg(0x07, 0x0023);
		LCD_WriteReg(0x07, 0x0033);
		LCD_WriteReg(0x07, 0x0133);
	}
	else if( DeviceCode == 0x7783 )
	{
		LCD_Code = ST7781;
		/* Start Initial Sequence */
		LCD_WriteReg(0x00FF,0x0001);
		LCD_WriteReg(0x00F3,0x0008);
		LCD_WriteReg(0x0001,0x0100);
		LCD_WriteReg(0x0002,0x0700);
		LCD_WriteReg(0x0003,0x1030);
		LCD_WriteReg(0x0008,0x0302);
		LCD_WriteReg(0x0008,0x0207);
		LCD_WriteReg(0x0009,0x0000);
		LCD_WriteReg(0x000A,0x0000);
		LCD_WriteReg(0x0010,0x0000);
		LCD_WriteReg(0x0011,0x0005);
		LCD_WriteReg(0x0012,0x0000);
		LCD_WriteReg(0x0013,0x0000);
		delay_ms(50);
		LCD_WriteReg(0x0010,0x12B0);
		delay_ms(50);
		LCD_WriteReg(0x0011,0x0007);
		delay_ms(50);
		LCD_WriteReg(0x0012,0x008B);
		delay_ms(50);
		LCD_WriteReg(0x0013,0x1700);
		delay_ms(50);
		LCD_WriteReg(0x0029,0x0022);
		LCD_WriteReg(0x0030,0x0000);
		LCD_WriteReg(0x0031,0x0707);
		LCD_WriteReg(0x0032,0x0505);
		LCD_WriteReg(0x0035,0x0107);
		LCD_WriteReg(0x0036,0x0008);
		LCD_WriteReg(0x0037,0x0000);
		LCD_WriteReg(0x0038,0x0202);
		LCD_WriteReg(0x0039,0x0106);
		LCD_WriteReg(0x003C,0x0202);
		LCD_WriteReg(0x003D,0x0408);
		delay_ms(50);
		LCD_WriteReg(0x0050,0x0000);
		LCD_WriteReg(0x0051,0x00EF);
		LCD_WriteReg(0x0052,0x0000);
		LCD_WriteReg(0x0053,0x013F);
		LCD_WriteReg(0x0060,0xA700);
		LCD_WriteReg(0x0061,0x0001);
		LCD_WriteReg(0x0090,0x0033);
		LCD_WriteReg(0x002B,0x000B);
		LCD_WriteReg(0x0007,0x0133);
	}
	else	/* special ID */
	{
		DeviceCode = LCD_ReadReg(0x67);

		if( DeviceCode == 0x0047 )
		{
	        LCD_Code = HX8347A;
	        LCD_WriteReg(0x0042,0x0008);
	        /* Gamma setting */
		    LCD_WriteReg(0x0046,0x00B4);
		    LCD_WriteReg(0x0047,0x0043);
		    LCD_WriteReg(0x0048,0x0013);
		    LCD_WriteReg(0x0049,0x0047);
		    LCD_WriteReg(0x004A,0x0014);
		    LCD_WriteReg(0x004B,0x0036);
		    LCD_WriteReg(0x004C,0x0003);
		    LCD_WriteReg(0x004D,0x0046);
		    LCD_WriteReg(0x004E,0x0005);
		    LCD_WriteReg(0x004F,0x0010);
		    LCD_WriteReg(0x0050,0x0008);
		    LCD_WriteReg(0x0051,0x000a);
		    /* Window Setting */
		    LCD_WriteReg(0x0002,0x0000);
		    LCD_WriteReg(0x0003,0x0000);
		    LCD_WriteReg(0x0004,0x0000);
		    LCD_WriteReg(0x0005,0x00EF);
		    LCD_WriteReg(0x0006,0x0000);
		    LCD_WriteReg(0x0007,0x0000);
		    LCD_WriteReg(0x0008,0x0001);
		    LCD_WriteReg(0x0009,0x003F);
		    delay_ms(10);
		    LCD_WriteReg(0x0001,0x0006);
		    LCD_WriteReg(0x0016,0x00C8);
		    LCD_WriteReg(0x0023,0x0095);
		    LCD_WriteReg(0x0024,0x0095);
		    LCD_WriteReg(0x0025,0x00FF);
		    LCD_WriteReg(0x0027,0x0002);
		    LCD_WriteReg(0x0028,0x0002);
		    LCD_WriteReg(0x0029,0x0002);
		    LCD_WriteReg(0x002A,0x0002);
		    LCD_WriteReg(0x002C,0x0002);
		    LCD_WriteReg(0x002D,0x0002);
		    LCD_WriteReg(0x003A,0x0001);
		    LCD_WriteReg(0x003B,0x0001);
		    LCD_WriteReg(0x003C,0x00F0);
		    LCD_WriteReg(0x003D,0x0000);
		    delay_ms(20);
		    LCD_WriteReg(0x0035,0x0038);
		    LCD_WriteReg(0x0036,0x0078);
		    LCD_WriteReg(0x003E,0x0038);
		    LCD_WriteReg(0x0040,0x000F);
		    LCD_WriteReg(0x0041,0x00F0);
		    LCD_WriteReg(0x0038,0x0000);
		    /* Power Setting */
		    LCD_WriteReg(0x0019,0x0049);
		    LCD_WriteReg(0x0093,0x000A);
		    delay_ms(10);
		    LCD_WriteReg(0x0020,0x0020);
		    LCD_WriteReg(0x001D,0x0003);
		    LCD_WriteReg(0x001E,0x0000);
		    LCD_WriteReg(0x001F,0x0009);
		    LCD_WriteReg(0x0044,0x0053);
		    LCD_WriteReg(0x0045,0x0010);
		    delay_ms(10);
		    LCD_WriteReg(0x001C,0x0004);
		    delay_ms(20);
		    LCD_WriteReg(0x0043,0x0080);
		    delay_ms(5);
		    LCD_WriteReg(0x001B,0x000a);
		    delay_ms(40);
		    LCD_WriteReg(0x001B,0x0012);
		    delay_ms(40);
		    /* Display On Setting */
		    LCD_WriteReg(0x0090,0x007F);
		    LCD_WriteReg(0x0026,0x0004);
		    delay_ms(40);
		    LCD_WriteReg(0x0026,0x0024);
		    LCD_WriteReg(0x0026,0x002C);
		    delay_ms(40);
		    LCD_WriteReg(0x0070,0x0008);
		    LCD_WriteReg(0x0026,0x003C);
		    LCD_WriteReg(0x0057,0x0002);
		    LCD_WriteReg(0x0055,0x0000);
		    LCD_WriteReg(0x0057,0x0000);
		}
	}
    delay_ms(50);   /* delay 50 ms */

    LCD_Clear(Black);

    printf("LCD Initialized\r\n");
}

/*******************************************************************************
* Function Name  : LCD_Clear
* Description    : clear LCD screen
* Input          : - Color: Screen Color
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Clear(uint16_t Color)
{
	uint32_t index;

	if( LCD_Code == HX8347A )
	{
		LCD_WriteReg(0x02,0x00);
		LCD_WriteReg(0x03,0x00);

		LCD_WriteReg(0x04,0x00);
		LCD_WriteReg(0x05,0xEF);

		LCD_WriteReg(0x06,0x00);
		LCD_WriteReg(0x07,0x00);

		LCD_WriteReg(0x08,0x01);
		LCD_WriteReg(0x09,0x3F);
	}
	else
	{
		LCD_SetCursor(0,0);
	}

	LCD_WriteIndex(0x0022);
	for( index = 0; index < MAX_X * MAX_Y; index++ )
	{
		LCD_WriteData(Color);
	}
  #if 0
	for( index = 0; index < MAX_X * MAX_Y; index++ )
	{
    printf("index = %d\r\n",index);
    LCD_WriteReg(0x0020,index%240);
    LCD_WriteReg(0x0021,index/240);
		LCD_ReadReg(0x0022);
	}
  #endif
}

/******************************************************************************
* Function Name  : LCD_BGR2RGB
* Description    : RRRRRGGGGGGBBBBB change BBBBBGGGGGGRRRRR
* Input          : - color: BRG color value
* Output         : None
* Return         : RGB color value
* Attention	 : None
*******************************************************************************/
static unsigned short LCD_BGR2RGB( unsigned short color )
{
	unsigned short  r, g, b, rgb;

	b = ( color>>0 )  & 0x1f;
	g = ( color>>5 )  & 0x3f;
	r = ( color>>11 ) & 0x1f;

	rgb =  (b<<11) + (g<<5) + (r<<0);

	return( rgb );
}

/******************************************************************************
* Function Name  : LCD_GetPoint
* Description    : Get the color value of the specified coordinates
* Input          : - Xpos: Row Coordinate
*                  - Xpos: Line Coordinate
* Output         : None
* Return         : Screen Color
* Attention	 : None
*******************************************************************************/
uint16_t LCD_GetPoint(uint16_t Xpos,uint16_t Ypos)
{
	uint16_t dummy;

	LCD_SetCursor(Xpos,Ypos);
	LCD_WriteIndex(0x0022);

	switch( LCD_Code )
	{
		case ST7781:
		case LGDP4531:
		case LGDP4535:
             dummy = LCD_ReadData();   /* Empty read */
             dummy = LCD_ReadData();
 		     return  dummy;
	    case HX8347A:
             {
		        uint8_t red,green,blue;

				dummy = LCD_ReadData();   /* Empty read */

		        red = LCD_ReadData() >> 3;
                green = LCD_ReadData() >> 2;
                blue = LCD_ReadData() >> 3;
                dummy = (uint16_t) ( ( red<<11 ) | ( green << 5 ) | blue );
		     }
	         return  dummy;

        default:	/* 0x9320 0x9325 0x5408 0x1505 0x0505 */
             dummy = LCD_ReadData();   /* Empty read */
             dummy = LCD_ReadData();
 		     return  LCD_BGR2RGB( dummy );
	}
}

/******************************************************************************
* Function Name  : LCD_SetPoint
* Description    : Draw point at the specified coordinates
* Input          : - Xpos: Row Coordinate
*                  - Ypos: Line Coordinate
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_SetPoint(uint16_t Xpos,uint16_t Ypos,uint16_t point)
{
	if( Xpos >= MAX_X || Ypos >= MAX_Y )
	{
		return;
	}
	LCD_SetCursor(Xpos,Ypos);
	LCD_WriteReg(0x0022,point);
}

/******************************************************************************
* Function Name  : LCD_DrawLine
* Description    : Bresenham's line algorithm
* Input          : - x1: A point line coordinates
*                  - y1: A point column coordinates
*		   - x2: B point line coordinates
*		   - y2: B point column coordinates
*		   - color: Line color
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_DrawLine( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color )
{
    short dx,dy;      /* The definition of the X Y axis increase the value of the variable */
    short temp;

    if( x0 > x1 )
    {
	    temp = x1;
		x1 = x0;
		x0 = temp;
    }
    if( y0 > y1 )
    {
		temp = y1;
		y1 = y0;
		y0 = temp;
    }

	dx = x1-x0;
	dy = y1-y0;

    if( dx == 0 )
    {
        do
        {
            LCD_SetPoint(x0, y0, color);
            y0++;
        }
        while( y1 >= y0 );
		return;
    }
    if( dy == 0 )
    {
        do
        {
            LCD_SetPoint(x0, y0, color);
            x0++;
        }
        while( x1 >= x0 );
		return;
    }

    if( dx > dy )
    {
	    temp = 2 * dy - dx;
        while( x0 != x1 )
        {
	        LCD_SetPoint(x0,y0,color);
	        x0++;
	        if( temp > 0 )
	        {
	            y0++;
	            temp += 2 * dy - 2 * dx;
	 	    }
            else
            {
			    temp += 2 * dy;
			}
        }
        LCD_SetPoint(x0,y0,color);
    }
    else
    {
	    temp = 2 * dx - dy;
        while( y0 != y1 )
        {
	 	    LCD_SetPoint(x0,y0,color);
            y0++;
            if( temp > 0 )
            {
                x0++;
                temp+=2*dy-2*dx;
            }
            else
			{
                temp += 2 * dy;
			}
        }
        LCD_SetPoint(x0,y0,color);
	}
}

/******************************************************************************
* Function Name  : PutChar
* Description    : Lcd screen displays a character
* Input          : - Xpos: Horizontal coordinate
*                  - Ypos: Vertical coordinate
*				   - ASCI: Displayed character
*				   - charColor: Character color
*				   - bkColor: Background color
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void PutChar( uint16_t Xpos, uint16_t Ypos, uint8_t ASCI, uint16_t charColor, uint16_t bkColor )
{
	uint16_t i, j;
    uint8_t buffer[16], tmp_char;
    GetASCIICode(buffer,ASCI);  /* get font data */
    for( i=0; i<16; i++ )
    {
        tmp_char = buffer[i];
        for( j=0; j<8; j++ )
        {
            if(((tmp_char >> (7 - j)) & 0x01) == 0x01)
            {
                LCD_SetPoint( Xpos + j, Ypos + i, charColor );  /* Character color */
            }
            else
            {
                LCD_SetPoint( Xpos + j, Ypos + i, bkColor );  /* Background color */
            }
        }
    }
}

/******************************************************************************
* Function Name  : GUI_Text
* Description    : Displays the string
* Input          : - Xpos: Horizontal coordinate
*                  - Ypos: Vertical coordinate
*				   - str: Displayed string
*				   - charColor: Character color
*				   - bkColor: Background color
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void GUI_Text(uint16_t Xpos, uint16_t Ypos, uint8_t *str,uint16_t Color, uint16_t bkColor)
{
    uint8_t TempChar;
    do
    {
        TempChar = *str++;
        PutChar( Xpos, Ypos, TempChar, Color, bkColor );
        if( Xpos < MAX_X - 8 )
        {
            Xpos += 8;
        }
        else if ( Ypos < MAX_Y - 16 )
        {
            Xpos = 0;
            Ypos += 16;
        }
        else
        {
            Xpos = 0;
            Ypos = 0;
        }
    }
    while ( *str != 0 );
}

void LCD_Test(void)
{
//  LCD_Send(0xAC);
//  LCD_Read();
//  uint8_t data;
//  data = LCD_Read();
//  printf("0x%02X\r\n",data);

//  LCD_Send(0xF5);
  //LCD_WriteData(0xAABB);
  //LCD_WriteIndex(0x00CD);

  //LCD_Initializtion();

//  GUI_Text(100,100,"HAJIHONG", White, Black);

//  LCD_DrawLine(100,200,200,200, Red);
//  LCD_DrawLine(150,100,150,200, Green);
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
