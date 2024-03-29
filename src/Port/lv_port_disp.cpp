#include "lv_port_disp.h"

// 用于初始化完毕后启用LVGL显示的TaskHandle_t
TaskHandle_t handleTaskLvgl;
// lvgl显示驱动
static lv_disp_drv_t disp_drv;

// lvgl更新任务
void TaskLvglUpdate(void* parameter) {
  // 阻塞在此处,直到xTaskNotifyGive
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  for (;;) {
    lv_task_handler();

    delay(5);
  }
}

/**
 * @brief  显示初始化
 * @param  无
 * @retval 无
 */
void Port_Init() {
  static TFT_eSPI screen;

  /* 屏幕初始化 */
  screen.begin();
  screen.initDMA(true);
  screen.setRotation(0);
  screen.fillScreen(TFT_BLACK);

  /* lvgl初始化 */
  lv_init();
  lv_port_disp_init(&screen);
  printf("lvInitDone\n");
  // 在核心2上执行LVGL
  xTaskCreatePinnedToCore(TaskLvglUpdate, "LvglThread", 20480, nullptr,
                          configMAX_PRIORITIES, &handleTaskLvgl, 1);
}

/**
 * @brief  自定义打印函数
 * @param  无
 * @retval 无
 */
void my_print(lv_log_level_t level, const char *file, uint32_t line,
              const char *fun, const char *dsc) {
  Serial.printf("%s@%d %s->%s\r\n", file, line, fun, dsc);
  Serial.flush();
}

/**
 * @brief  屏幕刷新回调函数
 * @param  disp:屏幕驱动地址
 * @param  area:刷新区域
 * @param  color_p:刷新缓冲区地址
 * @retval 无
 */
static void disp_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area,
                          lv_color_t *color_p) {
  TFT_eSPI *screen = (TFT_eSPI *)disp->user_data;

  int32_t w = (area->x2 - area->x1 + 1);
  int32_t h = (area->y2 - area->y1 + 1);

  screen->startWrite();
  screen->setAddrWindow(area->x1, area->y1, w, h);
  screen->pushPixelsDMA((uint16_t *)(&color_p->full), w * h);
  screen->endWrite();

  lv_disp_flush_ready(disp);
}


/**
 * @brief  屏幕初始化
 * @param  无
 * @retval 无
 */
void lv_port_disp_init(TFT_eSPI* scr) {
  lv_log_register_print_cb(reinterpret_cast<lv_log_print_g_cb_t>(
      my_print)); /* register print function for debugging */
  DMA_ATTR static lv_color_t *lv_disp_buf =
      static_cast<lv_color_t *>(heap_caps_malloc(
          DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA));
  static lv_disp_draw_buf_t disp_buf;
  lv_disp_draw_buf_init(&disp_buf, lv_disp_buf, nullptr,
                        DISP_BUF_SIZE);

  /*Initialize the display*/
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = DISP_HOR_RES;
  disp_drv.ver_res = DISP_VER_RES;
  disp_drv.flush_cb = disp_flush_cb;
  disp_drv.draw_buf = &disp_buf;
  disp_drv.user_data = scr;
  lv_disp_drv_register(&disp_drv);
}