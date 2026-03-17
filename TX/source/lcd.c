#include "lcd.h"

static void lcd_delay_cycles(volatile uint32_t n)
{
    while (n--) {
        __NOP();
    }
}

static void lcd_pulse_enable(void)
{
    SET_LCD_E(1);
    lcd_delay_cycles(500u);
    SET_LCD_E(0);
    lcd_delay_cycles(2000u);
}

static void lcd_write4(uint8_t nibble)
{
    SET_LCD_DATA_OUT(nibble & 0x0Fu);
    lcd_pulse_enable();
}

static void lcd_write_byte(uint8_t value, uint8_t rs)
{
    SET_LCD_RS(rs);
    SET_LCD_RW(0);
    lcd_write4((uint8_t)(value >> 4));
    lcd_write4((uint8_t)(value & 0x0Fu));
}

static void lcd_command(uint8_t cmd)
{
    lcd_write_byte(cmd, 0u);
    if (cmd == 0x01u || cmd == 0x02u) {
        lcd_delay_cycles(65000u);
    }
}

void Init_LCD(void)
{
    ENABLE_LCD_PORT_CLOCKS;

    PIN_DATA_PORT->PCR[3] = PORT_PCR_MUX(1);
    PIN_DATA_PORT->PCR[4] = PORT_PCR_MUX(1);
    PIN_DATA_PORT->PCR[5] = PORT_PCR_MUX(1);
    PIN_DATA_PORT->PCR[6] = PORT_PCR_MUX(1);
    PIN_E_PORT->PCR[PIN_E_SHIFT] = PORT_PCR_MUX(1);
    PIN_RW_PORT->PCR[PIN_RW_SHIFT] = PORT_PCR_MUX(1);
    PIN_RS_PORT->PCR[PIN_RS_SHIFT] = PORT_PCR_MUX(1);

    PIN_DATA_PT->PDDR |= PINS_DATA;
    PIN_E_PT->PDDR |= PIN_E;
    PIN_RW_PT->PDDR |= PIN_RW;
    PIN_RS_PT->PDDR |= PIN_RS;

    SET_LCD_E(0);
    SET_LCD_RW(0);
    SET_LCD_RS(0);

    /* Power-up wait for LCD controller */
    lcd_delay_cycles(200000u);

    /* 4-bit init sequence */
    lcd_write4(0x03u);
    lcd_delay_cycles(100000u);
    lcd_write4(0x03u);
    lcd_delay_cycles(30000u);
    lcd_write4(0x03u);
    lcd_delay_cycles(30000u);
    lcd_write4(0x02u);

    /* Function set: 4-bit, 2-line, 5x8 font */
    lcd_command(0x28u);
    /* Display on, cursor off */
    lcd_command(0x0Cu);
    /* Entry mode: increment, no shift */
    lcd_command(0x06u);
    /* Clear */
    lcd_command(0x01u);
}

void Set_Cursor(uint8_t column, uint8_t row)
{
    uint8_t base = (row == 0u) ? 0x00u : 0x40u;
    lcd_command((uint8_t)(0x80u | (base + column)));
}

void Clear_LCD(void)
{
    lcd_command(0x01u);
}

void lcd_putchar(char c)
{
    lcd_write_byte((uint8_t)c, 1u);
}

void Print_LCD(char *string)
{
    while (*string != '\0') {
        lcd_putchar(*string++);
    }
}
