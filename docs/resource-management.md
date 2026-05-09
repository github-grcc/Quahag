# Quahag 资源管理架构与流程

## 概述

Quahag 的资源管理涉及两层所有权：**GameWorld** 通过 `unique_ptr` 拥有所有实体，**QGraphicsScene** 通过 `addItem()` 持有实体的图形引用。两层之间通过 Qt 信号-槽机制保持同步，确保实体在删除前先从场景中移除，避免悬挂指针和双重释放。

## 核心数据结构

### GameWorld — 实体所有者

```
GameWorld
├── m_entities          vector<unique_ptr<ActorItem>>   ← 实体唯一所有权
├── m_pendingSpawn      vector<unique_ptr<ActorItem>>   ← 待刷新的创建队列
├── m_pendingDestroy    QVector<ActorItem*>             ← 待刷新的销毁队列（裸指针）
├── m_entitiesByKind    QHash<EntityKind, QVector<ActorItem*>>   ← 按类型索引
├── m_entitiesByFaction QHash<Faction, QVector<ActorItem*>>      ← 按阵营索引
├── m_player            QPointer<Player>                ← 玩家快捷引用（指针保护）
└── m_tileMap           TileMap                         ← 瓦片地图数据
```

### GameScene — 图形场景桥接

```
GameScene : QGraphicsScene
├── m_world       QPointer<GameWorld>   ← 世界引用（指针保护）
├── m_tileLayer   QGraphicsItem*        ← 瓦片图层（场景内裸指针，由 QGraphicsScene::clear() 删除）
└── [items]       QGraphicsItem*        ← 场景内所有图形项（实体 + 瓦片图层）
```

### 信号连接 (GameWorld → GameScene)

```
GameWorld::entitySpawned              →  GameScene::addEntityItem      (addItem)
GameWorld::entityAboutToBeDestroyed   →  GameScene::removeEntityItem   (removeItem)
```

## 所有权模型

### 实体：双重注册，单一所有权

每个实体同时存在于两个系统中：

| 系统 | 注册方式 | 所有权 | 删除行为 |
|------|---------|--------|---------|
| GameWorld | `m_entities` 中的 `unique_ptr` | **唯一所有权** | `delete` 实体 |
| QGraphicsScene | `addItem(entity)` | 引用（无所有权） | `removeItem(entity)` 仅移除，不删除 |

实体被删除时，必须先调用 `removeItem` 从场景中移除，再由 `unique_ptr` 销毁实体。`QGraphicsItem::~QGraphicsItem()` 会在析构时自动从场景移除自身——这是兜底安全网，但正常流程应当在析构前主动移除。

### 瓦片图层：场景单一所有权

`m_tileLayer` 是裸指针，由 `QGraphicsScene` 通过 `addItem()` 持有并负责删除。`clear()` 或 `~QGraphicsScene()` 会删除它。

---

## 实体生命周期

### 1. 创建（createEntity → flushSpawns）

```
createEntity<T>(args...)
  │
  ├─ make_unique<T>(args...)          // 分配实体
  ├─ m_pendingSpawn.push_back(...)    // 加入待刷新队列
  └─ return raw pointer               // 返回裸指针供调用方配置
```

调用方拿到裸指针后设定位置、Z 值等。实体此时**尚未**进入 `m_entities`，也**未**添加到场景。

```
flushSpawns()
  │
  ├─ for each ptr in m_pendingSpawn:
  │    ├─ raw = ptr.get()
  │    ├─ m_entities.push_back(move(ptr))   // 所有权转移至主容器
  │    ├─ indexEntity(raw)                  // 按 kind/faction 索引
  │    └─ emit entitySpawned(raw)           // → GameScene::addEntityItem → addItem
  │
  └─ m_pendingSpawn.clear()
```

**关键点**：`flushSpawns()` 是 `public` 的，`resetGame()` 直接调用它而无需通过完整的 `step()`。

### 2. 运行时逐个销毁（destroyLater → flushDestroys）

```
destroyLater(entity)
  │
  ├─ entity->markPendingDestroy()      // 标记待销毁（tick 会跳过它）
  └─ m_pendingDestroy.append(entity)   // 加入待刷新队列
```

```
flushDestroys()
  │
  ├─ for each entity in toDestroy:
  │    ├─ emit entityAboutToBeDestroyed(entity)  // → GameScene::removeEntityItem → removeItem
  │    ├─ unindexEntity(entity)                  // 从 kind/faction 索引中移除
  │    └─ m_entities.erase(it)                  // unique_ptr 出作用域 → delete entity
  │
  └─ m_entitiesDirty = true (已移除缓存机制，此标记不再存在)
```

