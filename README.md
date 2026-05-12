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

## 操作说明

| 按键 | 功能 |
|------|------|
| A / D 或 ← / → | 左右移动 |
| W / ↑ 或 Space | 跳跃（支持二段跳 / 贴墙跳） |
| J | 近战攻击 |
| R | 死亡 / 胜利后重新开始 |
| Q | 调试：缩放脉冲 |
| E | 调试：摄像机抖动 |
