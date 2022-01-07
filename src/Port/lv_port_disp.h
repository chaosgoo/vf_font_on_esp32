#ifndef LV_PORT_DISP_H_
#define LV_PORT_DISP_H_

#include "TFT_eSPI.h"
#include "lvgl.h"

#define DISP_HOR_RES 240
#define DISP_VER_RES 240
#define DISP_BUF_SIZE (DISP_HOR_RES*DISP_VER_RES/4)

extern TaskHandle_t handleTaskLvgl;
void Port_Init();
void lv_port_disp_init(TFT_eSPI* scr);
void lv_fs_if_init();
#endif