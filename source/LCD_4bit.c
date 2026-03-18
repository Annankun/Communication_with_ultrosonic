#include "LCD_4bit.h"
#include "delay.h"
#include <stdint.h>

uint8_t lcd_read_status(void)
{
  uint8_t status;

  SET_LCD_DATA_DIR_IN
  SET_LCD_RS(0) //Changing to instruction input
  SET_LCD_RW(1) //Changing to Read
  Delay(1); // Give chance to update?
  SET_LCD_E(1) //Prepare LCD bus for data
  Delay(1);
  status  = GET_LCD_DATA_IN << 4; //Shift Upper Nibble to prepare for lower nibble
  SET_LCD_E(0) //Latches
  Delay(1);
  SET_LCD_E(1)
  Delay(1);
  status |= GET_LCD_DATA_IN; // Combine upper and lower nibble
  SET_LCD_E(0)
  SET_LCD_DATA_DIR_OUT //Set the direction of DB4-DB7 back to output
  return(status);
}

void wait_while_busy(void)
{
	for( ; lcd_read_status() & LCD_BUSY_FLAG_MASK; ) //Compare the LCD data bus with LCD Busy Flag, if not busy stop waiting
		;
}

void lcd_write_4bit(uint8_t c)
{
	//RS Set to 1 typically before can write data
  SET_LCD_RW(0) //RW = 0, Writing data
  SET_LCD_E(1) //Prepare LCD bus to send data
  SET_LCD_DATA_OUT(c&0x0F) //Clear upper nibble and only send expecting lower half
  Delay(1);
  SET_LCD_E(0)
  Delay(1);
}

void lcd_write_cmd(uint8_t c)
{
  wait_while_busy();

  SET_LCD_RS(0)
  lcd_write_4bit(c>>4); //Send upper nibble
  lcd_write_4bit(c); //Send lower nibble
}

static void lcd_write_data(uint8_t c)
{
  wait_while_busy();

  SET_LCD_RS(1)
  lcd_write_4bit(c>>4); // Write upper nibble
  lcd_write_4bit(c); //Write lower nibble
}

void lcd_putchar(char c)
{ 
  lcd_write_data(c);
}

void lcd_init_port(void) {
	/* Enable clocks for peripherals        */
  ENABLE_LCD_PORT_CLOCKS                          

	/* Set Pin Mux to GPIO */
  	//PORTC
	PIN_DATA_PORT->PCR[PIN_DATA_SHIFT] = PORT_PCR_MUX(1); //Pin 3 DB4
	PIN_DATA_PORT->PCR[PIN_DATA_SHIFT+1] = PORT_PCR_MUX(1); //Pin 4
	PIN_DATA_PORT->PCR[PIN_DATA_SHIFT+2] = PORT_PCR_MUX(1); //Pin 5
	PIN_DATA_PORT->PCR[PIN_DATA_SHIFT+3] = PORT_PCR_MUX(1); //Pin 6
	PIN_E_PORT->PCR[PIN_E_SHIFT] = PORT_PCR_MUX(1); //Pin 7
	PIN_RW_PORT->PCR[PIN_RW_SHIFT] = PORT_PCR_MUX(1);
	PIN_RS_PORT->PCR[PIN_RS_SHIFT] = PORT_PCR_MUX(1);
}

void Init_LCD(void)
{ 
	/* initialize port(s) for LCD */
	lcd_init_port();
	
  /* Set all pins for LCD as outputs */
  SET_LCD_ALL_DIR_OUT
  Delay(100);
  SET_LCD_RS(0) //Writing Instruction
  lcd_write_4bit(0x3);  //Set to 8 bit
  Delay(100);
  lcd_write_4bit(0x3); //Try again
  Delay(10);
  lcd_write_4bit(0x3); //8-bit mode
  lcd_write_4bit(0x2); //Switch to 4-bit
  //CMD writes upper and lower nibble since using 4 data buses
  lcd_write_cmd(0x28); //Sets 4-bit into 5x8 font, 2 line display
  lcd_write_cmd(0x0C); //Display on, cursor blink, cursor off
  lcd_write_cmd(0x06); //Sets cursor direction
  lcd_write_cmd(0x80);                 
}

void Set_Cursor(uint8_t column, uint8_t row)
{
  uint8_t address;

  address =(row * 0x40) + column;
	address |= 0x80;
  lcd_write_cmd(address);
}

void Clear_LCD(void)
{
  lcd_write_cmd(0x01);                 
  Set_Cursor(0, 0);
}

void Print_LCD(char *string)
{
  while(*string)  {
    lcd_putchar(*string++);
  }
}
void Clear_Bit(){

}
