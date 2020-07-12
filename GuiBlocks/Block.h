#ifndef GUIBLOCKS_BLOCK_H
#define GUIBLOCKS_BLOCK_H

#include <QGraphicsItem>
#include <QVector>
#include <cstdint>
#include "GuiBlocks/Style.h"
#include <QDebug>
#include <QFontMetrics>

namespace GuiBlocks {

class Block : public QGraphicsItem
{
public:
    enum class PortDir
    {
        Input,
        Output
    };
    struct Port
    {
        PortDir  dir;
        QString name;
        QString type;
        bool connected;
        bool multipleConnections;
        QPainterPath connectorShape;
        Port(){}
        Port(PortDir dir,QString name,QString type);
    };
    enum class BlockOrientation
    {
        East,   //left  to right
        West    //right to left
    };
private:
    //ctor required
    QString type;
    QString name;
    QFontMetrics fontMetrics;
private:
    //internals
    QRectF blockRect;
    QRectF dragArea;
    QVector<Port> ports;
    QPainterPath blockShapePath;
    BlockOrientation dir;
    float &gridSize;
    int nInputs;
    int nOutputs;
    QPointF center;
    BlockOrientation blockOrientation;
    int portHintToDraw;
public:
    Block(const QString &type,
          const QString &name,
          //QFontMetrics  &fontMetrics,       //needed for determine the size of texts
          float &gridSize,
          QGraphicsItem *parent = nullptr);

    void addPort(PortDir dir,QString type,QString name = "");
    void setBlockOrientation(const BlockOrientation &orientation);
    void toggleBlockOrientation();
    //void setCentralPosition(const QPointF &centerPos);    //should be this implemented?

    void toggleConnectionPortState(int &indexPort);    //remove this, just for debug

    //Interface methods:
    QRectF boundingRect() const override{ return blockRect; }
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
//    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    void updateBoundingRect();
    void drawBoundingRect(QPainter *painter);
    void drawType(QPainter *painter);
    void drawName(QPainter *painter);
    void drawPortHint(QPainter *painter,const Port &port);
    void drawBlockShape(QPainter *painter);
    void drawConnectors(QPainter *painter);

    void drawConnector(QPainter *painter,PortDir dir,int connectorIndex);
    void drawPortConnectorShape(QPainter *painter,Port &port);
    Port& getPort(PortDir dir,int connectorIndex);
    void computeConnetorGapAndOffset(const int &nPorts,float &gap,float &offset);
};

} // namespace GuiBlocks

#endif // GUIBLOCKS_BLOCK_H
