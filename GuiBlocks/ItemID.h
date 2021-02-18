#ifndef ITEMID_H
#define ITEMID_H

#include <QGraphicsItem>

namespace GuiBlocks
{
enum class TypeID
{
    Block = QGraphicsItem::UserType,
    Link,
};
}

#endif // ITEMID_H
