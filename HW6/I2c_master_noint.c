// I2C Master utilities, using polling rather than interrupts
// The functions must be called in the correct order as per the I2C protocol
// I2C pins need pull-up resistors, 2k-10k
#include "I2c_master_noint.h"
#include "ssd1306.h"
#include "font.h"
#include <stdio.h>

void i2c_master_setup(void) {
    // using a large BRG to see it on the nScope, make it smaller after verifying that code works
    // look up TPGD in the datasheet
    I2C1BRG = 100; // I2CBRG = [1/(2*Fsck) - TPGD]*Pblck - 2 (TPGD is the Pulse Gobbler Delay)
    I2C1CONbits.ON = 1; // turn on the I2C1 module
}

void i2c_master_start(void) {
    I2C1CONbits.SEN = 1; // send the start bit
    while (I2C1CONbits.SEN) {
        ;
    } // wait for the start bit to be sent
}

void i2c_master_restart(void) {
    I2C1CONbits.RSEN = 1; // send a restart 
    while (I2C1CONbits.RSEN) {
        ;
    } // wait for the restart to clear
}

void i2c_master_send(unsigned char byte) { // send a byte to slave
    I2C1TRN = byte; // if an address, bit 0 = 0 for write, 1 for read
    while (I2C1STATbits.TRSTAT) {
        ;
    } // wait for the transmission to finish
    if (I2C1STATbits.ACKSTAT) { // if this is high, slave has not acknowledged
        // ("I2C1 Master: failed to receive ACK\r\n");
        while(1){} // get stuck here if the chip does not ACK back
    }
}

unsigned char i2c_master_recv(void) { // receive a byte from the slave
    I2C1CONbits.RCEN = 1; // start receiving data
    while (!I2C1STATbits.RBF) {
        ;
    } // wait to receive the data
    return I2C1RCV; // read and return the data
}

void i2c_master_ack(int val) { // sends ACK = 0 (slave should send another byte)
    // or NACK = 1 (no more bytes requested from slave)
    I2C1CONbits.ACKDT = val; // store ACK/NACK in ACKDT
    I2C1CONbits.ACKEN = 1; // send ACKDT
    while (I2C1CONbits.ACKEN) {
        ;
    } // wait for ACK/NACK to be sent
}

void i2c_master_stop(void) { // send a STOP:
    I2C1CONbits.PEN = 1; // comm is complete and master relinquishes bus
    while (I2C1CONbits.PEN) {
        ;
    } // wait for STOP to complete
}


void writePin(unsigned char adr, unsigned char reg, unsigned char val){
    i2c_master_start();
//    adr = adr | 0; //Force address to set to write
    i2c_master_send(adr);
    i2c_master_send(reg);
    i2c_master_send(val);
    i2c_master_stop();
}

unsigned char readPin(unsigned char adr, unsigned char reg){
    unsigned char val;
    i2c_master_start();
//    adr = adr & 0x00; //Force address to set to write
    i2c_master_send(adr);
    i2c_master_send(reg);
    i2c_master_restart();
    adr = adr | 0b00000001; //Force address to set to read
    i2c_master_send(adr);
    val = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
    return val;
}

void i2c_read_multi(unsigned char adr, unsigned char reg, unsigned char * data, int len){
    char l,m[30];       
//    sprintf(m,"Add = %d ",adr);
//    drawString(0,8,m);
//    ssd1306_update();
    i2c_master_start();
    i2c_master_send(adr);
    i2c_master_send(reg);
    sprintf(m,"Reg = %d",reg);
    drawString(0,16,m);
    ssd1306_update();
    i2c_master_restart();
    adr = adr | 0b00000001;
    sprintf(m,"Add = %d ",adr);
    drawString(0,8,m);
    ssd1306_update();

    for (l=0;l<len-1;l++){
        sprintf(m,"l = %d",l);
        drawString(0,16,m);
        ssd1306_update();
        data[l] = i2c_master_recv();
        i2c_master_ack(0);
    }
    data[l+1] = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
}