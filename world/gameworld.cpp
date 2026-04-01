#include"world/gameworld.h"
#include"entities/player.h"
GameWorld::GameWorld(QObject *parent){

}
Player *GameWorld::player()const{
    return m_player;
}