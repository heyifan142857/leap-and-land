//
// Created by user on 2024/1/16.
//

#ifndef LEAP_AND_LAND_MENU_H
#define LEAP_AND_LAND_MENU_H

#define NUM_WIDGETS 3
#define WIDGET_X 450
#define WIDGET_Y_TOP 270
#define WIDGET_Y_GAP 80

#include <common.h>
#include <utils/input.h>
#include <utils/display.h>

typedef struct {
    const char *text;
    int x, y;
    void (*action)();
} Widget;

void do_menu_logic();

#endif //LEAP_AND_LAND_MENU_H
