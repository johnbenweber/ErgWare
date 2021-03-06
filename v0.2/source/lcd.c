/*

Copyright (C) David W. Vernooy

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
//#include <util/delay.h> /*not thread-safe*/
#include "lcd.h"
#include <avr/eeprom.h>
#include "nil.h"

/* Command type sent to the lcd */
typedef enum { LCD_CMD  = 0, LCD_DATA = 1 } LcdCmdData;

/* declarations */
static void lcd_base_addr(uint16_t addr);
static void lcd_send(uint8_t data, LcdCmdData cd);


/* flag to invert text */
uint8_t invert = 0;
uint8_t bigfont = 0;

/* current position of lcd */
static int lcd_current_position;


/* Alphabet lookup --- table now stored in EEPROM in bytes 1 through 455 */
/*
unsigned char PROGMEM font5x7 [][5] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00 },   // sp
    { 0x00, 0x00, 0x2f, 0x00, 0x00 },   // !
    { 0x00, 0x07, 0x00, 0x07, 0x00 },   // "
    { 0x14, 0x7f, 0x14, 0x7f, 0x14 },   // #
    { 0x24, 0x2a, 0x7f, 0x2a, 0x12 },   // $
	{ 0x32, 0x34, 0x08, 0x16, 0x26 },   // %
    { 0x36, 0x49, 0x55, 0x22, 0x50 },   // &
    { 0x00, 0x05, 0x03, 0x00, 0x00 },   // '
    { 0x00, 0x1c, 0x22, 0x41, 0x00 },   // (
    { 0x00, 0x41, 0x22, 0x1c, 0x00 },   // )
    { 0x14, 0x08, 0x3E, 0x08, 0x14 },   // *
    { 0x08, 0x08, 0x3E, 0x08, 0x08 },   // +
    { 0x00, 0x00, 0x50, 0x30, 0x00 },   // ,
    { 0x10, 0x10, 0x10, 0x10, 0x10 },   // -
    { 0x00, 0x60, 0x60, 0x00, 0x00 },   // .
    { 0x20, 0x10, 0x08, 0x04, 0x02 },   // /
    { 0x3E, 0x51, 0x49, 0x45, 0x3E },   // 0
    { 0x00, 0x42, 0x7F, 0x40, 0x00 },   // 1
    { 0x42, 0x61, 0x51, 0x49, 0x46 },   // 2
    { 0x21, 0x41, 0x45, 0x4B, 0x31 },   // 3
    { 0x18, 0x14, 0x12, 0x7F, 0x10 },   // 4
    { 0x27, 0x45, 0x45, 0x45, 0x39 },   // 5
    { 0x3C, 0x4A, 0x49, 0x49, 0x30 },   // 6
    { 0x01, 0x71, 0x09, 0x05, 0x03 },   // 7
    { 0x36, 0x49, 0x49, 0x49, 0x36 },   // 8
    { 0x06, 0x49, 0x49, 0x29, 0x1E },   // 9
    { 0x00, 0x36, 0x36, 0x00, 0x00 },   // :
    { 0x00, 0x56, 0x36, 0x00, 0x00 },   // ;
    { 0x08, 0x14, 0x22, 0x41, 0x00 },   // <
    { 0x14, 0x14, 0x14, 0x14, 0x14 },   // =
    { 0x00, 0x41, 0x22, 0x14, 0x08 },   // >
    { 0x02, 0x01, 0x51, 0x09, 0x06 },   // ?
    { 0x32, 0x49, 0x59, 0x51, 0x3E },   // @
    { 0x7E, 0x11, 0x11, 0x11, 0x7E },   // A
    { 0x7F, 0x49, 0x49, 0x49, 0x36 },   // B
    { 0x3E, 0x41, 0x41, 0x41, 0x22 },   // C
    { 0x7F, 0x41, 0x41, 0x22, 0x1C },   // D
    { 0x7F, 0x49, 0x49, 0x49, 0x41 },   // E
    { 0x7F, 0x09, 0x09, 0x09, 0x01 },   // F
    { 0x3E, 0x41, 0x49, 0x49, 0x7A },   // G
    { 0x7F, 0x08, 0x08, 0x08, 0x7F },   // H
    { 0x00, 0x41, 0x7F, 0x41, 0x00 },   // I
    { 0x20, 0x40, 0x41, 0x3F, 0x01 },   // J
    { 0x7F, 0x08, 0x14, 0x22, 0x41 },   // K
    { 0x7F, 0x40, 0x40, 0x40, 0x40 },   // L
    { 0x7F, 0x02, 0x0C, 0x02, 0x7F },   // M
    { 0x7F, 0x04, 0x08, 0x10, 0x7F },   // N
    { 0x3E, 0x41, 0x41, 0x41, 0x3E },   // O
    { 0x7F, 0x09, 0x09, 0x09, 0x06 },   // P
    { 0x3E, 0x41, 0x51, 0x21, 0x5E },   // Q
    { 0x7F, 0x09, 0x19, 0x29, 0x46 },   // R
    { 0x46, 0x49, 0x49, 0x49, 0x31 },   // S
    { 0x01, 0x01, 0x7F, 0x01, 0x01 },   // T
    { 0x3F, 0x40, 0x40, 0x40, 0x3F },   // U
    { 0x1F, 0x20, 0x40, 0x20, 0x1F },   // V
    { 0x3F, 0x40, 0x38, 0x40, 0x3F },   // W
    { 0x63, 0x14, 0x08, 0x14, 0x63 },   // X
    { 0x07, 0x08, 0x70, 0x08, 0x07 },   // Y
    { 0x61, 0x51, 0x49, 0x45, 0x43 },   // Z
    { 0x00, 0x7F, 0x41, 0x41, 0x00 },   // [
    { 0x55, 0x2A, 0x55, 0x2A, 0x55 },   // 
    { 0x00, 0x41, 0x41, 0x7F, 0x00 },   // ]
    { 0x04, 0x02, 0x01, 0x02, 0x04 },   // ^
    { 0x40, 0x40, 0x40, 0x40, 0x40 },   // _
    { 0x00, 0x01, 0x02, 0x04, 0x00 },   // '
    { 0x20, 0x54, 0x54, 0x54, 0x78 },   // a
    { 0x7F, 0x48, 0x44, 0x44, 0x38 },   // b
    { 0x38, 0x44, 0x44, 0x44, 0x20 },   // c
    { 0x38, 0x44, 0x44, 0x48, 0x7F },   // d
    { 0x38, 0x54, 0x54, 0x54, 0x18 },   // e
    { 0x08, 0x7E, 0x09, 0x01, 0x02 },   // f
    { 0x0C, 0x52, 0x52, 0x52, 0x3E },   // g
    { 0x7F, 0x08, 0x04, 0x04, 0x78 },   // h
    { 0x00, 0x44, 0x7D, 0x40, 0x00 },   // i
    { 0x20, 0x40, 0x44, 0x3D, 0x00 },   // j
    { 0x7F, 0x10, 0x28, 0x44, 0x00 },   // k
    { 0x00, 0x41, 0x7F, 0x40, 0x00 },   // l
    { 0x7C, 0x04, 0x18, 0x04, 0x78 },   // m
    { 0x7C, 0x08, 0x04, 0x04, 0x78 },   // n
    { 0x38, 0x44, 0x44, 0x44, 0x38 },   // o
    { 0x7C, 0x14, 0x14, 0x14, 0x08 },   // p
    { 0x08, 0x14, 0x14, 0x18, 0x7C },   // q
    { 0x7C, 0x08, 0x04, 0x04, 0x08 },   // r
    { 0x48, 0x54, 0x54, 0x54, 0x20 },   // s
    { 0x04, 0x3F, 0x44, 0x40, 0x20 },   // t
    { 0x3C, 0x40, 0x40, 0x20, 0x7C },   // u
    { 0x1C, 0x20, 0x40, 0x20, 0x1C },   // v
    { 0x3C, 0x40, 0x30, 0x40, 0x3C },   // w
    { 0x44, 0x28, 0x10, 0x28, 0x44 },   // x
    { 0x0C, 0x50, 0x50, 0x50, 0x3C },   // y
    { 0x44, 0x64, 0x54, 0x4C, 0x44 },   // z
   };
*/


