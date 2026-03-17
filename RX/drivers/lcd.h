#ifndef LCD_H
#define LCD_H

#include <MKL25Z4.H>
#include <stdint.h>

#define LCD_COLUMNS   8
#define LCD_ROWS      2

/*-------------------- LCD interface hardware definitions --------------------*/
/* DB4=PTC3, DB5=PTC4, DB6=PTD5, DB7=PTD6                                   */
/* E=PTC7, RW=PTC8, RS=PTC9                                                  */
#define PIN_DATA_PORT         PORTC
#define PIN_DATA_PT           PTC
#define PIN_DATA_SHIFT        ( 3 )
#define PIN_E_PORT            PORTC
#define PIN_E_PT              PTC
#define PIN_E_SHIFT           ( 7 )
#define PIN_E                 ( 1 << PIN_E_SHIFT)
#define PIN_RW_PORT           PORTC
#define PIN_RW_PT             PTC
#define PIN_RW_SHIFT          ( 8 )
#define PIN_RW                ( 1 << PIN_RW_SHIFT)
#define PIN_RS_PORT           PORTC
#define PIN_RS_PT             PTC
#define PIN_RS_SHIFT          ( 9 )
#define PIN_RS                ( 1 << PIN_RS_SHIFT)
#define PINS_DATA             (0x0F << PIN_DATA_SHIFT)

#define ENABLE_LCD_PORT_CLOCKS \
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTC_MASK;

#define SET_LCD_E(x)          if (x) {PIN_E_PT->PSOR = PIN_E;}   else {PIN_E_PT->PCOR = PIN_E;}
#define SET_LCD_RW(x)         if (x) {PIN_RW_PT->PSOR = PIN_RW;} else {PIN_RW_PT->PCOR = PIN_RW;}
#define SET_LCD_RS(x)         if (x) {PIN_RS_PT->PSOR = PIN_RS;} else {PIN_RS_PT->PCOR = PIN_RS;}
#define SET_LCD_DATA_OUT(x)   PIN_DATA_PT->PDOR = (PIN_DATA_PT->PDOR & ~PINS_DATA) | ((x) << PIN_DATA_SHIFT)
#define GET_LCD_DATA_IN       (((PTC->PDIR & PINS_DATA) >> PIN_DATA_SHIFT) & 0x0F)

#define SET_LCD_ALL_DIR_OUT   { PIN_DATA_PT->PDDR |= PINS_DATA; \
                                PIN_E_PT->PDDR   |= PIN_E;      \
                                PIN_RW_PT->PDDR  |= PIN_RW;     \
                                PIN_RS_PT->PDDR  |= PIN_RS; }

#define SET_LCD_DATA_DIR_IN   PIN_DATA_PT->PDDR &= ~PINS_DATA;
#define SET_LCD_DATA_DIR_OUT  PIN_DATA_PT->PDDR |=  PINS_DATA;

#define LCD_BUSY_FLAG_MASK    (0x80)

void init_lcd(void);
void set_cursor(uint8_t column, uint8_t row);
void clear_lcd(void);
void print_lcd(char *string);
void lcd_putchar(char c);

#endif /* LCD_H */
