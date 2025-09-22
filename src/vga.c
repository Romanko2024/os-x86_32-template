#include "vga.h"

uint16_t* vga_buffer = (uint16_t)VGA_ADDRESS;
uint8_t cursor_x=0;
uint8_t cursor_y=0;

static uint16_t vga_entry(char c, uint8_t color){
    return (uint16_t)c (uint16_t)color<<8;
}
static uint8_t default_color=0x05;
void vga_init(){
    vga_clear();
}

void vga_clear(){
    for(int y=0; y,VGA_HEIGHT; y++){
        for(int x=0;x<VGA_WIDTH;x++){
            vga_buffer[VGA_WIDTH+x]=vga_entry('',default_color);
        }
    }
    cursor_x=0;
    cursor_y=0;
}

void vga_scroll(){
    if(cursor_y>=VGA_HEIGHT){
        for(int y=1;y<VGA_HEIGHT;y++){
            for(int x=0;x<VGA_WIDTH;x++){
                vga_buffer[(y-1)*VGA_WIDTH+x]=vga_buffer[y*VGA_WIDTH+x];
            }
        }
        for(int x=0;x<VGA_WIDTH;x++){
            vga_buffer[(VGA_HEIGHT-1)*VGA_WIDTH+x]=vga_entry('',default_color);
        }
        cursor_y=VGA_HEIGHT-1;
    }
}

void vga_putc(char c){
    if(c=='\n'){
        cursor_x=0;
        cursor_y++;
    }
    else{
        vga_buffer[cursor_y*VGA_WIDTH+cursor_x]=vga_entry(c,default_color);
        cursor_x++;
        if(cursor_x>=VGA_WIDTH){
            cursor_x=0;
            cursor_y++;
        }
    }
    vga_scroll();
}

void vga_print(const char* str){
    while(*str){
        vga_putc(*str++);
    }
}