/*
//starts at 512
const uint8_t NumLookupLow[13][10]={ 
{0x3F, 0x7F, 0xE0, 0xC0, 0xC0, 0xC0, 0xC0, 0xE0, 0x7F, 0x3F}, //0L
{0xE0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF, 0xFF, 0xC0, 0xC0, 0xE0}, //1L
{0xF8, 0xFC, 0xCE, 0xC7, 0xC3, 0xC1, 0xC0, 0xC0, 0xC0, 0xE0}, //2L
{0x18, 0x78, 0xE0, 0xC1, 0xC1, 0xC1, 0xC3, 0xE7, 0xFE, 0x3C}, //3L
{0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFF, 0x03}, //4L
{0x18, 0x78, 0xE0, 0xC0, 0xC0, 0xC0, 0xC0, 0xE1, 0xFF, 0x3F}, //5L
{0x3F, 0x7F, 0xE1, 0xC1, 0xC0, 0xC0, 0xC0, 0xE1, 0xFF, 0x3F}, //6L
{0x00, 0x00, 0x00, 0xF8, 0xFE, 0x0F, 0x03, 0x00, 0x00, 0x00}, //7L
{0x3E, 0x7F, 0xE3, 0xC1, 0xC1, 0xC1, 0xC1, 0xE3, 0x7F, 0x3E}, //8L
{0x70, 0xF1, 0xE3, 0xC3, 0xC3, 0xC3, 0xC3, 0xE1, 0x7F, 0x3F}, //9L
{0x70, 0x60, 0x61, 0xFF, 0x61, 0x61, 0xFF, 0x63, 0x3F, 0x1F}, //dollarL
{0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //pointL
{0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00}, //colonL
};

//starts at 648
const uint8_t NumLookupHigh[13][10]={ 
{0xFC, 0xFE, 0x07, 0x03, 0x03, 0x03, 0x03, 0x07, 0xFE, 0xFC}, //0H
{0x00, 0x06, 0x07, 0x03, 0x03, 0xFF, 0xFF, 0x00, 0x00, 0x00}, //1H
{0x1C, 0x1E, 0x07, 0x03, 0x83, 0xC3, 0xE3, 0x73, 0x3F, 0x1E}, //2H
{0x1C, 0x1E, 0x07, 0x83, 0x83, 0x83, 0xC3, 0xE7, 0x7F, 0x3E}, //3H
{0xC0, 0xE0, 0x70, 0x38, 0x1C, 0x0E, 0x07, 0xFF, 0xFF, 0x00}, //4H
{0xFF, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0x83, 0x07}, //5H
{0xFC, 0xFE, 0x87, 0xC3, 0xC3, 0xC3, 0xC3, 0xC7, 0x8F, 0x0E}, //6H
{0x07, 0x03, 0x03, 0x03, 0x03, 0x83, 0xE3, 0xFB, 0x3F, 0x0F}, //7H
{0x7C, 0xFE, 0xC7, 0x83, 0x83, 0x83, 0x83, 0xC7, 0xFE, 0x7C}, //8H
{0xFC, 0xFE, 0x87, 0x03, 0x03, 0x03, 0x83, 0x87, 0xFE, 0xFC}, //9H
{0x78, 0xFC, 0xC6, 0xFF, 0x86, 0x86, 0xFF, 0x86, 0x86, 0x0E}, //dollarH
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //pointH
{0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00}, //colonH

};
*/

