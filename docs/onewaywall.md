# 单向可通过墙壁（One-Way Wall）

## 概述

单向墙壁是一种特殊的 tile，ActorItem（bullet、enemy、player）只能从指定方向穿过。穿过后墙壁消失，所有实体及视线均可自由通过，0.5 秒内无实体重叠则恢复。

## 地图格式

在 `rsc/map3.txt` 中使用以下字符表示：

| 字符 | TileType 值 | 可通过方向 | 方向标识 |
|------|-------------|-----------|----------|
| `r`  | 6 (OneWayRight) | 向右 ↑ | `>>` |
| `l`  | 7 (OneWayLeft)  | 向左 ← | `<<` |
| `u`  | 4 (OneWayUp)    | 向上 ↑ | `^`/`^` |
| `d`  | 5 (OneWayDown)  | 向下 ↓ | `v`/`v` |

地图解析在 `TileMap::initTiles()` (`world/tilemap.cpp:99`) 中完成，非数字字符通过硬编码映射表转换为对应 tile 值。

## 运行机制

### 状态模型

每个单向墙 tile 在运行时处于两种状态之一：

- **关闭（closed）**：表现为普通实心 tile，阻挡所有方向的碰撞、移动和视线
- **开启（open）**：完全透明，无碰撞、无视线阻挡、不可见

状态存储在 `TileMap::m_openWalls`（`QHash<QPoint, qreal>`），key 为 `(col, row)`，value 为剩余开启时间。

### 开启条件

当 ActorItem 的包围盒与墙壁 tile 重叠**且**实体中心位于墙壁的正确一侧**且**速度方向匹配可通过方向时，墙壁开启。

具体判断逻辑在 `TileMap::tryPassOneWayWall()` (`world/tilemap.cpp:166`)：

| 墙壁类型 | 实体中心位置 | 速度方向 |
|----------|-------------|---------|
| OneWayRight | 实体在墙左侧 (`entityCenter.x < tileCenter.x`) | 向右 (`velX > 0`) |
| OneWayLeft | 实体在墙右侧 (`entityCenter.x > tileCenter.x`) | 向左 (`velX < 0`) |
| OneWayUp | 实体在墙下方 (`entityCenter.y > tileCenter.y`) | 向上 (`velY < 0`) |
| OneWayDown | 实体在墙上方 (`entityCenter.y < tileCenter.y`) | 向下 (`velY > 0`) |

同时检查位置和方向确保 X resolver 不会误触发 u/d 墙，Y resolver 不会误触发 l/r 墙。

### 关闭条件

每帧末 `GameWorld::step()` 调用 `TileMap::updateOneWayWalls()` (`world/tilemap.cpp:206`)：

1. 遍历所有开启的墙壁
2. 若有任何实体包围盒与墙壁 tile 重叠 → 重置计时器到 0.5 秒
3. 若无重叠 → 计时器递减 dt
4. 计时器 ≤ 0 → 关闭墙壁（从 `m_openWalls` 移除）

### 碰撞解析集成

碰撞解析流程（以 r-wall 为例）：

```
Frame N:
  Player 向右移动 → resolveTileCollisionsX()
    → solidTilesOverlapping() 发现 r-wall（isSolidTile 返回 true，壁关闭）
    → tryPassOneWayWall() 检查：entityCenter 在左侧 ✓, velX > 0 ✓
    → openWall() 标记为开启，跳过 push-out
    → Player 继续向右移动穿过 tile
  resolveTileCollisionsY() 调用时 isSolidTile 已返回 false（壁开了）
Frame N 末:
  updateOneWayWalls(): 墙壁开启，Player 仍重叠 → 重置计时器 0.5s
Frame N+1 末:
  Player 已离开 → 无重叠 → 计时器递减
...
Frame N+K 末:
  计时器 ≤ 0 → 墙壁关闭
```

## 受影响代码路径

| 路径 | 文件 | 行为 |
|------|------|------|
| Player X/Y 碰撞 | `entities/player.cpp` | `tryPassOneWayWall` 开启墙壁，跳过推挤 |
| Enemy X/Y 碰撞 | `entities/enemy.cpp` | 同上 |
| Bullet 碰撞 | `entities/bullet.cpp` | 同上；开启则子弹通过，否则销毁 |
| Enemy 视线 | `entities/enemy.cpp:hasLineOfSight` | 通过 `isSolidTile()`，开墙可透视，闭墙阻挡 |
| Enemy 巡逻检测 | `entities/enemy.cpp:hasGroundAhead/hasWallAhead` | 同上 |
| Player 墙滑 | `entities/player.cpp:detectWallSide` | 跳过单向墙 tile，不做 wall-sliding |
| 渲染 | `graphics/tilelayeritem.cpp` | 闭墙：褐色填充 + 方向标识；开墙：不绘制 |

## 渲染

在 `TileLayerItem::paint()` (`graphics/tilelayeritem.cpp:90`) 中：

- 底色：`QColor(100, 65, 30)`（深褐色）
- 文字色：`QColor(220, 180, 100)`（浅褐色）
- 字体：10px 粗体
- 开墙：跳过，不绘制

## 核心文件

| 文件 | 职责 |
|------|------|
| `world/tilemap.h` | TileType 枚举、单向墙 API 声明、m_openWalls 成员 |
| `world/tilemap.cpp` | 地图解析、isSolidTile 逻辑、tryPassOneWayWall、计时器管理 |
| `entities/player.cpp` | 碰撞解析集成 |
| `entities/enemy.cpp` | 碰撞解析 + 视线集成 |
| `entities/bullet.cpp` | 子弹碰撞集成 |
| `world/gameworld.cpp` | step() 末尾调用 updateOneWayWalls |
| `graphics/tilelayeritem.cpp` | 渲染 |
