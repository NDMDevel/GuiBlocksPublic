#ifndef TYPEID_H
#define TYPEID_H

#include <QGraphicsItem>

namespace GuiBlocks
{

enum TypeID
{
    BlockID = QGraphicsItem::UserType + 1,
    LinkID
};

} // namespace GuiBlocks

#endif // TYPEID_H