/*
//starts at 513
const uint8_t NumLookupLow[16][11]= {

{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Code for char  
//exclude below
{0x00,0x00,0x00,0x00,0x33,0x33,0x00,0x00,0x00,0x00,0x00}, // Code for char !
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Code for char "
{0x02,0x1E,0x1F,0x03,0x02,0x1E,0x1F,0x03,0x02,0x00,0x00}, // Code for char #
{0x00,0x04,0x0C,0x0C,0x3F,0x3F,0x0C,0x0F,0x07,0x00,0x00}, // Code for char $
{0x30,0x38,0x1C,0x0E,0x07,0x03,0x01,0x38,0x38,0x38,0x00}, // Code for char %
{0x00,0x1F,0x3F,0x31,0x21,0x37,0x1E,0x1C,0x36,0x22,0x00}, // Code for char &
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Code for char '
{0x00,0x00,0x03,0x0F,0x1F,0x38,0x20,0x20,0x00,0x00,0x00}, // Code for char (
{0x00,0x00,0x20,0x20,0x38,0x1F,0x0F,0x03,0x00,0x00,0x00}, // Code for char )
{0x00,0x0C,0x0E,0x03,0x0F,0x0F,0x03,0x0E,0x0C,0x00,0x00}, // Code for char *
{0x00,0x01,0x01,0x01,0x0F,0x0F,0x01,0x01,0x01,0x00,0x00}, // Code for char +
//exclude above
//include below
{0x00,0x00,0x00,0xB8,0xF8,0x78,0x00,0x00,0x00,0x00,0x00}, // Code for char ,
{0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00}, // Code for char -
{0x00,0x00,0x00,0x38,0x38,0x38,0x00,0x00,0x00,0x00,0x00}, // Code for char .
{0x18,0x1C,0x0E,0x07,0x03,0x01,0x00,0x00,0x00,0x00,0x00}, // Code for char /
{0x07,0x1F,0x1E,0x33,0x31,0x30,0x30,0x30,0x18,0x1F,0x07}, // Code for char 0
{0x00,0x00,0x30,0x30,0x30,0x3F,0x3F,0x30,0x30,0x30,0x00}, // Code for char 1
{0x30,0x38,0x3C,0x3E,0x37,0x33,0x31,0x30,0x30,0x30,0x30}, // Code for char 2
{0x0C,0x1C,0x38,0x30,0x30,0x30,0x30,0x30,0x39,0x1F,0x0E}, // Code for char 3
{0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x3F,0x3F,0x03,0x03}, // Code for char 4
{0x0C,0x1C,0x38,0x30,0x30,0x30,0x30,0x30,0x38,0x1F,0x0F}, // Code for char 5
{0x0F,0x1F,0x39,0x30,0x30,0x30,0x30,0x30,0x39,0x1F,0x0F}, // Code for char 6
{0x00,0x00,0x00,0x30,0x3C,0x0F,0x03,0x00,0x00,0x00,0x00}, // Code for char 7
{0x0F,0x1F,0x39,0x30,0x30,0x30,0x30,0x30,0x39,0x1F,0x0F}, // Code for char 8
{0x00,0x00,0x30,0x30,0x30,0x38,0x1C,0x0E,0x07,0x03,0x00}, // Code for char 9
{0x00,0x00,0x00,0x1C,0x1C,0x1C,0x00,0x00,0x00,0x00,0x00} // Code for char :
//include below
//exclude below
{0x00,0x00,0x00,0x9C,0xFC,0x7C,0x00,0x00,0x00,0x00,0x00}, // Code for char ;
{0x00,0x00,0x01,0x03,0x07,0x0E,0x1C,0x38,0x30,0x00,0x00}, // Code for char <
{0x00,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x00}, // Code for char =
{0x00,0x30,0x38,0x1C,0x0E,0x07,0x03,0x01,0x00,0x00,0x00}, // Code for char >
{0x00,0x00,0x00,0x00,0x37,0x37,0x00,0x00,0x00,0x00,0x00}, // Code for char ?
{0x0F,0x1F,0x18,0x33,0x37,0x36,0x37,0x37,0x36,0x03,0x01}, // Code for char @
{0x38,0x3F,0x07,0x06,0x06,0x06,0x06,0x07,0x3F,0x38,0x00}, // Code for char A
{0x3F,0x3F,0x30,0x30,0x30,0x30,0x30,0x39,0x1F,0x0F,0x00}, // Code for char B
{0x03,0x0F,0x1C,0x38,0x30,0x30,0x30,0x38,0x1C,0x0C,0x00}, // Code for char C
{0x3F,0x3F,0x30,0x30,0x30,0x30,0x38,0x1C,0x0F,0x03,0x00}, // Code for char D
{0x3F,0x3F,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x00}, // Code for char E
{0x3F,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Code for char F
{0x03,0x0F,0x1C,0x38,0x30,0x30,0x30,0x30,0x3F,0x3F,0x00}, // Code for char G
{0x3F,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0x3F,0x00}, // Code for char H
{0x00,0x00,0x30,0x30,0x3F,0x3F,0x30,0x30,0x00,0x00,0x00}, // Code for char I
{0x0E,0x1E,0x38,0x30,0x30,0x30,0x30,0x38,0x1F,0x07,0x00}, // Code for char J
{0x3F,0x3F,0x00,0x01,0x03,0x07,0x0E,0x1C,0x38,0x30,0x00}, // Code for char K
{0x3F,0x3F,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x00}, // Code for char L
{0x3F,0x3F,0x00,0x00,0x01,0x01,0x00,0x00,0x3F,0x3F,0x00}, // Code for char M
{0x3F,0x3F,0x00,0x00,0x00,0x03,0x07,0x1C,0x3F,0x3F,0x00}, // Code for char N
{0x03,0x0F,0x1C,0x38,0x30,0x30,0x38,0x1C,0x0F,0x03,0x00}, // Code for char O
{0x3F,0x3F,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00}, // Code for char P
{0x03,0x0F,0x1C,0x38,0x30,0x36,0x3E,0x1C,0x3F,0x33,0x00}, // Code for char Q
{0x3F,0x3F,0x01,0x01,0x03,0x07,0x0F,0x1D,0x38,0x30,0x00}, // Code for char R
{0x0C,0x1C,0x38,0x30,0x30,0x30,0x30,0x39,0x1F,0x0F,0x00}, // Code for char S
{0x00,0x00,0x00,0x00,0x3F,0x3F,0x00,0x00,0x00,0x00,0x00}, // Code for char T
{0x07,0x1F,0x38,0x30,0x30,0x30,0x30,0x38,0x1F,0x07,0x00}, // Code for char U
{0x00,0x00,0x01,0x0F,0x3E,0x3E,0x0F,0x01,0x00,0x00,0x00}, // Code for char V
{0x3F,0x3F,0x1C,0x06,0x03,0x03,0x06,0x1C,0x3F,0x3F,0x00}, // Code for char W
{0x30,0x3C,0x0E,0x03,0x01,0x01,0x03,0x0E,0x3C,0x30,0x00}, // Code for char X
{0x00,0x00,0x00,0x00,0x3F,0x3F,0x00,0x00,0x00,0x00,0x00}, // Code for char Y
{0x30,0x3C,0x3E,0x33,0x31,0x30,0x30,0x30,0x30,0x30,0x00}, // Code for char Z
{0x00,0x00,0x3F,0x3F,0x30,0x30,0x30,0x30,0x00,0x00,0x00}, // Code for char [
{0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x07,0x0E,0x1C,0x18}, // Code for <Backslash>
{0x00,0x00,0x30,0x30,0x30,0x30,0x3F,0x3F,0x00,0x00,0x00}, // Code for char ]
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Code for char ^
{0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0}, // Code for char _
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Code for char `
{0x1C,0x3E,0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x3F,0x00}, // Code for char a
{0x3F,0x3F,0x30,0x30,0x30,0x30,0x30,0x38,0x1F,0x0F,0x00}, // Code for char b
{0x0F,0x1F,0x38,0x30,0x30,0x30,0x30,0x30,0x18,0x08,0x00}, // Code for char c
{0x0F,0x1F,0x38,0x30,0x30,0x30,0x30,0x30,0x3F,0x3F,0x00}, // Code for char d
{0x0F,0x1F,0x3B,0x33,0x33,0x33,0x33,0x33,0x13,0x01,0x00}, // Code for char e
{0x00,0x00,0x3F,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Code for char f
{0x03,0xC7,0xCE,0xCC,0xCC,0xCC,0xCC,0xE6,0x7F,0x3F,0x00}, // Code for char g
{0x3F,0x3F,0x00,0x00,0x00,0x00,0x00,0x3F,0x3F,0x00,0x00}, // Code for char h
{0x00,0x00,0x30,0x30,0x3F,0x3F,0x30,0x30,0x00,0x00,0x00}, // Code for char i
{0x00,0x00,0x60,0xE0,0xC0,0xC0,0xFF,0x7F,0x00,0x00,0x00}, // Code for char j
{0x00,0x3F,0x3F,0x03,0x07,0x0F,0x1C,0x38,0x30,0x00,0x00}, // Code for char k
{0x00,0x00,0x30,0x30,0x3F,0x3F,0x30,0x30,0x00,0x00,0x00}, // Code for char l
{0x3F,0x3F,0x00,0x00,0x3F,0x3F,0x00,0x00,0x3F,0x3F,0x00}, // Code for char m
{0x00,0x3F,0x3F,0x00,0x00,0x00,0x00,0x00,0x3F,0x3F,0x00}, // Code for char n
{0x0F,0x1F,0x38,0x30,0x30,0x30,0x30,0x38,0x1F,0x0F,0x00}, // Code for char o
{0xFF,0xFF,0x0C,0x18,0x18,0x18,0x18,0x1C,0x0F,0x07,0x00}, // Code for char p
{0x07,0x0F,0x1C,0x18,0x18,0x18,0x18,0x0C,0xFF,0xFF,0x00}, // Code for char q
{0x00,0x3F,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Code for char r
{0x11,0x33,0x33,0x33,0x33,0x33,0x3F,0x1E,0x00,0x00,0x00}, // Code for char s
{0x00,0x00,0x1F,0x3F,0x30,0x30,0x30,0x30,0x00,0x00,0x00}, // Code for char t
{0x0F,0x1F,0x38,0x30,0x30,0x30,0x30,0x18,0x3F,0x3F,0x00}, // Code for char u
{0x00,0x01,0x07,0x1E,0x38,0x38,0x1E,0x07,0x01,0x00,0x00}, // Code for char v
{0x07,0x1F,0x38,0x1C,0x0F,0x0F,0x1C,0x38,0x1F,0x07,0x00}, // Code for char w
{0x30,0x38,0x1D,0x0F,0x07,0x0F,0x1D,0x38,0x30,0x00,0x00}, // Code for char x
{0x00,0x00,0x81,0xE7,0x7E,0x1E,0x07,0x01,0x00,0x00,0x00}, // Code for char y
{0x30,0x38,0x3C,0x36,0x33,0x31,0x30,0x30,0x30,0x00,0x00}, // Code for char z
{0x00,0x00,0x01,0x1F,0x3F,0x70,0x60,0x60,0x60,0x00,0x00}, // Code for char {
{0x00,0x00,0x00,0x00,0x3F,0x3F,0x00,0x00,0x00,0x00,0x00}, // Code for char |
{0x00,0x60,0x60,0x60,0x70,0x3F,0x1F,0x01,0x00,0x00,0x00}, // Code for char }
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Code for char ~
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}  // Code for <Degrees>
};
*/

