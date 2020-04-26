#include <string.h> // for memset
#include <xc.h> // for the core timer delay
#include "font.h"
#include "ssd1306.h"



void drawChar(unsigned char x, unsigned char y, unsigned char l){
    char i,j,row,col=0;
    l = l-0x20;
    for (i=x;i<(x+5);i++){
        if (i < 0 || i > 127){return;}
        row = 0;
        for (j=y;j<(y+8);j++){
            if (j < 0 || j > 32){return;}
            ssd1306_drawPixel(i,j,((ASCII[l][col]>>row) & 1));
            row++;
        }
        col++;
    }
}

void drawString(char x, char y, char * m){
    char c=0, i=0,j=0;
    while(m[c] != 0){
        drawChar(x+i,y+j,m[c]);
        if (x+i < 128){
            i = i+6;
        } else{
            x = 0;
            i = 0;
            j = j+8;
        }
        if (y+j > 32){
            y = 0;
            j = 0;
        
        } 
        c++;
    }
    
}