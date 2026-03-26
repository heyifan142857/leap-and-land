//
// Created by user on 2024/1/18.
//

#ifndef LEAP_AND_LAND_DISPLAY_H
#define LEAP_AND_LAND_DISPLAY_H

#include <common.h>

void display_quit_cache(void);
void display_image(const char* filePath,int x,int y);
void display_font(const char* filePath,const char* text,int size,int r,int g,int b,int x,int y);
void display_font_alpha(const char* filePath,const char* text,int size,int r,int g,int b,Uint8 alpha,int x,int y);


#endif //LEAP_AND_LAND_DISPLAY_H
