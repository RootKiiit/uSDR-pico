/*
 * lcd.c
 *
 * Created: Mar 2021
 * Author: Arjan te Marvelde
 * 
 * Grove 16x2 LCD, HD44780 chip with JHD1804 I2C interface
 * Display RAM addresses 0x00-0x1f for top row and 0x40-0x5f for bottom row
 * Character Generator addresses are 0x00-0x07
 *
 * Character 0x00:
 *         +-+-+-+-+-+
 * 000000  | |1| | | |
 *         +-+-+-+-+-+
 * 000001  |1| | | | |
 *         +-+-+-+-+-+
 * 000010  | |1| | | |
 *         +-+-+-+-+-+
 * 000011  |1| | | | |
 *         +-+-+-+-+-+
 * 000100  | |1| | | |
 *         +-+-+-+-+-+
 * 000101  |1| | | | |
 *         +-+-+-+-+-+
 * 000110  | |1| | | |
 *         +-+-+-+-+-+
 * 000111  | | | | | |	<= do not use, cursor 
 *         +-+-+-+-+-+
 *
 */
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "lcd.h"





// commands
#define LCD_CLEARDISPLAY 	0x01					// Note: LCD_ENTRYINC is set
#define LCD_RETURNHOME 		0x02
#define LCD_ENTRYMODESET 	0x04
#define LCD_DISPLAYCONTROL 	0x08
#define LCD_CURSORSHIFT 	0x10
#define LCD_FUNCTIONSET 	0x20
#define LCD_SETCGRAMADDR 	0x40
#define LCD_SETDDRAMADDR 	0x80

// flags for display entry mode: LCD_ENTRYMODESET
#define LCD_ENTRYSHIFT		0x01
#define LCD_ENTRYNOSHIFT	0x00
#define LCD_ENTRYINC		0x02					// Also applies to CGRAM writes
#define LCD_ENTRYDEC		0x00					// Also applies to CGRAM writes

// flags for display on/off control: LCD_DISPLAYCONTROL
#define LCD_DISPLAYON 		0x04
#define LCD_DISPLAYOFF 		0x00
#define LCD_CURSORON 		0x02
#define LCD_CURSOROFF 		0x00
#define LCD_BLINKON 		0x01
#define LCD_BLINKOFF 		0x00

// flags for display/cursor shift: LCD_CURSORSHIFT
#define LCD_DISPLAYMOVE 	0x08
#define LCD_CURSORMOVE 		0x00
#define LCD_MOVERIGHT 		0x04
#define LCD_MOVELEFT 		0x00

// flags for function set: LCD_FUNCTIONSET
#define LCD_8BITMODE 		0x10
#define LCD_4BITMODE 		0x00
#define LCD_2LINE 			0x08
#define LCD_1LINE 			0x00
#define LCD_5x10DOTS 		0x04
#define LCD_5x8DOTS 		0x00

#define LCD_DELAY			100
#define LCD_COMMAND			0x80
#define LCD_DATA			0x40

/* I2C address and pins */
#define I2C_LCD 			0x27 //0x27 for PCF8574T;

/*
 * User defined characters
 */
uint8_t cgram[8][8] = 									// Write CGRAM
{ 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},	// 0x00: blank
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00},	// 0x01: Level 1
	{0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x00, 0x00},	// 0x02: Level 2
	{0x00, 0x00, 0x00, 0x1f, 0x1f, 0x1f, 0x00, 0x00},	// 0x03: Level 3
	{0x00, 0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x00},	// 0x04: Level 4
	{0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x00},	// 0x05: Level 5
	{0x00, 0x04, 0x04, 0x04, 0x1f, 0x0e, 0x04, 0x00},	// 0x06: Receive arrow down
	{0x04, 0x0e, 0x1f, 0x04, 0x04, 0x04, 0x00, 0x00}	// 0x07: Transmit arrow up
    };

//@R6LFJ For LCD 1602 with PCF8574T I added functions from SDK and change lcd_init;

// flag for backlight control
const int LCD_BACKLIGHT = 0x08;
const int LCD_ENABLE_BIT = 0x04;

/* Quick helper function for single byte transfers */
void i2c_write_byte(uint8_t val) {
    i2c_write_blocking(i2c1, I2C_LCD, &val, 1, false);
}

