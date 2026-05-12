# 圆哈镇3022（Quahag）

本项目是基于C++语言和Qt图形框架开发的一款名为"圆哈镇3022（Quahag）"的桌面端2D平台跳跃/动作游戏。故事设定在3022年——玩家操控外出归来的"耄耋"返回故乡"圆哈镇"，却发现家园已被名为"胖宝宝"的敌人占领。为了夺回属于自己的王座，耄耋穿行于复杂的博古架地形之间，与巡逻的胖宝宝周旋战斗。途中散落着"哈基米特饮"（短暂全属性加速）和"口香糖"（恢复生命值）等道具，合理利用它们是通关的关键。

## 构建与安装（Debian / Linux）

```sh
# 安装依赖
sudo apt install build-essential cmake ninja-build qt6-base-dev libgl1-mesa-dev

# 配置（Qt6, Release）
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6

# 构建
cmake --build build

# 运行
./build/Quahag
```

### ASAN 调试构建

```sh
cmake -B build-asan -G Ninja -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6 -DENABLE_ASAN=ON
cmake --build build-asan
./build-asan/Quahag
```

## 为 Windows 构建与分发

### 方式一：Debian 交叉编译（MinGW-w64）

```sh
# 安装 MinGW 工具链
sudo apt install mingw-w64 cmake ninja-build

# 下载 Qt 6.10.2 Windows (MinGW 64-bit) 离线安装程序
# 从 https://download.qt.io/archive/qt/ 下载并安装到 ~/Qt/6.10.2/mingw_64/
# 同时安装同版本的 Linux (gcc_64) 套件到 ~/Qt/6.10.2/gcc_64/（QT_HOST_PATH 使用）

# 构建
./scripts/build-win64.sh

# 部署（打包 Qt DLL）
./scripts/deploy-win64.sh

# 输出：deploy/win64/Quahag.exe + Qt DLL，可直接拷贝到 Windows 运行
```

也可通过环境变量自定义 Qt 路径：

```sh
QT_MINGW_PATH=/opt/qt/6.5.0/mingw_64 QT_HOST_PATH=/opt/qt/6.5.0/gcc_64 ./scripts/build-win64.sh
```

> 音效：Windows 上使用独立线程 + `PlaySoundW` 播放。如需更低延迟的 `QSoundEffect`，通过 Qt Maintenance Tool 为 MinGW 套件安装 **Qt Multimedia** 模块，CMake 会自动启用。

### 方式二：Windows 原生构建

**前置条件：**
- Qt 6.x（MSVC 或 MinGW 套件）
- CMake ≥ 3.16
- Visual Studio 2022（MSVC）或 MinGW-w64

**MSVC（Visual Studio 开发者命令提示符）：**

```cmd
cmake -B build -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH=C:/Qt/6.10.2/msvc2022_64
cmake --build build --config Release
```

**MinGW-w64：**

```cmd
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH=C:/Qt/6.10.2/mingw_64
cmake --build build
```

**部署：**

```cmd
windeployqt build/Quahag.exe
```

## 操作说明

| 按键 | 功能 |
|------|------|
| A / D 或 ← / → | 左右移动 |
| W / ↑ 或 Space | 跳跃（支持二段跳 / 贴墙跳） |
| J | 近战攻击 |
| R | 死亡 / 胜利后重新开始 |
| Q | 调试：缩放脉冲 |
| E | 调试：摄像机抖动 |
