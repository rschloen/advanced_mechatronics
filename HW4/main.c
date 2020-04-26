#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include<string.h>
#include"I2c_master_noint.h"
#include"ssd1306.h"
#include "font.h"
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
//#define PI 3.14159

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
    __builtin_enable_interrupts();
    unsigned char msg[30];
    float time=0,j;
    unsigned char add = 0b01000000; //Address always sent as write, changed to read in read func.
    
    writePin(add,0x00,0x00);
    writePin(add,0x01,0xFF);
    
    while (1) {
        
        //Heartbeat led
        LATAbits.LATA4 = 1;
        delay(20);
        LATAbits.LATA4 = 0;
        delay(20);
        _CP0_SET_COUNT(0);
        
        if (readPin(add,0x13)){ //GPIOB
            writePin(add,0x14,0b00000000); //OLATA
        } else{
            writePin(add,0x14,0b10000000); //OLATA
        }
        sprintf(msg, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        drawString(0,LINE1,msg);
        sprintf(msg, "Hello World!");
        drawString(0,LINE2,msg);
        sprintf(msg, "i = %.0f",j);
        drawString(0,LINE3,msg);
        ssd1306_update();
        time = _CP0_GET_COUNT();
        j++;
        sprintf(msg, "FPS = %.2f", 1/(time/48000000));
        drawString(60,LINE4,msg);
        
        
               
        
    }
}



