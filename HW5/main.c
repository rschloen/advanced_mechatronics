#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include<string.h>
#include"I2c_master_noint.h"
#include"ssd1306.h"
#include "font.h"
#include"ws2812b.h"
// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz  2
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
//
//
#define LINE1 0
#define LINE2 8
#define LINE3 16
#define LINE4 24
#define NUM_LEDS 4

void delay(int t){
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT() < 24000000/t);
}


int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;
    TRISBbits.TRISB4 = 1;
    
    i2c_master_setup();
    ssd1306_setup();
    ws2812b_setup();
    __builtin_enable_interrupts();
    unsigned char msg[30];
    float time=0,j,hue;
    unsigned char add = 0b01000000; //Address always sent as write, changed to read in read func.
    wsColor c[NUM_LEDS];
    writePin(add,0x00,0x00);
    writePin(add,0x01,0xFF);
    hue = 0;
    while (1) {
        
        //Heartbeat led
        LATAbits.LATA4 = 1;
        delay(20);
        LATAbits.LATA4 = 0;
        delay(20);
//        _CP0_SET_COUNT(0);
        
        c[0] = HSBtoRGB(hue, 1, .2);
        if (hue+30 < 360){
            c[1] = HSBtoRGB(hue+30, 1, .2);
        }else{
            c[1] = HSBtoRGB((hue+30)- 360, 1, .2);
        }
        if (hue+60 < 360){
            c[2] = HSBtoRGB(hue+60, 1, .2);
        }else{
            c[2] = HSBtoRGB((hue+60)- 360, 1, .2);
        }
        if (hue+90 < 360){
            c[3] = HSBtoRGB(hue+90, 1, .2);
        }else{
            c[3] = HSBtoRGB((hue+90)- 360, 1, .2);
        }
        ws2812b_setColor(c, NUM_LEDS);
        
        if (hue < 360){
            hue = hue +7;
        } else{
            hue = 0;
        }
        
        
        
//        ssd1306_update();s
        
        
        
               
        
    }
}



