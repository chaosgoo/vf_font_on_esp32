# 在ESP32上运行LVGL结合FreeType实现可变字体的显示

详细教程可见博客文章,本文为"LGVL配合FreeType为可变字体设置字重-ESP32篇(下)"的项目源码

* [LGVL配合FreeType为可变字体设置字重-模拟器篇](https://blog.chaosgoo.com/2021/12/12/FreeType-and-LVGL/)
* [LGVL配合FreeType为可变字体设置字重-ESP32篇(上)](https://blog.chaosgoo.com/2021/12/18/lvgl-freetype-and-esp32/)
* [LGVL配合FreeType为可变字体设置字重-ESP32篇(下)](https://blog.chaosgoo.com/2021/12/18/lvgl-freetype-and-esp32-2/)

# 前言

[上篇](https://blog.chaosgoo.com/2021/12/12/FreeType-and-LVGL/)中介绍了如何在在ESP32上运行的让LVGL,现在终于可以开始奔着[第一篇](https://blog.chaosgoo.com/2021/12/12/FreeType-and-LVGL/)文章末尾的效果去了.

## 准备工作

### 软件准备

访问[https://download.savannah.gnu.org/releases/freetype/](https://download.savannah.gnu.org/releases/freetype/)下载源码,解压后重命名为freetype并复制到项目lib文件夹下
我下载的是目前(2021.12.11)最新版本freetype-2.11.1.tar.gz(较老的版本不支持可变字体操作)
然后去这个repo获取SD和SPI库,复制到项目lib文件夹下
````bash
.
├── include
│   └── README
├── lib
│   └── README
│   └── lvgl_freetype
│       └── lvgl
│       └── freetype
│       └── SD
│       └── SPI
│   └── TFT_eSPI
├── platformio.ini
├── src
│   └── main.cpp
│   └── Port
│       └── lv_port_disp.cpp
│       └── lv_port_disp.h
└── test
    └── README
````

到freetype文件夹下,创建FreeType的**library.json**和**library.properties**文件
**library.properties**内容如下
````
name=FreeType
version=2.11.1
author=David Turner, Robert Wilhelm, Werner Lemberg
maintainer=Werner Lemberg
sentence=A freely available software library to render fonts.
paragraph=It is written in C, designed to be small, efficient, highly customizable, and portable while capable of producing high-quality output (glyph images) of most vector and bitmap font formats.documentation.
category=Font
url=https://freetype.org/
architectures=*
repository=https://gitlab.freedesktop.org/freetype/freetype
license=GNU
````

**library.json**内容如下
````json
{
  "name":"freetype",
  "version": "2.11.1",
  "description":"Software library to render fonts",
  "keywords":"freetype",
  "license": "FreeType License",
  "repository": {
    "type": "git",
    "url": "https://gitlab.freedesktop.org/freetype"
	},
  "frameworks": "*",
  "platforms": "*",
  "build": {
    "srcFilter": [
        "+<base/ftsystem.c>",
        "+<base/ftmm.c>",
        "+<base/ftinit.c>",
        "+<base/ftdebug.c>",

        "+<base/ftbase.c>",

        "+<base/ftbbox.c>",
        "+<base/ftglyph.c>",

        "+<base/ftbdf.c>",

        "+<bdf/bdf.c>",
        "+<cff/cff.c>",
        "+<truetype/truetype.c>",
        "+<sfnt/sfnt.c>",

        "+<smooth/smooth.c>",

        "+<cache/ftcache.c>",
        "+<gzip/ftgzip.c>",
        "+<base/ftbitmap.c>"
    ],
    "flags": [ "-DFT2_BUILD_LIBRARY", "-I include" ],
    "includeDir": "devel"
  }
}
````
### 硬件准备

和上篇相比,多了一个SD卡模块.

~~~起初也尝试了直接把字体文件存在SPIFF里面,减少涉及到的硬件,奈何速度慢的感人,于是回到这里重新写了SD卡版本.~~~

名称 | 数量 | 备注 | 图例 |
:-: | :-: | :-:
ESP32 开发板 | 1 | \ | <img src="https://chaosgoo-pic.oss-cn-shanghai.aliyuncs.com/blog_pic/lv_ft/esp32.jpg?x-oss-process=style/compress_50" width="240px" style="padding:30px"/>
1.54寸LCD | 1 | 驱动ST7789,分辨率240x240 |  <img src="https://chaosgoo-pic.oss-cn-shanghai.aliyuncs.com/blog_pic/lv_ft/lcd.jpg?x-oss-process=style/compress_50" width="240px" style="padding:30px"/>
杜邦线若干 | N | \ | <img src="https://chaosgoo-pic.oss-cn-shanghai.aliyuncs.com/blog_pic/lv_ft/wire.jpg?x-oss-process=style/compress_50" width="240px" style="padding:30px"/>
Micro SD卡模块和卡 | 1 | \ | <img src="https://chaosgoo-pic.oss-cn-shanghai.aliyuncs.com/blog_pic/lv_ft/sd_adapter.png" width="240px" style="padding:30px"/>

接线和点亮屏幕请去参考[LGVL配合FreeType为可变字体设置字重-ESP32篇(上)](https://blog.chaosgoo.com/2021/12/2021/12/18/lvgl-freetype-and-esp32/)


Micro SD卡模块的接线如下

ESP32引脚名称|Micro SD卡模块引脚名称
:-:|:-:
GND | GND 
G26 | MISO 
G13 | MOSI
G14| SCK
G15 | CS
5V(Vin) | VCC

## 准备完毕,开干

### 启用LVGL的FreeType

将lv_conf.h内的宏定义内容, 并启用FTC_SBitCache_Lookup
````c
/*FreeType library*/
#define LV_USE_FREETYPE 0
#define LV_USE_FREETYPE 1
#if LV_USE_FREETYPE
    /*Memory used by FreeType to cache characters [bytes] (-1: no caching)*/
    #define LV_FREETYPE_CACHE_SIZE (16 * 1024)
    #if LV_FREETYPE_CACHE_SIZE >= 0
        /* 1: bitmap cache use the sbit cache, 0:bitmap cache use the image cache. */
        /* sbit cache:it is much more memory efficient for small bitmaps(font size < 256) */
        /* if font size >= 256, must be configured as image cache */
        #define LV_FREETYPE_SBIT_CACHE 1
        /* Maximum number of opened FT_Face/FT_Size objects managed by this cache instance. */
        /* (0:use system defaults) */
        #define LV_FREETYPE_CACHE_FT_FACES 0
        #define LV_FREETYPE_CACHE_FT_SIZES 0
    #endif
#endif
````
0设置为1

### 配置FreeType的文件系统

创建一个命为ft_fs_port.cpp的文件,内容为
````cpp
#include "FS.h"
#include "SD.h"
#include "SPIFFS.h"

extern "C" {
typedef void lvbe_FILE;
lvbe_FILE *lvbe_fopen(const char *filename, const char *mode) {
  File f = SD.open(filename, mode);
  if (f) {
    File *f_ptr = new File(f);  // copy to dynamic object
    *f_ptr = f;                 // TODO is this necessary?
    return f_ptr;
  }
  return nullptr;
}
int lvbe_fclose(lvbe_FILE *stream) {
  File *f_ptr = (File *)stream;
  f_ptr->close();
  delete f_ptr;
  return 0;
}
size_t lvbe_fread(void *ptr, size_t size, size_t count, lvbe_FILE *stream) {
  File *f_ptr = (File *)stream;
  int32_t ret = f_ptr->read((uint8_t *)ptr, size * count);
  if (ret < 0) {  // error
    ret = 0;
  }
  return ret;
}
int lvbe_fseek(lvbe_FILE *stream, long int offset, int origin) {
  File *f_ptr = (File *)stream;
  fs::SeekMode mode = fs::SeekMode::SeekSet;
  if (SEEK_CUR == origin) {
    mode = fs::SeekMode::SeekCur;
  } else if (SEEK_END == origin) {
    mode = fs::SeekMode::SeekEnd;
  }
  bool ok = f_ptr->seek(offset, mode);
  return ok ? 0 : -1;
}
int lvbe_ftell(lvbe_FILE *stream) {
  File *f_ptr = (File *)stream;
  return f_ptr->position();
}
}
````

打开位于freetype文件夹下的**devel\ft2build.h**
并添加``
````c
#ifndef FT2BUILD_H_
#define FT2BUILD_H_

#define FT_CONFIG_MODULES_H  <ftmodule.h>
#define FT_CONFIG_OPTIONS_H  <ftoption.h>
#define FT_CONFIG_STANDARD_LIBRARY_H <ftstdlib.h>

#include <freetype/config/ftheader.h>

#endif /* FT2BUILD_H_ */
````
在同级目录下创建**ftstdlib.h**(文末附件栏有完整版)文件,内容从`freetype\include\freetype\config\ftstdlib.h`复制

修改file handling部分的内容为
````c
  /**************************************************************************
   *
   *                          file handling
   *
   */
#include <stdio.h>
typedef void lvbe_FILE;
extern lvbe_FILE * lvbe_fopen(const char * filename, const char * mode );
extern int lvbe_fclose(lvbe_FILE * stream);
extern size_t lvbe_fread(void * ptr, size_t size, size_t count, lvbe_FILE * stream);
extern int lvbe_fseek(lvbe_FILE * stream, long int offset, int origin );
extern int lvbe_ftell(lvbe_FILE * stream);

#define FT_FILE     lvbe_FILE
#define ft_fclose   lvbe_fclose
#define ft_fopen    lvbe_fopen
#define ft_fread    lvbe_fread
#define ft_fseek    lvbe_fseek
#define ft_ftell    lvbe_ftell
#define ft_sprintf  sprintf
````

在同级目录下创建**ftmodule.h**,启用所需模块
内容为
````cpp
FT_USE_MODULE( FT_Driver_ClassRec, tt_driver_class )
FT_USE_MODULE( FT_Module_Class, sfnt_module_class )
FT_USE_MODULE( FT_Renderer_Class, ft_smooth_renderer_class )
````

**在同级目录下创建**ftoption.h**, 内容从`freetype\include\freetype\config\ftoptions.h`复制,详细的配置过长,故此处省略,见文末附件**

此时初步FreeType的基本功能配置完成

主要的涉及到**freetype\devel**下的4个文件
````bash
│       └── freetype
│         └── devel
│           └── ft2build.h
│           └── ftmodule.h
│           └── ftoption.h
│           └── ftstdlib.h
````
### 测试FreeType移植结果

打开main.cpp 随便写点内容,烧录到ESP32上运行查看效果.

````cpp
#include <Arduino.h>

#include "./Port/lv_port_disp.h"
#include "SD.h"
#include "SPI.h"

// 初始化SD卡
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

  Port_Init();
  if (!SD_Init()) {
    Serial.println("SPIFFS Mount Failed");
  };
  /*创建字体*/
  static lv_ft_info_t info;
  /*在SD卡根目录下放置前一节中的可变字体文件,重命名为archivo.ttf*/
  info.name = "/archivo.ttf";
  info.weight = 18;
  info.style = FT_FONT_STYLE_NORMAL;
  if (!lv_ft_font_init(&info)) {
    LV_LOG_ERROR("create failed.");
  } else {
    LV_LOG_ERROR("create done.");
  }

  /*为新字体创建Style*/
  static lv_style_t style;
  lv_style_init(&style);
  lv_style_set_text_font(&style, info.font);
  lv_style_set_text_color(&style, lv_color_black());

  /*应用Style到Label*/
  lv_obj_t *altria = lv_label_create(lv_scr_act());
  lv_obj_t *shirou = lv_label_create(lv_scr_act());
  // lv_obj_add_style(altria, &style, 0);

  lv_label_set_text(altria, "Toou.\nAnata wa watashi no masuta ka?");
  lv_label_set_text(shirou, "Master?");
  lv_obj_set_style_text_font(shirou, info.font, 0);
  lv_obj_center(shirou);

  // 一切就绪, 启动LVGL任务
  xTaskNotifyGive(handleTaskLvgl);
}

void loop() {}
````

运行效果如下

![Weight=900](https://chaosgoo-pic.oss-cn-shanghai.aliyuncs.com/blog_pic/lv_ft/freetype_demo_on_esp32.png)

### 拓展LVGL的FreeType

就像前篇内容中的一样,需要手动地为当前版本LVGL添加可变字体支持.

**lv_freetype.h**内的**lv_ft_info_t**修改为
````cpp
typedef struct {
    const char * name;  /* The name of the font file */
    const void * mem;   /* The pointer of the font file */
    size_t mem_size;    /* The size of the memory */
    lv_font_t * font;   /* point to lvgl font */
    uint16_t height;    /* font size */
    uint16_t weight;    /* font weight */
    uint16_t style;     /* font style */
} lv_ft_info_t;
````

然后去**lv_freetype.c**里添加可变字体操作代码
````c
  FT_MM_Var *amaster = NULL;
  FT_Error err = FT_Get_MM_Var(face, &amaster);
  if (err) {
    LV_LOG_ERROR("FT_Get_MM_Var error:%d\n", err);
    return err;
  }
  FT_Fixed w = dsc->weight << 16;
  if (w > amaster->axis->maximum) {
    w = amaster->axis->maximum;
  }
  err = FT_Set_Var_Design_Coordinates(face, 1, &w);
  if (err) {
    LV_LOG_ERROR("FT_Set_Var_Design_Coordinates error:%d\n", err);
    return err;
  }
  FT_Done_MM_Var(library, amaster);
````

**别忘记我们已经修改了weight的含义,此时字体大小已经由height决定了**

回到main.cpp,在创建字体初时候,设置weight为100

```cpp
static lv_ft_info_t info;
info.name = "/archivo.ttf";
info.height = 18;
info.weight = 100;
info.style = FT_FONT_STYLE_NORMAL;
````

然后烧录到ESP32,可以看到字重已经降低了.

![Weight=900](https://chaosgoo-pic.oss-cn-shanghai.aliyuncs.com/blog_pic/lv_ft/freetype_demo_on_esp32_100_weight.png)

## 动起来吧,字重

利用LVGL的lv_timer_t或者lv_anim_t动态的修改字重

````cpp
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
````

````cpp
  lv_timer_t *weightTimer = lv_timer_create(onTimer, 50, &label);
  // 一切就绪, 启动LVGL任务
  xTaskNotifyGive(handleTaskLvgl);
````

效果图

![dynamic](https://chaosgoo-pic.oss-cn-shanghai.aliyuncs.com/blog_pic/lv_ft/img.gif)



## 环境:
````
Espressif 32 (3.4.0) > ESP32 Pico Kit
framework-arduinoespressif32 3.10006.210326 (1.0.6)
tool-esptoolpy 1.30100.210531 (3.1.0)
toolchain-xtensa32 2.50200.97 (5.2.0)
<lvgl> 8.1.1-dev
<TFT_eSPI> 2.3.89
esptool.py v3.1
````

# 参考资料
* https://github.com/lvgl/lvgl
* https://github.com/peng-zhihui/Peak
* https://www.youtube.com/watch?v=LY3ypzPcDCE