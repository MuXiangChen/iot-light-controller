# 物联网灯光控制器 (IoT Light Controller)

<p align="center">
  <img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License: MIT">
  <img src="https://img.shields.io/badge/Platform-IoT-green.svg" alt="Platform: IoT">
  <img src="https://img.shields.io/badge/Framework-Arduino-orange.svg" alt="Framework: Arduino">
  <img src="https://img.shields.io/badge/Backend-Node.js-lightgreen.svg" alt="Backend: Node.js">
  <img src="https://img.shields.io/badge/Mobile-Flutter-blue.svg" alt="Mobile: Flutter">
  <img src="https://img.shields.io/badge/Web-Angular-red.svg" alt="Web: Angular">
</p>

## 📖 项目简介

这是一个基于物联网技术的智能灯光控制器项目，支持通过网络远程控制 0-10V 调光电源。该项目包含完整的硬件设计、固件开发、后端服务、移动应用和 Web 管理后台，为学习者提供了一个完整的物联网解决方案参考。

## ✨ 特性

- 🔌 **标准接口**：12V 电源、DIM+ 调光信号、GND 接地三线接口
- 📡 **双网连接**：主用 WiFi 连接，4G 网络智能备份
- 🛡️ **网络冗余**：WiFi 断网时自动切换至 4G，确保连接稳定性
- 📶 **广域覆盖**：支持远程部署，不受 WiFi 覆盖范围限制
- 🎛️ **精确调光**：支持 0-10V 调光协议，兼容市面上主流调光电源
- 📱 **多端控制**：支持 Flutter 移动应用和 Angular Web 后台
- 🔧 **完整方案**：从硬件到软件的完整开源解决方案

## 🏗️ 项目架构

```
iot-light-controller/
├── hardware/          # 硬件设计
│   ├── schematic/     # 原理图文件
│   └── pcb/          # PCB 设计文件
├── firmware/          # 固件代码
│   ├── src/          # Arduino 源代码
│   ├── lib/          # 库文件
│   └── platformio.ini # PlatformIO 配置
├── server/           # 后端服务器
│   ├── api/          # API 接口
│   ├── models/       # 数据模型
│   └── config/       # 配置文件
├── flutter/          # Flutter 移动应用
│   ├── lib/          # 应用源代码
│   └── pubspec.yaml  # 依赖配置
├── web/              # Angular Web 管理后台
│   ├── src/          # Web 应用源代码
│   └── package.json  # 依赖配置
└── README.md
```

## 🔌 硬件规格

### 接口定义
| 引脚 | 功能 | 电压范围 | 描述 |
|------|------|----------|------|
| 12V  | 电源输入 | 12V DC | 为控制器供电 |
| DIM+ | 调光信号 | 0-10V | 输出至调光电源的控制信号 |
| GND  | 接地 | 0V | 公共接地线 |

### 技术参数
- 工作电压：12V DC
- 调光输出：0-10V 模拟信号
- 无线连接：WiFi 802.11 b/g/n
- 工作温度：-20°C ~ +70°C
- 控制精度：12-bit (0-4095)

## 🚀 快速开始

### 硬件准备

1. 按照 `hardware/` 目录中的原理图和 PCB 文件制作电路板
2. 连接 12V 电源、调光电源的 DIM+ 接口和公共地线

### 固件烧录

1. 安装 [PlatformIO](https://platformio.org/)
2. 克隆项目仓库：
   ```bash
   git clone https://github.com/MuXiangChen/iot-light-controller.git
   cd iot-light-controller/firmware
   ```
3. 烧录固件：
   ```bash
   pio run --target upload
   ```

### 服务器部署

1. 进入服务器目录：
   ```bash
   cd server
   npm install
   ```
2. 配置环境变量并启动：
   ```bash
   npm start
   ```

### 移动应用

1. 安装 [Flutter](https://flutter.dev/)
2. 进入 Flutter 目录：
   ```bash
   cd flutter
   flutter pub get
   flutter run
   ```

### Web 管理后台

1. 进入 Web 目录：
   ```bash
   cd web
   npm install
   ng serve
   ```

## 📱 功能演示

### 移动应用功能
- 设备扫描与连接
- 实时亮度调节
- 定时控制
- 场景模式

### Web 管理后台功能
- 设备管理
- 用户权限控制
- 数据统计
- 系统配置

## 🔧 开发环境

### 硬件开发
- **PCB 设计**：KiCad / Altium Designer
- **原理图**：支持主流 EDA 工具

### 固件开发
- **框架**：Arduino Framework
- **IDE**：PlatformIO
- **芯片**：ESP32 / ESP8266

### 后端开发
- **运行时**：Node.js
- **框架**：Express.js
- **数据库**：MongoDB / MySQL
- **API**：RESTful API

### 前端开发
- **移动端**：Flutter (Dart)
- **Web 端**：Angular (TypeScript)
- **UI 库**：Material Design

## 🤝 贡献指南

欢迎大家为这个项目做出贡献！请遵循以下步骤：

1. Fork 本仓库
2. 创建您的特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交您的更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开一个 Pull Request

## 📄 许可证

本项目基于 MIT 许可证开源 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 📞 联系我们

- 项目维护者：Mo
- 邮箱：uaena_mo@Outlook.com
- 项目链接：[https://github.com/MuXiangChen/iot-light-controller](https://github.com/MuXiangChen/iot-light-controller)

## 🙏 致谢

- 感谢所有为这个项目做出贡献的开发者
- 感谢开源社区提供的优秀工具和库
- 特别感谢在项目开发过程中提供帮助的朋友们

<!-- ## 📚 学习资源

- [0-10V 调光协议详解](docs/dimming-protocol.md)
- [ESP32 开发指南](docs/esp32-guide.md)
- [Flutter 物联网开发](docs/flutter-iot.md)
- [Angular 后台管理系统开发](docs/angular-admin.md) -->

---

<p align="center">
  如果这个项目对您有帮助，请给它一个 ⭐️
</p>