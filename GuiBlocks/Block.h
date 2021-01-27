#ifndef GUIBLOCKS_BLOCK_H
#define GUIBLOCKS_BLOCK_H

#include <QGraphicsItem>
#include <QVector>
#include <cstdint>
#include <memory>
#include <QDebug>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include "GuiBlocks/Style.h"
//#include "GuiBlocks/TypeID.h"

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
        Block   *parent = nullptr;
        PortDir dir = PortDir::Input;
        QString type;
        QString name;
        bool    connected = false;
        bool    multipleConnections = false;
        QPainterPath connectorShape;
        Port(){}
        Port(Block *parent,PortDir dir,QString type,QString name="");
        Block* getParent(){ return parent; }
    };
    enum class BlockOrientation
    {
        East,   //left  to right
        West    //right to left
    };
private:
    //ctor required
    QString _type;
    QString name;

private:
    //internals
    QRectF blockRect;
    QRectF dragArea;
    QVector<std::shared_ptr<Port>> ports;
    BlockOrientation dir;
    int nInputs;
    int nOutputs;
    QPointF center;
    BlockOrientation blockOrientation;
    int portIndexHintToDraw;
    bool enableDrag = false;
    bool hover = false;

    //when mouse is over a block, the pointer blockUderMouse
    //is set with the block address. If the mouse is also over
    //a port, portUnderMouse y set with the address of the port.
    //Note: this are updated in hoverMoveEvent and hoverLeaveEvent.
//    static Block*       blockUnderMouse;
//    static Block::Port* portUnderMouse;

public:
    Block(const QString &_type,
          const QString &name,
          QGraphicsItem *parent = nullptr);
    virtual ~Block();

//    int type() const override{return TypeID::BlockID;}

    const QString& getType() const { return _type; }
    const QString& getName() const { return name; }

    void addPort(PortDir dir,QString _type,QString name = "");
    void setBlockOrientation(const BlockOrientation &orientation);
    BlockOrientation getBlockOrientation()const { return blockOrientation; }
    void toggleBlockOrientation();
    void setCentralPosition(const QPointF &centerPos);    //should be this implemented?
    QPointF getPortConnectionPoint(const Port &port);
    Port* isMouseOverPort(const QPointF &pos);
//    bool isMouseOverPort(const QPointF &pos,QPointF& connectionPoint,Port **port = nullptr) const;
    bool isMouseOverBlock(const QPointF &pos);

    //test methods:
    void toggleConnectionPortState(int &indexPort);    //remove this, just for debug

    //Interface (pure virtual) methods:
    QRectF boundingRect() const override{ return blockRect; }
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
    //User Interaction methods:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    //methods to be called from the View object to determine
    //if a block or port is under mouse position:
//    static Block*       getBlockUnderMouse(){return blockUnderMouse;}
//    static Block::Port* getPortUnderMouse() {return portUnderMouse;}
//    int type() const override{ return TypeID::BlockID; }

private:
    void updateBoundingRect();
    void drawBoundingRect(QPainter *painter);
    void drawType(QPainter *painter);
    void drawName(QPainter *painter);
    void drawPortHint(QPainter *painter,const Port &port);
    void drawBlockShape(QPainter *painter);
    void drawConnectors(QPainter *painter);
    //void drawShadowPlace(QPainter *painter);

    void drawConnector(QPainter *painter,PortDir dir,int connectorIndex);
    void drawPortConnectorShape(QPainter *painter,Port &port);
    Port& getPort(PortDir dir,int connectorIndex);
    void computeConnetorGapAndOffset(const int &nPorts,float &gap,float &offset) const;
    void setBlockEffect(const QColor &color);
};

} // namespace GuiBlocks

#endif // GUIBLOCKS_BLOCK_H