**注意**：`flushDestroys()` 会调用 `unindexEntity()`，逐个从 `m_entitiesByKind`/`m_entitiesByFaction` 中移除实体引用，并检查是否需要清除 `m_player`。这是运行时逐个销毁的必经路径。

### 3. 批量清空（clearAllEntities）

用于游戏重置（`resetGame()` 中按下 R 键时调用）。

```
clearAllEntities()
  │
  ├─ m_pendingSpawn.clear()            // 销毁待刷新队列中的实体
  ├─ m_pendingDestroy.clear()          // 清空待销毁列表（裸指针，无 delete）
  │
  ├─ for each ptr in m_entities:
  │    └─ emit entityAboutToBeDestroyed(ptr.get())  // → GameScene::removeEntityItem → removeItem
  │
  ├─ m_entitiesByKind.clear()          // 索引全量清空
  ├─ m_entitiesByFaction.clear()
  ├─ m_player.clear()
  └─ m_entities.clear()                // 所有 unique_ptr 销毁 → delete 所有实体
```

**设计要点**：
- **不调用 `unindexEntity()`**：因为索引在循环后直接全量 `clear()`，逐个移除是浪费。
- **保留信号发射**：每个实体通过 `entityAboutToBeDestroyed` 信号通知 `GameScene` 调用 `removeItem()`。这确保实体在被 `unique_ptr` 删除前先从场景中移除。
- **与 `flushDestroys()` 的区别**：`flushDestroys()` 需要精确地从 `m_entities` 中查找并删除单个实体（`find_if + erase`），同时逐项维护索引；`clearAllEntities()` 直接全量清空所有容器。

### 4. 程序退出（~GameWorld → ~QGraphicsScene）

这是最特殊的所有权转移路径。

**Qt 子对象析构顺序（逆构造序）**：

```
GameView 构造:
  m_scene(new GameScene(this))   // 子对象 1（最先创建）
  m_loop(this)                    // 子对象 2
  new GameWorld(this)             // 子对象 3（最后创建）

GameView 析构时 Qt deleteChildren() 顺序:
  1. ~GameWorld()   （子对象 3，最先析构）
  2. ~GameLoop()    （子对象 2）
  3. ~GameScene()   （子对象 1，最后析构）
     └─ ~QGraphicsScene() → clear()  ← 删除场景中所有项！
```

**`~QGraphicsScene()` 必定调用 `clear()`**。如果此时实体仍被 `unique_ptr` 持有且同时存在于场景中，`clear()` 和 `unique_ptr` 会双重释放同一实体。

**解决方案：`~GameWorld()` 释放所有权**：

```cpp
GameWorld::~GameWorld()
{
    for (auto &ptr : m_entities)
        (void)ptr.release();       // 放弃所有权，不 delete
    for (auto &ptr : m_pendingSpawn)
        (void)ptr.release();
    // m_entities 和 m_pendingSpawn 中的 unique_ptr 现在是空的
}
```

析构流程变为：

```
1. ~GameWorld()
   └─ release() 所有 unique_ptr  →  实体不再被 GameWorld 拥有
                                    →  实体仍存在于场景中（未移除、未删除）

2. ~GameLoop()
   └─ 定时器停止

3. ~GameScene()
   └─ ~QGraphicsScene::clear()
        ├─ removeItem(entity) + delete entity   ← 每个实体删除一次 ✓
        └─ removeItem(tileLayer) + delete tileLayer  ← 瓦片图层删除一次 ✓
```

**为什么不用信号**：`~GameWorld()` 执行时 Qt 子对象析构已在进行中，信号-槽机制可能不可靠。释放所有权交给 `~QGraphicsScene::clear()` 统一处理更安全。

---

## 场景管理

### attachWorld — 初次绑定

```
GameView 构造:
  auto *world = new GameWorld(this);
  builder.build(*world);         // 实体进入 m_pendingSpawn
  m_scene->attachWorld(world);   // 绑定场景
    ├─ rebuildScene()            // 创建瓦片图层
    │    ├─ clear()              // 清空场景（初次为空）
    │    ├─ new TileLayerItem + addItem
    │    └─ for entity in world->entities() → addEntityItem (初次为空)
    └─ connect(信号)
  world->step(initCtx);          // flushSpawns → emit entitySpawned → addItem
```

