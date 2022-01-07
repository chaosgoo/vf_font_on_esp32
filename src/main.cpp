#include <Arduino.h>

#include "./Port/lv_port_disp.h"
#include "SD.h"
#include "SPI.h"

typedef struct _VF_Label {
  lv_obj_t *label;
  lv_ft_info_t *font;
} VF_Label;

static void onTimer(lv_timer_t *timer) {
  static uint16_t weight = 100;
  static uint16_t size = 18;
  VF_Label *vfl = (VF_Label *)(timer->user_data);
  if (vfl == nullptr) {
    printf("vfl == nullptr\n");
  }

  if (vfl == nullptr) {
    printf("vfl->font->font == nullptr\n");
  }

  lv_ft_font_destroy(vfl->font->font);
  printf("lv_ft_font_destroy\n");
  vfl->font->name = "/archivo.ttf";
  vfl->font->height = size;
  vfl->font->weight = weight;
  vfl->font->style = FT_FONT_STYLE_NORMAL;
  lv_ft_font_init(vfl->font);
  printf("lv_ft_font_init\n");
  size = 48;
  weight += 100;
  if (weight > 600) {
    weight = 100;
  }
  lv_obj_set_style_text_font(vfl->label, vfl->font->font, 0);
}

bool SD_Init() {
  pinMode(22, INPUT);

  SPIClass *sd_spi = new SPIClass(HSPI);     // another SPI
  if (!SD.begin(15, *sd_spi, 12000000, ""))  // SD-Card SS pin is 15
  {
    Serial.println("Card Mount Failed");
    return false;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return false;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB, used size:%llu\n", cardSize,
                SD.usedBytes());

  return true;
}

void setup() {
  Serial.begin(115200);  // Set to a high rate for fast image transfer to a PC

  Port_Init();SD
  if (!SD_Init()) {
    Serial.println("SD Mount Failed");
  };
  /*创建字体*/
  static lv_ft_info_t info;
  /**/
  info.name = "/archivo.ttf";
  info.height = 18;
  info.weight = 100;
  info.style = FT_FONT_STYLE_NORMAL;
  if (!lv_ft_font_init(&info)) {
    LV_LOG_ERROR("create failed.");
  } else {
    LV_LOG_ERROR("create done.");
  }

  lv_obj_t *altria = lv_label_create(lv_scr_act());
  lv_obj_t *shirou = lv_label_create(lv_scr_act());

  lv_label_set_text(altria, "Toou.\nAnata wa watashi no masuta ka?");
  lv_label_set_text(shirou, "Master?");
  lv_obj_set_style_text_font(shirou, info.font, 0);
  lv_obj_center(shirou);

  static VF_Label label = VF_Label{};

  label.font = &info;
  label.label = shirou;

  lv_timer_t *weightTimer = lv_timer_create(onTimer, 50, &label);
  // 一切就绪, 启动LVGL任务
  xTaskNotifyGive(handleTaskLvgl);
}

void loop() {}