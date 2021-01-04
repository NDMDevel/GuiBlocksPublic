#ifndef GUIBLOCK_LINK_H
#define GUIBLOCK_LINK_H

#include <memory>
#include <QVector>

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include "Block.h"

namespace GuiBlocks {

class Link : public QGraphicsItem
{
private:
    QPainterPath path;
    QPointF start,end;
    std::weak_ptr<Block::Port> inPort;
    QVector<std::weak_ptr<Block::Port>> outPorts;
public:
    Link();
    void setStartPoint(const QPointF &startPos){ start = startPos; }
    void setEndPoint(const QPointF &endPos){ end = endPos; update(); }
    void setInPort(std::shared_ptr<Block::Port> port);
    void addOutPort(std::shared_ptr<Block::Port> port);
    void removeInPort(std::shared_ptr<Block::Port> port);
    void removeOutPort(std::shared_ptr<Block::Port> port);

    const QPainterPath& getPath(){ return path; }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QRectF boundingRect() const override { return path.boundingRect(); }

};

} // namespace GuiBlock

#endif // GUIBLOCK_LINK_H
