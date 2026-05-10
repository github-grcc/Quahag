# Quahag 资源管理架构与流程

## 概述

Quahag 的资源管理涉及两层所有权：**GameWorld** 通过 `unique_ptr` 拥有所有实体，**QGraphicsScene** 通过 `addItem()` 持有实体的图形引用。两层之间通过 Qt 信号-槽机制保持同步。

关键设计决策：**不在删除实体前主动调用 `QGraphicsScene::removeItem()`**，而是将实体设为不可见，由 Qt 的 `~QGraphicsItem()` 析构链统一处理场景分离。这一策略基于 Qt 6.10 的一个实际问题——`removeItem()` 内部可能存在异步索引清理，导致立即删除实体时渲染管线仍持有悬挂引用，引发 SIGSEGV 或 "pure virtual method called"。

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
GameWorld::entityAboutToBeDestroyed   →  GameScene::removeEntityItem   (hide, 不 removeItem)
```

---

## 所有权模型

### 实体：双重注册，单一所有权

每个实体同时存在于两个系统中：

| 系统 | 注册方式 | 所有权 | 删除行为 |
|------|---------|--------|---------|
| GameWorld | `m_entities` 中的 `unique_ptr` | **唯一所有权** | `delete` 实体 |
| QGraphicsScene | `addItem(entity)` | 引用（无所有权） | 仅隐式引用，不在外部调用 `removeItem` |

### 为什么不在删除前主动 removeItem

在 Qt 6.10 中，`removeItem()` 调用后 Qt 内部可能尚未完成索引清理（场景变换缓存、空间索引等异步更新）。如果在 `removeItem()` 后立即 `delete` 实体或在下个事件循环中 `delete`，Qt 的渲染管线（`drawSubtreeRecursive`）可能仍持有该实体的悬挂引用，导致：

- **SIGSEGV**：访问已释放内存（ASAN 0xFD 填充 → 垃圾变换矩阵值）
- **pure virtual method called**：虚表指针部分有效但指向基类，`paint()` / `boundingRect()` 成为纯虚调用

**解决方案**：删除前将实体设为不可见（`setVisible(false)` + `setEnabled(false)`），让 `~QGraphicsItem()` 在析构时内部完成场景分离。Qt 的析构链保证了内部索引和场景引用的原子性清理。

### removeEntityItem 实现

```cpp
void GameScene::removeEntityItem(ActorItem *entity)
{
    if (!entity)
        return;
    // 只隐藏，不 removeItem()。渲染遍历中 isVisible() 检查会跳过
    // 隐藏项。真正的场景移除由 ~QGraphicsItem() 在 delete 时完成。
    entity->setVisible(false);
    entity->setEnabled(false);
}
```

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
  │    ├─ emit entityAboutToBeDestroyed(entity)  // → setVisible(false), setEnabled(false)
  │    ├─ unindexEntity(entity)                  // 从 kind/faction 索引中移除
  │    ├─ it->release()                          // 放弃 unique_ptr 所有权
  │    ├─ m_entities.erase(it)                   // 从主容器移除
  │    └─ QTimer::singleShot(0, [raw] { delete raw; })
  │              // 延迟到下个事件循环迭代再 delete
  │              // 确保本轮的 paint event 先处理完毕
  │              // delete 时 ~QGraphicsItem() 完成 scene 分离
  │
  └─ m_pendingDestroy.clear()
```

**设计要点**：

- **不调用 removeItem()**：只将实体设为不可见。Qt 渲染遍历 (`drawSubtreeRecursive`) 检查 `isVisible()`，隐藏项被跳过。
- **QTimer::singleShot(0)**：将 `delete` 延迟到下一个事件循环迭代。`deleteLater()` 的 `DeferredDelete` 事件在本轮 paint 之前处理，时机不满足要求；零定时器确保本轮的 paint event 先处理完毕。
- **release + erase**：放弃 `unique_ptr` 所有权后从 `m_entities` 移除。实体在短暂窗口内存在于场景中（不可见）但不被任何系统拥有，直到定时器触发 delete。
- **内存占用**：每帧处于"隐藏待删"窗口的实体最多几十个（几个敌人 + 几十个粒子），每个约几百字节，总内存占用 < 100KB，下一帧即被释放。

### 3. 批量清空（clearAllEntities）

用于游戏重置（`resetGame()` 中按下 R 键时调用）。

