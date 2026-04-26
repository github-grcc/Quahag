#pragma once

namespace ZLayer {
    constexpr double Background = 0.0;
    constexpr double Particles  = 3.0;
    constexpr double Bullets    = 4.0;
    constexpr double Player     = 5.0;
    constexpr double Enemies    = 6.0;
    constexpr double Effects    = 7.0;
}

enum class EntityKind{
    Player,
    Enemy,
    Bullet,
    Effect
};
enum class Faction{
    Player,
    Enemy,
    Neutral
};