/*
//starts at 690
const uint8_t NumLookupHigh[16][11]= {
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Code for char  
//exclude below
{0x00,0x00,0x00,0x7C,0xFF,0xFF,0x7C,0x00,0x00,0x00,0x00}, // Code for char !
{0x00,0x00,0x3C,0x3C,0x00,0x00,0x3C,0x3C,0x00,0x00,0x00}, // Code for char "
{0x00,0x10,0x90,0xF0,0x7E,0x1E,0x90,0xF0,0x7E,0x1E,0x10}, // Code for char #
{0x00,0x78,0xFC,0xCC,0xFF,0xFF,0xCC,0xCC,0x88,0x00,0x00}, // Code for char $
{0x00,0x38,0x38,0x38,0x00,0x80,0xC0,0xE0,0x70,0x38,0x1C}, // Code for char %
{0x00,0x00,0xB8,0xFC,0xC6,0xE2,0x3E,0x1C,0x00,0x00,0x00}, // Code for char &
{0x00,0x00,0x00,0x27,0x3F,0x1F,0x00,0x00,0x00,0x00,0x00}, // Code for char '
{0x00,0x00,0xF0,0xFC,0xFE,0x07,0x01,0x01,0x00,0x00,0x00}, // Code for char (
{0x00,0x00,0x01,0x01,0x07,0xFE,0xFC,0xF0,0x00,0x00,0x00}, // Code for char )
{0x00,0x98,0xB8,0xE0,0xF8,0xF8,0xE0,0xB8,0x98,0x00,0x00}, // Code for char *
{0x00,0x80,0x80,0x80,0xF0,0xF0,0x80,0x80,0x80,0x00,0x00}, // Code for char +
//exclude above
//include below
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Code for char ,
{0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x00}, // Code for char -
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Code for char .
{0x00,0x00,0x00,0x00,0x80,0xC0,0xE0,0x70,0x38,0x1C,0x0E}, // Code for char /
{0xF8,0xFE,0x06,0x03,0x83,0xC3,0x63,0x33,0x1E,0xFE,0xF8}, // Code for char 0
{0x00,0x00,0x0C,0x0C,0x0E,0xFF,0xFF,0x00,0x00,0x00,0x00}, // Code for char 1
{0x1C,0x1E,0x07,0x03,0x03,0x83,0xC3,0xE3,0x77,0x3E,0x1C}, // Code for char 2
{0x0C,0x0E,0x07,0xC3,0xC3,0xC3,0xC3,0xC3,0xE7,0x7E,0x3C}, // Code for char 3
{0xC0,0xE0,0x70,0x38,0x1C,0x0E,0x07,0xFF,0xFF,0x00,0x00}, // Code for char 4
{0x3F,0x7F,0x63,0x63,0x63,0x63,0x63,0x63,0xE3,0xC3,0x83}, // Code for char 5
{0xC0,0xF0,0xF8,0xDC,0xCE,0xC7,0xC3,0xC3,0xC3,0x80,0x00}, // Code for char 6
{0x03,0x03,0x03,0x03,0x03,0x03,0xC3,0xF3,0x3F,0x0F,0x03}, // Code for char 7
{0x00,0xBC,0xFE,0xE7,0xC3,0xC3,0xC3,0xE7,0xFE,0xBC,0x00}, // Code for char 8
{0x3C,0x7E,0xE7,0xC3,0xC3,0xC3,0xC3,0xC3,0xE7,0xFE,0xFC}, // Code for char 9
{0x00,0x00,0x00,0x70,0x70,0x70,0x00,0x00,0x00,0x00,0x00} // Code for char :
//include above
//exclude below
{0x00,0x00,0x00,0x70,0x70,0x70,0x00,0x00,0x00,0x00,0x00}, // Code for char ;
{0x00,0xC0,0xE0,0xF0,0x38,0x1C,0x0E,0x07,0x03,0x00,0x00}, // Code for char <
{0x00,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x00}, // Code for char =
{0x00,0x03,0x07,0x0E,0x1C,0x38,0xF0,0xE0,0xC0,0x00,0x00}, // Code for char >
{0x1C,0x1E,0x07,0x03,0x83,0xC3,0xE3,0x77,0x3E,0x1C,0x00}, // Code for char ?
{0xF8,0xFE,0x07,0xF3,0xFB,0x1B,0xFB,0xFB,0x07,0xFE,0xF8}, // Code for char @
{0x00,0x00,0xE0,0xFC,0x1F,0x1F,0xFC,0xE0,0x00,0x00,0x00}, // Code for char A
{0xFF,0xFF,0xC3,0xC3,0xC3,0xC3,0xE7,0xFE,0xBC,0x00,0x00}, // Code for char B
{0xF0,0xFC,0x0E,0x07,0x03,0x03,0x03,0x07,0x0E,0x0C,0x00}, // Code for char C
{0xFF,0xFF,0x03,0x03,0x03,0x03,0x07,0x0E,0xFC,0xF0,0x00}, // Code for char D
{0xFF,0xFF,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0x03,0x03,0x00}, // Code for char E
{0xFF,0xFF,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0x03,0x03,0x00}, // Code for char F
{0xF0,0xFC,0x0E,0x07,0x03,0xC3,0xC3,0xC3,0xC7,0xC6,0x00}, // Code for char G
{0xFF,0xFF,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xFF,0xFF,0x00}, // Code for char H
{0x00,0x00,0x03,0x03,0xFF,0xFF,0x03,0x03,0x00,0x00,0x00}, // Code for char I
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00}, // Code for char J
{0xFF,0xFF,0xC0,0xE0,0xF0,0x38,0x1C,0x0E,0x07,0x03,0x00}, // Code for char K
{0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Code for char L
{0xFF,0xFF,0x1E,0x78,0xE0,0xE0,0x78,0x1E,0xFF,0xFF,0x00}, // Code for char M
{0xFF,0xFF,0x0E,0x38,0xF0,0xC0,0x00,0x00,0xFF,0xFF,0x00}, // Code for char N
{0xF0,0xFC,0x0E,0x07,0x03,0x03,0x07,0x0E,0xFC,0xF0,0x00}, // Code for char O
{0xFF,0xFF,0x83,0x83,0x83,0x83,0x83,0xC7,0xFE,0x7C,0x00}, // Code for char P
{0xF0,0xFC,0x0E,0x07,0x03,0x03,0x07,0x0E,0xFC,0xF0,0x00}, // Code for char Q
{0xFF,0xFF,0x83,0x83,0x83,0x83,0x83,0xC7,0xFE,0x7C,0x00}, // Code for char R
{0x3C,0x7E,0xE7,0xC3,0xC3,0xC3,0xC3,0xC7,0x8E,0x0C,0x00}, // Code for char S
{0x00,0x03,0x03,0x03,0xFF,0xFF,0x03,0x03,0x03,0x00,0x00}, // Code for char T
{0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00}, // Code for char U
{0x07,0x3F,0xF8,0xC0,0x00,0x00,0xC0,0xF8,0x3F,0x07,0x00}, // Code for char V
{0xFF,0xFF,0x00,0x00,0x80,0x80,0x00,0x00,0xFF,0xFF,0x00}, // Code for char W
{0x03,0x0F,0x1C,0x30,0xE0,0xE0,0x30,0x1C,0x0F,0x03,0x00}, // Code for char X
{0x03,0x0F,0x3C,0xF0,0xC0,0xC0,0xF0,0x3C,0x0F,0x03,0x00}, // Code for char Y
{0x03,0x03,0x03,0x03,0xC3,0xE3,0x33,0x1F,0x0F,0x03,0x00}, // Code for char Z
{0x00,0x00,0xFF,0xFF,0x03,0x03,0x03,0x03,0x00,0x00,0x00}, // Code for char [
{0x0E,0x1C,0x38,0x70,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00}, // Code for <Backslash>
{0x00,0x00,0x03,0x03,0x03,0x03,0xFF,0xFF,0x00,0x00,0x00}, // Code for char ]
{0x60,0x70,0x38,0x1C,0x0E,0x07,0x0E,0x1C,0x38,0x70,0x60}, // Code for char ^
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Code for char _
{0x00,0x00,0x00,0x00,0x3E,0x7E,0x4E,0x00,0x00,0x00,0x00}, // Code for char `
{0x00,0x40,0x60,0x60,0x60,0x60,0x60,0x60,0xE0,0xC0,0x00}, // Code for char a
{0xFF,0xFF,0xC0,0x60,0x60,0x60,0x60,0xE0,0xC0,0x80,0x00}, // Code for char b
{0x80,0xC0,0xE0,0x60,0x60,0x60,0x60,0x60,0xC0,0x80,0x00}, // Code for char c
{0x80,0xC0,0xE0,0x60,0x60,0x60,0xE0,0xC0,0xFF,0xFF,0x00}, // Code for char d
{0x80,0xC0,0xE0,0x60,0x60,0x60,0x60,0x60,0xC0,0x80,0x00}, // Code for char e
{0xC0,0xC0,0xFC,0xFE,0xC7,0xC3,0xC3,0x03,0x00,0x00,0x00}, // Code for char f
{0x80,0xC0,0xE0,0x60,0x60,0x60,0x60,0x60,0xE0,0xE0,0x00}, // Code for char g
{0xFF,0xFF,0xC0,0x60,0x60,0x60,0xE0,0xC0,0x80,0x00,0x00}, // Code for char h
{0x00,0x00,0x00,0x60,0xEC,0xEC,0x00,0x00,0x00,0x00,0x00}, // Code for char i
{0x00,0x00,0x00,0x00,0x00,0x60,0xEC,0xEC,0x00,0x00,0x00}, // Code for char j
{0x00,0xFF,0xFF,0x00,0x80,0xC0,0xE0,0x60,0x00,0x00,0x00}, // Code for char k
{0x00,0x00,0x00,0x03,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00}, // Code for char l
{0xE0,0xC0,0xE0,0xE0,0xC0,0xC0,0xE0,0xE0,0xC0,0x80,0x00}, // Code for char m
{0x00,0xE0,0xE0,0x60,0x60,0x60,0x60,0xE0,0xC0,0x80,0x00}, // Code for char n
{0x80,0xC0,0xE0,0x60,0x60,0x60,0x60,0xE0,0xC0,0x80,0x00}, // Code for char o
{0xE0,0xE0,0x60,0x60,0x60,0x60,0x60,0xE0,0xC0,0x80,0x00}, // Code for char p
{0x80,0xC0,0xE0,0x60,0x60,0x60,0x60,0x60,0xE0,0xE0,0x00}, // Code for char q
{0x00,0xE0,0xE0,0xC0,0x60,0x60,0x60,0x60,0xE0,0xC0,0x00}, // Code for char r
{0xC0,0xE0,0x60,0x60,0x60,0x60,0x60,0x40,0x00,0x00,0x00}, // Code for char s
{0x60,0x60,0xFE,0xFE,0x60,0x60,0x60,0x00,0x00,0x00,0x00}, // Code for char t
{0xE0,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0xE0,0x00}, // Code for char u
{0x60,0xE0,0x80,0x00,0x00,0x00,0x00,0x80,0xE0,0x60,0x00}, // Code for char v
{0xE0,0xE0,0x00,0x00,0xE0,0xE0,0x00,0x00,0xE0,0xE0,0x00}, // Code for char w
{0x60,0xE0,0xC0,0x80,0x00,0x80,0xC0,0xE0,0x60,0x00,0x00}, // Code for char x
{0x00,0x60,0xE0,0x80,0x00,0x00,0x80,0xE0,0x60,0x00,0x00}, // Code for char y
{0x60,0x60,0x60,0x60,0x60,0xE0,0xE0,0x60,0x20,0x00,0x00}, // Code for char z
{0x00,0x80,0xC0,0xFC,0x7E,0x07,0x03,0x03,0x03,0x00,0x00}, // Code for char {
{0x00,0x00,0x00,0x00,0xBF,0xBF,0x00,0x00,0x00,0x00,0x00}, // Code for char |
{0x00,0x03,0x03,0x03,0x07,0x7E,0xFC,0xC0,0x80,0x00,0x00}, // Code for char }
{0x10,0x18,0x0C,0x04,0x0C,0x18,0x10,0x18,0x0C,0x04,0x00}, // Code for char ~
{0x00,0x18,0x3C,0x66,0x66,0x3C,0x18,0x00,0x00,0x00,0x00}  // Code for <Degrees>
};
*/

