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
#include "GuiBlocks/TypeID.h"

namespace GuiBlocks {

//forward declaration of Link to avoid #include "Link.h" that will rise compile errors
class Link;
class Block : public QGraphicsItem
{
public: //exported types
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
        uint32_t uid;
        std::weak_ptr<Port> getCopy() const { return parent->getWeakPtr(this); }
        QPainterPath connectorShape;
        struct
        {
            uint16_t nodeIdx;
            Link *link = nullptr;
        } connectionLink;
        Port(){}
        Port(Block *parent,PortDir dir,QString type,QString name="");
        Block* getParent() const { return parent; }
        void connectPortToLink(Link *link,uint16_t nodeIdx);
        void disconnectPortFromLink();
        bool isConnected(){ return connectionLink.link != nullptr; }
    };
    enum class BlockOrientation
    {
        East,   //left  to right (input at right, outputs at left)
        West    //right to left  (input at left, outputs at right)
    };

public: //general methods
    Block(const QString &_type,
          const QString &name,
          QGraphicsItem *parent = nullptr);
    virtual ~Block() override;
    int type() const override{return static_cast<int>(TypeID::BlockID);}

    const QString& getType() const { return _type; }
    const QString& getName() const { return name; }

    void addPort(PortDir dir,QString _type,QString name = "");
    void setBlockOrientation(const BlockOrientation &orientation);
    BlockOrientation getBlockOrientation()const { return blockOrientation; }
    void toggleBlockOrientation();
    void setCentralPosition(const QPointF &centerPos);    //should be this implemented?
    QPointF getPortConnectionPoint(const Port &port);
    Port* isMouseOverPort(const QPointF &pos);
    bool isMouseOverBlock(const QPointF &pos);


    //test methods:
    //void toggleConnectionPortState(int &indexPort);    //remove this, just for debug

    //Interface (pure virtual) methods:
    QRectF boundingRect() const override{ return blockRect; }
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
    QPainterPath shape() const override;

    //User Interaction methods:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
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
    //void drawShadowPlace(QPainter *painter);

    void drawConnector(QPainter *painter,PortDir dir,int connectorIndex);
    void drawPortConnectorShape(QPainter *painter,Port &port);
    Port& getWeakPtr(PortDir dir,int connectorIndex);
    void computeConnetorGapAndOffset(const int &nPorts,double &gap,double &offset) const;
    void setBlockEffect(const QColor &color);

    std::weak_ptr<Port> getWeakPtr(const Port *port) const;

private: //ctor required
    QString _type;
    QString name;
    uint32_t uid;
private://internal vars
    QRectF blockRect;
    QRectF dragArea;
    std::vector<std::shared_ptr<Port>> ports;
    int nInputs;
    int nOutputs;
    QPointF center;
    BlockOrientation blockOrientation;
    int portIndexHintToDraw;
    bool enableDrag = false;
    bool hover = false;
};

} // namespace GuiBlocks

#endif // GUIBLOCKS_BLOCK_H
