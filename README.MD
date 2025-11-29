# 在 LVGL 中实现可变字体 (Variable Font)

> **⚠️ 说明 / Note:**
> 本仓库包含项目的**完整源代码**。
> 关于代码的**详细实现原理、FreeType 配置逻辑及踩坑记录**，请移步我的博客阅读完整图文教程。

## 📚 完整教程 / Full Tutorials

这个项目涉及的知识点较多，我将其拆分为三篇深度文章进行解析：

* **1. 基础原理与模拟：**
    [👉 在 LVGL 中实现可变字体 (Variable Font) - 第一章](https://chaosgoo.com/2021/12/12/FreeType-and-LVGL/)
    *(LVGL 基本概念与 PC 模拟器环境搭建)*

* **2. 移植到 ESP32 (上)：**
    [👉 在 LVGL 中实现可变字体 (Variable Font) - 第二章](https://chaosgoo.com/2021/12/18/lvgl-freetype-and-esp32/)
    *(如何将 LVGL 库移植到 ESP32)*

* **3. 性能优化与实战 (下)：**
    [👉 在 LVGL 中实现可变字体 (Variable Font) - 第三章](https://chaosgoo.com/2022/01/03/lvgl-freetype-and-esp32-2/)
    *(ESP32 上实现可变字体渲染与性能优化)*

## 🛠️ 如何编译 / Build

本项目基于 PlatformIO 构建。

1. Clone 本仓库
2. 使用 VSCode + PlatformIO 打开
3. 点击 `Build` 即可