/* Performs IO & LCD controller initialization */
void lcd_init(void)
{
    // Pull-up on reset pin
    LCD_PORT |= LCD_RST_PIN;
	
	// Set output bits on lcd port
	LCD_DDR |= LCD_RST_PIN | LCD_CE_PIN | LCD_DC_PIN | LCD_DATA_PIN | LCD_CLK_PIN;
    
	// Wait for reset (max 30ms)
	   chThdSleepMilliseconds(15);

//    _delay_ms(15); //not thread-safe
    
    // Toggle reset pin
    LCD_PORT &= ~LCD_RST_PIN;
  
  //  lcd_delay();
   chThdSleepMilliseconds(15);
//   _delay_ms(15); // not thread-safe
  
  LCD_PORT |= LCD_RST_PIN;

    // Disable controller
    LCD_PORT |= LCD_CE_PIN;

    lcd_send(0x21, LCD_CMD);  // Extended Command set
    lcd_send(0xC8, LCD_CMD);  // Set Contrast
    lcd_send(0x06, LCD_CMD);  // Set Tempco
    lcd_send(0x13, LCD_CMD);  // LCD bias mode 1:48
    lcd_send(0x20, LCD_CMD);  // Horizontal address, std commands
    lcd_send(0x0C, LCD_CMD);  // normal mode
    
    // Clear lcd
    lcd_clear();
	
}

