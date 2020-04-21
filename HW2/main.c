#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include"spi.h"
#include<math.h>

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

//void initSPI();
//unsigned char spi_io(unsigned char o);
#define NUMSAMPS 100
#define PI 3.14159
static volatile float Waveform[NUMSAMPS];

void delay(void){
    while(_CP0_GET_COUNT() < 24000000/10);
}

unsigned short percentWave(float z){
    float per;
    per = z/3.3; //Percentage of voltage
    return 4095*per; //represent percentage as short
}

void makeSineWaveform(){
    int i = 0,j=0;
    for (i = 0;i < NUMSAMPS; ++i){
        Waveform[i] = 1.65*sin(4*PI*j)+1.65;
        j=j+0.01;
    }
}

unsigned short genAnalogSig(unsigned char channel, unsigned short volt){
    unsigned short p=0;
    p = channel << 15;// Set channel select bit
    p = p | (0b111 << 12); //Set other config bits
    p = p | volt; //Set DAC bits
    return p;
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
    
    initSPI();

    __builtin_enable_interrupts();
    makeSineWaveform();
    unsigned char i = 0;
    unsigned short v,a,b;
    float x;
    while (1) {
        for(i=0;i<NUMSAMPS;++i){
            v = percentWave(Waveform[i]);
            a = genAnalogSig(0,v);// DACa = 0
            v = percentWave(2.5);
            b = genAnalogSig(1,v);// DACb = 1
            LATAbits.LATA0 = 0;
            spi_io(a>>8);
            spi_io(a);
            LATAbits.LATA0 = 1;
            _CP0_SET_COUNT(0);
            delay();
            LATAbits.LATA0 = 0;
            spi_io(b>>8);
            spi_io(b);
            LATAbits.LATA0 = 1;
            _CP0_SET_COUNT(0);
            delay();
        }
    }
}



