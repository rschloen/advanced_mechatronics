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


#define NUMSAMPS 100
#define PI 3.14159
static volatile float SINE[NUMSAMPS],TRI[NUMSAMPS];

void delay(int t){
    while(_CP0_GET_COUNT() < 24000000/t);
}

unsigned short percentWave(float z){
    float per;
    per = z/3.3; //Percentage of voltage
    return 4095*per; //represent percentage as short
}

void makeWaveform(){
    int i = 0;
    float t=0,f;
    for (i = 0;i < NUMSAMPS; ++i){
        SINE[i] = 1.65*sin(4*PI*t)+1.65; //sin(2*pi*2hz*t) 2 Hz sine wave 
        f = 3.3*2*(t-floor(t+.5)); //1 Hz triangle wave (absolute value of sawtooth)
        if (f<0){ //absolute value for float
            f = -f;
        }
        TRI[i] = f;
        t=t+0.01; //time
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
    makeWaveform();
    unsigned char i = 0;
    unsigned short v,a,b;

    while (1) {
        for(i=0;i<NUMSAMPS;++i){ //Loop through signal
            v = percentWave(SINE[i]); //Get volt as short
            a = genAnalogSig(0,v);// DACa = 0
            v = percentWave(TRI[i]); //Get volt as short
            b = genAnalogSig(1,v);// DACb = 1
            LATAbits.LATA0 = 0;
            spi_io(a>>8); //send highest 8 bits of short
            spi_io(a); //send last 8 bits of short
            LATAbits.LATA0 = 1;
            LATAbits.LATA0 = 0;
            spi_io(b>>8);
            spi_io(b);
            LATAbits.LATA0 = 1;
            _CP0_SET_COUNT(0);
            delay(100); //wait 100th of a second
        }
    }
}