void lcd_toggle_enable(uint8_t val) {
    // Toggle enable pin on LCD display
    // We cannot do this too quickly or things don't work
#define DELAY_US 600
    sleep_us(DELAY_US);
    i2c_write_byte(val | LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
    i2c_write_byte(val & ~LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
}

// The display is sent a byte as two separate nibble transfers
void lcd_send_byte(uint8_t val, int mode) {
    uint8_t high = mode | (val & 0xF0) | LCD_BACKLIGHT;
    uint8_t low = mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT;

    i2c_write_byte(high);
    lcd_toggle_enable(high);
    i2c_write_byte(low);
    lcd_toggle_enable(low);
}
//@R6LFJ End of my addition funtional;

void lcd_init(void)
{ 
	uint8_t txdata[10];
	//uint8_t i;

	uint8_t i, j;	

	//sleep_ms(50);
	//txdata[0] = LCD_COMMAND; 
	
	/* Initialize function set (see datasheet fig 23)*/
	//txdata[0] = LCD_COMMAND; 
	//txdata[1] = LCD_FUNCTIONSET | LCD_8BITMODE | LCD_2LINE | LCD_5x8DOTS;
	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);
	//sleep_us(4500);
	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);
	//sleep_us(100);
	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);
	//sleep_us(LCD_DELAY);
	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);
	//sleep_us(LCD_DELAY);

	/* Initialize display control */
	//txdata[0] = LCD_COMMAND; 
	//txdata[1] = LCD_DISPLAYCONTROL | LCD_DISPLAYOFF | LCD_CURSOROFF | LCD_BLINKOFF;
	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);
	//sleep_us(LCD_DELAY);
	
	/* Display clear */
	//txdata[0] = LCD_COMMAND; 
	//txdata[1] = LCD_CLEARDISPLAY;
	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);
	//sleep_us(1530);

    lcd_send_byte(0x03, 0);
    lcd_send_byte(0x03, 0);
    lcd_send_byte(0x03, 0);
    lcd_send_byte(0x02, 0);

	/* Initialize entry mode set */ 
	//txdata[0] = LCD_COMMAND; 
	//txdata[1] = LCD_ENTRYMODESET | LCD_ENTRYINC | LCD_ENTRYNOSHIFT;
	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);
	//sleep_us(LCD_DELAY);

    lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYINC, 0);
    lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE, 0);
	
	/* Load CGRAM */
	for (i=0; i<8; i++)
	{

		//txdata[0] = LCD_COMMAND; 
		//txdata[1] = LCD_SETCGRAMADDR | (i<<3); 								//Set CGRAM address
		//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);
		//sleep_us(LCD_DELAY);

        lcd_send_byte(LCD_SETCGRAMADDR | (i<<3), 0);

		//txdata[0] = LCD_DATA;
		//for (int j=0; j<8; j++) txdata[1+j] = cgram[i][j];
        //i2c_write_blocking(i2c1, I2C_LCD, txdata, 9, false);
		//sleep_us(LCD_DELAY);
        //lcd_send_byte(LCD_SETCGRAMADDR, 0);
        //i2c_write_blocking(i2c1, I2C_LCD, cgram, 65, false);	

        j = 0;
        while(j < 8){
        uint8_t val = cgram[i][j];
        lcd_send_byte(val, 1);
        j++;
            }
       }

	/* Initialize display control */
	//txdata[0] = LCD_COMMAND; 
	//txdata[1] = LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);
	//sleep_us(LCD_DELAY);

    lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, 0);
	
	/* Display clear once more */
	//txdata[0] = LCD_COMMAND; 
	//txdata[1] = LCD_CLEARDISPLAY;
	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);
	//sleep_us(1530);

    lcd_send_byte(LCD_CLEARDISPLAY, 0);   
}

void lcd_clear(void)
{
	//uint8_t txdata[3];
	//txdata[0] = LCD_COMMAND; 
	//txdata[1] = LCD_CLEARDISPLAY;
	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);

     lcd_send_byte(LCD_CLEARDISPLAY, 0);

	//sleep_us(1530);
}

void lcd_curxy(uint8_t x, uint8_t y, bool on)
{
	//uint8_t txdata[3];
	//x &= 0x0f;
	//y &= 0x01;
	//txdata[0] = LCD_COMMAND; 
	//txdata[1] = x | 0x80 | (y==1?0x40:0x00);
	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);

    int cur_pos = (y == 0) ? 0x80 + x : 0xC0 + x;
    lcd_send_byte(cur_pos, 0);

	//sleep_us(LCD_DELAY);
	//txdata[0] = LCD_COMMAND; 
	//txdata[1] = LCD_DISPLAYCONTROL | LCD_DISPLAYON | (on?LCD_CURSORON:LCD_CURSOROFF) | LCD_BLINKOFF;

    uint8_t val = LCD_DISPLAYCONTROL | LCD_DISPLAYON | (on?LCD_CURSORON:LCD_CURSOROFF) | LCD_BLINKOFF;

	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);

    lcd_send_byte(val, 0);

	//sleep_us(LCD_DELAY);
}

void lcd_putxy(uint8_t x, uint8_t y, uint8_t c)
{
	//uint8_t txdata[3];
	//x &= 0x0f;
	//y &= 0x01;
	//txdata[0] = LCD_COMMAND; 
	//txdata[1] = x | 0x80 | (y==1?0x40:0x00);

    int cur_pos = (y == 0) ? 0x80 + x : 0xC0 + x;

	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);

    lcd_send_byte(cur_pos, 0);

	//sleep_us(LCD_DELAY);
	//txdata[0] = LCD_DATA; 
	//txdata[1] = c;

    uint8_t val = c;

	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);

    lcd_send_byte(val, 1);

	//sleep_us(LCD_DELAY);
}

void lcd_writexy(uint8_t x, uint8_t y, uint8_t *s)
{
	uint8_t i, len;
	uint8_t txdata[18];

	//x &= 0x0f;
	//y &= 0x01;
	//txdata[0] = LCD_COMMAND; 
	//txdata[1] = x | 0x80 | ((y==1)?0x40:0x00);

    int cur_pos = (y == 0) ? 0x80 + x : 0xC0 + x;

	//i2c_write_blocking(i2c1, I2C_LCD, txdata, 2, false);

    lcd_send_byte(cur_pos, 0);

	//sleep_us(LCD_DELAY);

	//len = strlen(s);
	//len = (len>(16-x))?(16-x):len;
	//txdata[0] = LCD_DATA; 
	//for(i=0; i<len; i++) txdata[i+1]=s[i];
	//i2c_write_blocking(i2c1, I2C_LCD, txdata, len+1, false);

    while (*s) {
        char val = *s;
        lcd_send_byte(val, 1);
        val = *s++;
    }

	//sleep_us(LCD_DELAY);
}

void lcd_test(void)
{
	uint8_t chr[17];
	int i, j;
	
	chr[16] = 0; 
	lcd_clear();
	for (i=0; i<16; i++)
	{
		for(j=0; j<16; j++) chr[j] = (uint8_t)(16*i+j);
		lcd_writexy(0, 0, chr);
		sleep_ms(800);
		lcd_writexy(0, 1, chr);
	}
	lcd_clear();
}