/* Contrast */
void lcd_contrast(uint8_t contrast)
{
	lcd_send(0x21, LCD_CMD);				// LCD Extended Commands
    lcd_send(0x80 | contrast, LCD_CMD);		// Set LCD Vop(Contrast)
    lcd_send(0x20, LCD_CMD);				// LCD std cmds, hori addr mode
}

/* Clears the display */
void lcd_clear(void)
{
	lcd_current_position = 0;
	lcd_base_addr(lcd_current_position);
	
    // Set the entire cache to zero and write 0s to lcd
    for(int i=0;i<LCD_TOTAL;i++) {
		lcd_send(0, LCD_DATA);
    }
}

/* Sets cursor location to xy location corresponding to basic font size */
void lcd_goto_xy(uint8_t x, uint8_t y)
{
    lcd_current_position = (x-1)*6 + (y-1)*84;
}

/* Sets cursor location to xy location corresponding to basic font size */
void lcd_go_up_one(void)
{
    lcd_current_position -= 84;
}

/* Displays a character at current cursor location */
void lcd_chr(char chr)
{
	lcd_base_addr(lcd_current_position);
	uint16_t temp;

   if (bigfont ==0) {
		// 5 pixel wide characters and add space
		for(uint8_t i=0;i<5;i++) {
			//lcd_send(pgm_read_byte(&font5x7[chr-32][i]) << 1, LCD_DATA);
			temp = 5*(chr-32)+1+i;
			lcd_send(eeprom_read_byte((uint8_t*) temp) << 1, LCD_DATA);
		}
		lcd_send(0, LCD_DATA);
		lcd_current_position += 6;
	}
	
	if (bigfont ==1) {
		//11 pixel wide characters and add space
		for(uint8_t i=0;i<11;i++) {
			//lcd_send(pgm_read_byte(&font5x7[chr-32][i]) << 1, LCD_DATA);
			if (chr == 32) {
				temp = 11*(chr-32)+512+1+i;
			}
			else {
				temp = 11*(chr-43)+512+1+i;
			}
			//lcd_send(eeprom_read_byte((uint8_t*) temp) << 1, LCD_DATA);
			lcd_send(eeprom_read_byte((uint8_t*) temp), LCD_DATA);

		}
		
		lcd_current_position -= 84;
		lcd_base_addr(lcd_current_position);
		
		for(uint8_t i=0;i<11;i++) {
			//lcd_send(pgm_read_byte(&font5x7[chr-32][i]) << 1, LCD_DATA);
			if (chr == 32) {
				temp = 11*(chr-32)+689+1+i;
			}
			else {
				temp = 11*(chr-43)+689+1+i;
			}
			//lcd_send(eeprom_read_byte((uint8_t*) temp) << 1, LCD_DATA);
			lcd_send(eeprom_read_byte((uint8_t*) temp), LCD_DATA);
		}
		
		lcd_send(0, LCD_DATA);
		lcd_current_position += 84;
		lcd_current_position += 12;
	}
	
}


