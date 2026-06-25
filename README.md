# 低功耗智能健康监护可穿戴设备

基于 STM32F407VETx + FreeRTOS 的边缘 AI 低功耗智能健康监护穿戴设备，融合**端侧神经网络跌倒检测**、**多传感器数据采集**、**MQTT 远程通信**与**双分区 OTA 安全升级**，实现生命体征监测及远程告警功能。

## 硬件规格

| 类别 | 器件 | 接口 |
|------|------|------|
| MCU | STM32F407VETx (Cortex-M4F, 168 MHz, 512 KB Flash, 192 KB SRAM) | — |
| 显示 | SSD1306 OLED 128×64 单色 | 软件 I2C |
| 姿态传感器 | MPU6050 六轴 IMU | 硬件 I2C1 |
| WiFi 模块 | ESP8266 (MQTT 通信) | USART3 |
| 外部 Flash | W25Q128 (16 MB) | SPI1 |
| EEPROM | AT24C02 (256 B) | 硬件 I2C1 |
| 输入 | 实体按键 | GPIO |
| 指示 | 红色/蓝色 LED | GPIO |
| 调试 | 串口日志 | USART1 (115200 bps) |

## 软件架构

```
App/
├── Config/          FreeRTOS 配置、系统调参 (含跌倒检测参数)
├── Model/           共享数据模型 (DataCenter)、时间设置
├── Tasks/           4 个 FreeRTOS 任务 (传感器/显示/按键/启动)
├── UI/
│   ├── Pages/       10 个页面 (时钟/菜单/秒表/LED/姿态/游戏/表情/水平仪/设置/计步)
│   └── UI_Manager   页面路由管理
Drivers/
├── BSP/             硬件驱动层 (OLED/MPU6050/W25Q128/AT24C02/ESP8266/按键/USART 等)
├── CMSIS/           ARM Cortex-M4 核心支持
└── STM32F4xx_StdPeriph_Driver/  标准外设库
FreeRTOS/            FreeRTOS V11.1.0
Middlewares/
├── Algorithm/       Kalman 滤波、计步、端侧 NN 跌倒检测
└── Utils/           调试串口
Bootloader/          独立引导程序项目
```

### RTOS 任务设计

| 任务 | 优先级 | 周期 | 职责 |
|------|--------|------|------|
| StartTask | 4 | 一次性 (自删除) | 系统初始化、OTA 测试菜单、创建其他任务 |
| KeyTask | 3 | 10 ms | 按键事件处理、页面交互、跌倒告警清除 |
| DisplayTask | 2 | 30~1000 ms (按页面动态调整) | OLED 渲染、自动休眠/调暗、跌倒告警界面 |
| MPU6050Task | 1 | 5 ms / 100 ms | 传感器采集、Kalman 姿态解算、计步、抬腕唤醒、**跌倒检测** |

## 功能特性

### 端侧 AI 跌倒检测

- **两级检测流水线**：加速度差分预滤波 (0.8g) → 3 层全连接神经网络 (16→16→8→1)
- **实时特征提取**：250ms 滑动窗口 (50 样本 @ 200Hz)，16 维时域统计与尖峰特征
- **联合决策**：NN 概率 + 连续窗口确认 + 10s 冷却期，有效抑制误报
- **资源占用**：仅 1.7KB Flash (权重) + ~5KB 代码，单次推理 <20μs @ 168MHz
- **可演进**：预留串口数据采集模式，支持真实数据离线重训练替换权重
- **告警方式**：红蓝 LED 交替快闪 + OLED 全屏 "FALL DETECTED!" 提示

### 姿态传感器与算法

- **Kalman 滤波** 姿态解算（Roll/Pitch + 陀螺仪零偏估计，KasBot V2 实现）
- **计步算法** 基于加速度幅值差分与时间窗口的峰值检测
- **抬腕唤醒** 检测特定姿态角度 + 上升沿触发

### 显示页面

- **时钟** — 模拟指针 + 数字时间，日期显示
- **菜单** — 应用启动器网格界面
- **秒表** — 开始/暂停/复位
- **LED 控制** — 红蓝 LED 开关
- **姿态显示** — 实时 Roll/Pitch/Yaw 数值 + 可视化
- **小恐龙游戏** — 跳跃躲避障碍物，碰撞检测
- **表情动画** — 动态表情渲染
- **水平仪** — 气泡水平仪
- **设置** — 时间设定、配置项
- **计步器** — 当日/累计步数

### 电源管理

- 页面相关的动态采样率调整（5~1000 ms）
- OLED 自动调暗（30 秒无操作）/ 自动关闭（180 秒无操作）
- 空闲时进入 WFI 低功耗模式

### OTA 无线升级

- ESP8266 WiFi 从 HTTP 服务器下载固件
- 双分区 A/B 升级（App_A 192 KB + App_B 256 KB）
- CRC32 硬件校验保证固件完整性
- AT24C02 EEPROM 存储引导标志，升级失败自动回退

### 安全引导 (Bootloader)

- 独立引导程序（64 KB），上电检测 OTA 标志 → CRC 校验 → 双分区切换 → 跳转应用

## 跌倒检测神经网络架构

```
输入层 (16特征: 均值/标准差/峰值/SMA/过零率/能量/姿态方差/轴向均值)
   ↓ W1[16×16] + b1, ReLU
隐藏层1 (16神经元)
   ↓ W2[8×16] + b2, ReLU
隐藏层2 (8神经元)
   ↓ W3[1×8] + b3, Sigmoid
输出 → 跌倒概率 [0, 1]
```

- 总参数: 417 floats (1.7 KB), 存放于 Flash .rodata
- 浮点推理, Cortex-M4F FPU 硬件加速
- 特征标准化 (均值/标准差归一化) 集成在推理管线中
- 权重可通过 `generate_weights.py` + 真实标注数据重新训练

## 开发环境

- **IDE**：Keil MDK-ARM (μVision)
- **工具链**：ARMCC (ARM Compiler 5)
- **RTOS**：FreeRTOS V11.1.0
- **标准库**：STM32F4xx Standard Peripheral Library

## Flash 分区布局

```
0x08000000 +----------------+  64 KB
           |  Bootloader    |
0x08010000 +----------------+ 192 KB
           |  App_A (默认)   |
0x08040000 +----------------+ 256 KB
           |  App_B         |
0x08080000 +----------------+
```

## 项目结构

```
SmartWatch_F407/
├── App/            应用层 (任务、UI、数据模型)
├── Bootloader/     引导程序 (独立 Keil 工程)
├── Drivers/        驱动层 (CMSIS、标准外设库、BSP)
├── FreeRTOS/       FreeRTOS 内核源码
├── Middlewares/    中间件 (Kalman、计步、NN 推理、跌倒检测)
├── Project/        Keil 工程文件
└── User/           用户入口 (main.c、中断服务)
```
