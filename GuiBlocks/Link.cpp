#include "Link.h"

#include <QDebug>
#include <QPainter>

namespace GuiBlocks {

Link::Link()
{
//    path.connectPath()
}

void Link::setInPort(std::shared_ptr<Block::Port> port)
{
    (void)port;
}

void Link::addOutPort(std::shared_ptr<Block::Port> port)
{
    (void)port;
}

void Link::removeInPort(std::shared_ptr<Block::Port> port)
{
    (void)port;
}

void Link::removeOutPort(std::shared_ptr<Block::Port> port)
{
    (void)port;
}

void Link::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    (void)event;
    //path.lineTo(mapFromScene(event->pos()));
    qDebug() << "mouseMoveEvent()";
}

void Link::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    (void)option;
    (void)widget;
    qDebug() << "Link::paint()";
    painter->drawLine(start,end);
}

} // namespace GuiBlock