// Set the base address of the lcd
void lcd_base_addr(uint16_t addr) {
	lcd_send(0x80 |(addr % LCD_X_DIMENSION), LCD_CMD);
	lcd_send(0x40 |(addr / LCD_X_DIMENSION), LCD_CMD);
}

/* Sends data to display controller */
void lcd_send(uint8_t data, LcdCmdData cd)
{
	// Data/DC are outputs for the lcd (all low)
	LCD_DDR |= LCD_DATA_PIN | LCD_DC_PIN;
	
    // Enable display controller (active low)
    LCD_PORT &= ~LCD_CE_PIN;

    // Either command or data
    if(cd == LCD_DATA) {
		//if text invert on
		if (invert) data = ~data;
        LCD_PORT |= LCD_DC_PIN;
    } else {
        LCD_PORT &= ~LCD_DC_PIN;
    }
	
	for(uint8_t i=0;i<8;i++) {
	
		// Set the DATA pin value
		if((data>>(7-i)) & 0x01) {
			LCD_PORT |= LCD_DATA_PIN;
		} else {
			LCD_PORT &= ~LCD_DATA_PIN;
		}
		
		// Toggle clock
		LCD_PORT |= LCD_CLK_PIN;
		LCD_PORT &= ~LCD_CLK_PIN;
	}

	// Disable display control
    LCD_PORT |= LCD_CE_PIN;
	
	// Data/DC can be reused
	LCD_DDR &= ~(LCD_DATA_PIN | LCD_DC_PIN);
	LCD_PORT |= LCD_DATA_PIN | LCD_DC_PIN;
}

// Called by printf as a stream handler
int lcd_chr_printf(char var, FILE *stream) {
	lcd_chr(var);
	return 0;
}

