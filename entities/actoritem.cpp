#include "entities/actoritem.h"

#include <QDebug>

ActorItem::ActorItem(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
}

ActorItem::~ActorItem()
{
    // If the item is still in a scene when destroyed, Qt will crash later
    // when it tries to render this now-deleted item.
    if (scene()) {
        qCritical() << "BUG: ActorItem deleted while still in scene!"
                     << "addr:" << (void*)this
                     << "scene:" << scene();
    }
}

QVariant ActorItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneHasChanged) {
        // Use metaObject()->className() instead of kind() because
        // itemChange can be called during ~QGraphicsItem after the
        // derived class has been destroyed, making kind() pure virtual.
        qDebug() << "ActorItem scene change:"
                 << "addr:" << (void*)this
                 << "type:" << metaObject()->className()
                 << "hasScene:" << (value.toBool());
    }
    return QGraphicsObject::itemChange(change, value);
}
 