```
clearAllEntities()
  │
  ├─ m_pendingSpawn.clear()            // 销毁待刷新队列中的实体
  ├─ m_pendingDestroy.clear()          // 清空待销毁列表（裸指针，无 delete）
  │
  ├─ for each ptr in m_entities:
  │    └─ emit entityAboutToBeDestroyed(ptr.get())  // → setVisible(false), setEnabled(false)
  │
  ├─ m_entitiesByKind.clear()          // 索引全量清空
  ├─ m_entitiesByFaction.clear()
  ├─ m_player.clear()
  │
  ├─ for each ptr in m_entities:
  │    └─ ptr.release()                // 放弃所有权，不 delete
  │
  └─ m_entities.clear()
```

实体在 `clearAllEntities()` 后变为不可见（`setVisible(false)`）但仍在场景中。所有权已通过 `release()` 放弃。

随后 `resetGame()` 调用 `rebuildScene()` → `QGraphicsScene::clear()`，会删除场景中所有项（包括被隐藏的旧实体和瓦片图层），由 Qt 的析构链统一处理场景分离和内存释放。不会发生双重释放，因为实体已不在 `unique_ptr` 中。

**设计要点**：

- **不调用 `unindexEntity()`**：索引在循环后直接全量 `clear()`，逐个移除是浪费。
- **不调用 `removeItem()`**：与 `flushDestroys()` 一致，只隐藏实体。
- **不独立 delete**：利用后续 `rebuildScene()` → `clear()` 统一删除，避免与 Qt 内部索引竞争。
- **与 `flushDestroys()` 的区别**：`flushDestroys()` 使用 `QTimer::singleShot(0)` 逐个延迟删除；`clearAllEntities()` 依赖 `clear()` 的批量删除，不需要独立定时器。

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

用于 `resetGame()` 中清除旧瓦片图层并创建新的。同时负责删除 `clearAllEntities()` 中隐藏的旧实体。

```
rebuildScene()
  ├─ clear()                              // 删除场景中所有项（旧实体已隐藏 + 瓦片图层）
  ├─ new TileLayerItem + addItem          // 重建瓦片图层
  └─ for entity in world->entities()      // 重新添加实体（clearAllEntities 后为空）
       └─ addEntityItem(entity)
```

**前置条件**：调用 `rebuildScene()` 前必须先调用 `clearAllEntities()`，将旧实体隐藏并放弃所有权。`clear()` 统一删除所有场景项，不会出现双重释放。

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
  ├─ world->clearAllEntities()          // 步骤 1: 隐藏实体 → 批量释放所有权
  ├─ m_scene->rebuildScene()            // 步骤 2: clear() 删旧项 → 重建瓦片图层
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
             addItem(entity)               setVisible(false)            setVisible(false)              ~QGraphicsScene
                                           setEnabled(false)            setEnabled(false)              ::clear()
                                                  │                            │                              │
                                                  ▼                            ▼                              ▼
                                           QTimer::singleShot(0)        rebuildScene()                 delete entity
                                           → delete raw                 → clear()                      delete tileLayer
                                           → ~QGraphicsItem()           → delete all items
                                           → 自动 removeItem
```

---

## 设计原则

1. **唯一所有权**：实体由 `unique_ptr` 唯一拥有，场景只是图形引用的注册表。
2. **隐含场景移除**：不在外部显式调用 `removeItem()`，通过 `setVisible(false)` 让渲染跳过实体，由 `~QGraphicsItem()` 在析构时统一处理场景分离。这一策略避免了 Qt 6.10 中 `removeItem()` 异步索引清理与立即 delete 之间的竞争条件。
3. **延迟删除**：运行时销毁使用 `QTimer::singleShot(0)` 将 delete 推迟到下个事件循环迭代，确保 Qt 的渲染事件先处理完毕。重置时依赖 `rebuildScene()` 中的 `clear()` 批量删除。
4. **延迟创建**：创建不立即生效，通过队列积累后统一刷新，避免迭代器失效。
5. **信号驱动同步**：GameWorld 不直接访问 GameScene，通过 Qt 信号解耦两层。
6. **批量优化**：`clearAllEntities()` 跳过逐项 unindex，直接全量清空索引。
7. **退出安全**：`~GameWorld()` 释放所有权，让 `~QGraphicsScene::clear()` 统一删除，避免与 Qt 内部清理冲突。
