#include "imu.h"
#include "I2c_master_noint.h"
//#include
void imu_setup(){
    unsigned char who = 0;
    //read from whoami
    who = readPin(IMU_ADDR,IMU_WHOAMI);
    if (who != 0b01101001){
        while(1){}
    }
    //init imu_ctrl1_xl
    writePin(IMU_ADDR,IMU_CTRL1_XL,0b10000010);
    //init imu_ctrl2_g
    writePin(IMU_ADDR,IMU_CTRL2_G,0b10001000);
    //init imu_ctrl3_c
    writePin(IMU_ADDR,IMU_CTRL3_C,0b00000100);
}


void imu_read(unsigned char reg, signed short * data, int len){
    unsigned char vals[len*2];
    char i,j=0;
    i2c_read_multi(IMU_ADDR,reg,vals,len*2);
    for (i=0;i<len;i++){
        data[i] = (vals[j+1]<< 8) | vals[j]; //shift the high bits(j+1) and or with low bits(j)
        j=j+2;
    }
}