### rebuildScene — 重建场景

用于 `resetGame()` 中清除旧瓦片图层并创建新的。

```
rebuildScene()
  ├─ clear()                              // 删除场景中所有项（此时只有瓦片图层）
  ├─ new TileLayerItem + addItem          // 重建瓦片图层
  └─ for entity in world->entities()      // 重新添加实体（clearAllEntities 后为空）
       └─ addEntityItem(entity)
```

**前置条件**：调用 `rebuildScene()` 前必须先调用 `clearAllEntities()`，确保实体已在场景外。`clearAllEntities()` 的信号已将实体从场景移除，`clear()` 只删除瓦片图层。

---

## 游戏流程集成

### resetGame() 完整序列

```
GameView::resetGame()
  │
  ├─ m_camera.stopZoomPulse()           // 停止相机效果
  ├─ m_shakeRequested = false
  ├─ m_zoomPulseRequested = false
  │
  ├─ world->clearAllEntities()          // 步骤 1: 信号移除实体 → 批量删除
  ├─ m_scene->rebuildScene()            // 步骤 2: clear() 删瓦片图层 → 重建瓦片图层
  ├─ LevelBuilder().build(*world)       // 步骤 3: 创建新实体 → m_pendingSpawn
  ├─ world->flushSpawns()               // 步骤 4: 刷新生成 → m_entities → emit → addItem
  │
  ├─ m_camera.setTargetZoom(1.0)        // 相机重置为 1x
  ├─ m_camera.snapToTarget()            // 立即生效（无动画）
  ├─ m_input = InputState{}             // 清除输入状态
  └─ m_gameState = GameState::Playing   // 进入游戏状态
```

**为什么用 `flushSpawns()` 而不是 `step()`**：`step()` 除了刷新生成队列外还会 tick 全部实体。对于重置场景，只需要把新生成的实体搬入 `m_entities` 并添加到场景，不需要多余的 tick。

### 世界暂停（Title 状态）

```
Title 状态: m_loop.setWorldPaused(true)
  │
  └─ GameLoop::tick() 仍运行:
       ├─ dt 测量 ✓
       ├─ m_world->step(ctx) ← 跳过 ✓
       ├─ emit stepped(dt)   ← 发出（相机更新依赖此信号）
       └─ emit 相机效果信号  ← 发出
```

世界暂停时实体不运动，但相机持续跟踪玩家位置，叠加层文字持续渲染。

---

## 所有权流转汇总

```
                    创建                        运行时销毁                     批量清空                        程序退出
                    ────                        ────────                     ────────                        ────────

实体分配      make_unique<T>()              destroyLater()              clearAllEntities()               ~GameWorld()
                  │                              │                            │                              │
                  ▼                              ▼                            ▼                              ▼
待处理队列    m_pendingSpawn                m_pendingDestroy            （不经过队列）                  release()
                  │                              │                            │                              │
                  ▼                              ▼                            ▼                              ▼
活跃集合      m_entities                    m_entities                   m_entities                     m_entities
             (unique_ptr 所有)             (unique_ptr 所有)            (unique_ptr 所有)              (已 release)
                  │                              │                            │                              │
                  ▼                              ▼                            ▼                              ▼
场景注册      emit entitySpawned            emit entityAboutToBe         emit entityAboutToBe           （不经过信号）
                  │    Destroyed                  │    Destroyed                  │
                  ▼                              ▼                            ▼                              ▼
             addItem(entity)               removeItem(entity)           removeItem(entity)              ~QGraphicsScene
                                                                                                       ::clear()
                                                                                                         │
                                                                                                         ▼
                                                                                                    delete entity
                                                                                                    delete tileLayer
```

---

## 设计原则

1. **唯一所有权**：实体由 `unique_ptr` 唯一拥有，场景只是图形引用的注册表。
2. **先移除再删除**：实体删除前必须从场景移除，防止 `QGraphicsScene` 持有悬挂指针。
3. **延迟操作**：创建/销毁不立即生效，通过队列积累后统一刷新，避免迭代器失效。
4. **信号驱动同步**：GameWorld 不直接访问 GameScene，通过 Qt 信号解耦两层。
5. **批量优化**：`clearAllEntities()` 跳过逐项 unindex，直接全量清空索引。
6. **退出安全**：`~GameWorld()` 释放所有权，让 `~QGraphicsScene::clear()` 统一删除，避免与 Qt 内部清理冲突。
