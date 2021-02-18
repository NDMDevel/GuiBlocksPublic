#include "Block.h"
#include <QPainter>
#include "Utils.h"
#include <cmath>

#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>

namespace GuiBlocks {

//Block*       Block::blockUnderMouse = nullptr;
//Block::Port* Block::portUnderMouse  = nullptr;


Block::Block(const QString &type,
             const QString &name,
             QGraphicsItem *parent)
    : QGraphicsItem(parent),
      _type(type),
      name(name),
      dir(BlockOrientation::East),
      nInputs(0),
      nOutputs(0),
      center(0.0,0.0),
      blockOrientation(BlockOrientation::West),
      portIndexHintToDraw(-1)
{
    //QUuid::createUuid()
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    //block flags:
    setFlags(QGraphicsItem::ItemIsMovable);
    //setFlags(QGraphicsItem::ItemIsSelectable);
    setAcceptDrops(true);
    setAcceptHoverEvents(true);

    //shadow effect:
    setBlockEffect(StyleBlockShape::shadowColor);

    //block style:
    setOpacity(qreal(StyleBlockShape::opacity));
}

Block::~Block()
{

}

void Block::addPort(Block::PortDir dir,QString type,QString name)
{
    switch( dir )
    {
    case PortDir::Input:
        ports.insert(nInputs++,std::make_shared<Port>(Port(this,dir,type,name)));
        break;
    case PortDir::Output:
        ports.insert(nInputs+nOutputs++,std::make_shared<Port>(Port(this,dir,type,name)));
        break;
    }
    updateBoundingRect();
}

void Block::setBlockOrientation(const Block::BlockOrientation &orientation)
{
    blockOrientation = orientation;
    update();
}

void Block::toggleBlockOrientation()
{
    if( blockOrientation == BlockOrientation::East )
        setBlockOrientation(BlockOrientation::West);
    else
        setBlockOrientation(BlockOrientation::East);
}

void Block::setCentralPosition(const QPointF &centerPos)
{
    if( center == centerPos )
        return;

    center = centerPos;
    dragArea.moveCenter(center);
    blockRect.moveCenter(center);
}

QPointF Block::getPortConnectionPoint(const Block::Port &port)
{
    QPointF connectionPoint;
    if( port.dir == Block::PortDir::Input )
    {
        if( getBlockOrientation() == Block::BlockOrientation::East )
            connectionPoint.setX(port.connectorShape.boundingRect().width());
        else
            connectionPoint.setX(0);
    }
    else
    {
        if( getBlockOrientation() == Block::BlockOrientation::East )
            connectionPoint.setX(0);
        else
            connectionPoint.setX(port.connectorShape.boundingRect().width());
    }
    connectionPoint.setY(port.connectorShape.boundingRect().height()/2.0);
    connectionPoint = port.connectorShape.boundingRect().topLeft()+mapToScene(connectionPoint.x(),connectionPoint.y());

    return connectionPoint;
}

Block::Port* Block::isMouseOverPort(const QPointF &pos)
{
    for( int i=0 ; i<ports.length() ; i++ )
        if( (*ports[i]).connectorShape.boundingRect().contains(pos) )
            return &(*ports[i]);
    return nullptr;
}
//
//bool Block::isMouseOverPort(const QPointF &pos,QPointF& connectionPoint,Port **port) const
//{
//    auto localPos = mapFromScene(pos);
//    if( port != nullptr )
//        *port = nullptr;
//    for( int i=0 ; i<ports.length() ; i++ )
//        if( (*ports[i]).connectorShape.boundingRect().contains(localPos) )
//        {
//            if( port != nullptr )
//            {
//                *port = &(*ports[i]);
//
//                if( (*port)->dir == Block::PortDir::Input )
//                {
//                    if( getBlockOrientation() == Block::BlockOrientation::East )
//                        connectionPoint.setX((*port)->connectorShape.boundingRect().width());
//                    else
//                        connectionPoint.setX(0);
//                }
//                else
//                {
//                    if( getBlockOrientation() == Block::BlockOrientation::East )
//                        connectionPoint.setX(0);
//                    else
//                        connectionPoint.setX((*port)->connectorShape.boundingRect().width());
//                }
//                connectionPoint.setY((*port)->connectorShape.boundingRect().height()/2.0);
//                connectionPoint = (*port)->connectorShape.boundingRect().topLeft()+mapToScene(connectionPoint.x(),connectionPoint.y());
//            }
//            return true;
//        }
//    return false;
//}
//
bool Block::isMouseOverBlock(const QPointF &pos)
{
    bool isOver = dragArea.contains(pos);
//    qDebug() << dragArea << pos << isOver;
    return isOver;
}

void Block::toggleConnectionPortState(int &indexPort)
{
    if( ports.length() == 0 )
        return;
    if( indexPort >= ports.length() || indexPort < 0 )
        indexPort = 0;
    (*ports[indexPort]).connected = !(*ports[indexPort]).connected;
    update();
}

void Block::paint(QPainter *painter,
                  const QStyleOptionGraphicsItem *option,
                  QWidget *widget)
{
    //painter->setClipRect(option->exposedRect);

    Q_UNUSED(option)
    Q_UNUSED(widget)

//    debug_msg("paint");

    //drawBoundingRect(painter);    //just for debug
    drawBlockShape(painter);
    drawType(painter);
    drawName(painter);
    drawConnectors(painter);
    //drawShadowPlace(painter);

    if( portIndexHintToDraw != -1 )
        drawPortHint(painter,(*ports[portIndexHintToDraw]));
}

void Block::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if( enableDrag )
    {
        QGraphicsItem::mouseMoveEvent(event);
//        update();
    }
}

void Block::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    if( event->button() == Qt::MouseButton::LeftButton )
    {
        //if the clic pos is not over the drag area, the event
        //is ignore and therefore sent to the block that is behind if any
        if( isMouseOverBlock(event->pos()) )
            enableDrag = true;
        else
            event->ignore();
        return;

//        //if the clic pos is not over the drag area nor a port,
//        //the event is send to the block that is behind this (if any)
//        if( !isMouseOverBlock(event->pos()) && !isMouseOverPort(event->pos()) )
//        //if( getBlockUnderMouse() != this /*&& getPortUnderMouse() == nullptr*/ )
//        {
//            event->ignore();
//            return;
//        }
//#warning "This lines should be removed because view obj should do this instead of the block."
//        auto items = collidingItems();
//        for( auto item : items )
//        {
//            if( item->zValue() > 0 )
//                item->setZValue(0);
//        }
//        setZValue(1);
//        if( dragArea.contains(event->pos()) )
//            enableDrag = true;
//        //else
//        //    debug_msg("click over port");
//        //event->accept();
    }
}

void Block::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    if( event->button() == Qt::MouseButton::LeftButton )
    {
        if( enableDrag == true )
        {
            enableDrag = false;
            QPointF p = nextGridPosition(mapToScene(event->pos())-event->pos(),StyleGrid::gridSize);
//            if( pos() == p )
//                return;
            setPos(p);
        }
    }
}

void Block::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItem::hoverMoveEvent(event);
    int currentHintIndex = portIndexHintToDraw;
    for( int i=0 ; i<ports.length() ; i++ )
        if( (*ports[i]).connectorShape.boundingRect().contains(event->pos()) )
        {
            portIndexHintToDraw = i;
//            portUnderMouse = &(*ports[portIndexHintToDraw]);
//            blockUnderMouse = portUnderMouse->getParent();
//            debug_msg(portUnderMouse->name);
            break;
        }
        else
        {
            portIndexHintToDraw = -1;
//            portUnderMouse = nullptr;
        }

    bool needsUpdate = (currentHintIndex != portIndexHintToDraw);
    if( hover != dragArea.contains(event->pos()) )
    {
//        debug_msg("hover");
        hover = dragArea.contains(event->pos());
        if( hover )
        {
            setBlockEffect(StyleBlockShape::shadowColorOnHover);
//            blockUnderMouse = this;
//            debug_msg(blockUnderMouse->getName());
//            debug_msg("hover block");
        }
        else
        {
            setBlockEffect(StyleBlockShape::shadowColor);
//            if( portUnderMouse == nullptr )
//                blockUnderMouse = nullptr;
        }
        needsUpdate = true;
    }
    if( needsUpdate )
        update();
}

void Block::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItem::hoverLeaveEvent(event);
    bool needsUpdate = false;

    //if hover was enabled, the block needs to change its shadow
    //to the normal state, and thus needs update:
    if( hover == true )
    {
        hover = false;
        setBlockEffect(StyleBlockShape::shadowColor);
        needsUpdate = true;
    }

    //when the mouse move too fast going outside of the block
    //the hoverMoveEvent may fail to set the hint index to -1,
    //to avoid that this method is implemented
    if( portIndexHintToDraw != -1 )
    {
        portIndexHintToDraw = -1;
        needsUpdate = true;
    }
    if( needsUpdate )
        update();

//    blockUnderMouse = nullptr;
//    portUnderMouse  = nullptr;
}

void Block::updateBoundingRect()
{
    //compute inner block width (enough space to write the "type" of the block plus some gap)
    //This inner width will always be an even multiple of the gridSize
    QFontMetrics fontMetrics = QFontMetrics(StyleText::blockTypeFont);
    float innerBlockWidth = 2.0f*StyleText::gapTypeToBorderGridSizePercent*fontMetrics.capHeight()
                            + fontMetrics.horizontalAdvance(_type);
    innerBlockWidth  = nextEvenGridValue(innerBlockWidth,StyleGrid::gridSize);

    //Compute Max width of the texts:
    //This texts are the block "name" which will be displayed on top of the block.
    //Below the block will be displayed the connector "name" and "type"
    //(this one inside parentheses).
    //This text width (maxPortTextWidth) or the inner block width (innerBlockWidth),
    //whichever greater, will define the boundingRect width (maxBoundingWidth):
    fontMetrics = QFontMetrics(StyleText::blockNameFont);
    float maxPortTextWidth = fontMetrics.horizontalAdvance(name);
    fontMetrics = QFontMetrics(StyleText::blockHintFont);
    bool hasPortType = false;
    bool hasPortName = false;
    for( auto &port : ports )
    {
        if( (*port).name != "" )
            hasPortName = true;
        int width = fontMetrics.horizontalAdvance((*port).name);
        maxPortTextWidth = max(width,maxPortTextWidth);
        if( (*port).type != "" )
        {
            hasPortType = true;
            width = fontMetrics.horizontalAdvance("("+(*port).type+")");
            maxPortTextWidth = max(width,maxPortTextWidth);
        }
    }

    maxPortTextWidth = nextEvenGridValue(maxPortTextWidth,StyleGrid::gridSize);

    float maxBoundingWidth = 0;
    if( nInputs!=0 || nOutputs!=0 )
        maxBoundingWidth = max(maxPortTextWidth,innerBlockWidth + 2.0f*StyleBlockShape::connectorSizeGridSizePercent.width()*StyleGrid::gridSize);

    //compute text header and footer (block "name" and connectors "name" and "type"):
    if( QFontMetrics(StyleText::blockNameFont).capHeight() > fontMetrics.capHeight() )
        fontMetrics = QFontMetrics(StyleText::blockNameFont);
    float headerFooterTextHeight = 0;
    if( hasPortName || !_type.isEmpty() )
        headerFooterTextHeight += 2.0f*(fontMetrics.capHeight()*1.75f+0*StyleText::gapTextToBorderGridSizePercent*StyleGrid::gridSize);
    if( hasPortType )
        headerFooterTextHeight += 2.0f*(fontMetrics.capHeight()*1.75f+0*StyleText::gapTextToBorderGridSizePercent*StyleGrid::gridSize);

    //compute Max height due to the IO ports:
    float maxConHeight = 2*max(nInputs,nOutputs);
    maxConHeight = max(maxConHeight,2)*StyleGrid::gridSize;

    //Compute the inner height: it will be defined by the heigth required by the connectors
    //or by "type" displayed inside the block. Whichever greater will define the inner heigth:
    fontMetrics = QFontMetrics(StyleText::blockTypeFont);
    float innerBlockHeight = max(maxConHeight,nextEvenGridValue(fontMetrics.capHeight()*(1.0f+2.0f*StyleText::gapTypeToBorderGridSizePercent),StyleGrid::gridSize));

    //innerBlockHeight will always be an even multiple of the gridSize,
    //this implies that the size of the dragArea (inner block) has a heigth
    //and width that is a even multple of the gridSize, and so the center
    //will always be located at a exact grid location:

    dragArea.setSize(QSizeF(innerBlockWidth,innerBlockHeight));
    dragArea.moveCenter(center);

    float maxBoundingHeigth = nextGridValue(innerBlockHeight+headerFooterTextHeight,StyleGrid::gridSize);
    blockRect.setSize(QSizeF(maxBoundingWidth,maxBoundingHeigth));
    blockRect.moveCenter(center);
//    {
//      auto effect = new QGraphicsDropShadowEffect;
//      effect->boundingRectFor(dragArea);
//      effect->setOffset(2, 2);
//      effect->setBlurRadius(10);
//      effect->setColor(StyleBlockShape::shadowColor);
//
//      setGraphicsEffect(effect);
//    }
}

void Block::drawBoundingRect(QPainter *painter)
{
    painter->save();

    QPen pen(Qt::red);
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);

    painter->drawRoundedRect(blockRect,8,8);

    painter->restore();
}

void Block::drawType(QPainter *painter)
{
    painter->save();

    painter->setPen(StyleText::blockTypeColor);
    painter->setFont(StyleText::blockTypeFont);
    QFontMetrics fontMetrics = QFontMetrics(StyleText::blockTypeFont);
    QPointF offsetPos = fontMetrics.boundingRect(_type).center();

    painter->drawText(dragArea.center()-offsetPos,_type);

    painter->restore();
}

void Block::drawName(QPainter *painter)
{
    painter->save();

    painter->setPen(StyleText::blockNameColor);
    painter->setFont(StyleText::blockNameFont);
    QFontMetrics fontMetrics = QFontMetrics(StyleText::blockNameFont);
    QRectF boundingRectText = fontMetrics.boundingRect(name);
    QPointF offsetPos;
    offsetPos.setY(dragArea.top()-fontMetrics.capHeight()*0.75-0*StyleText::gapTextToBorderGridSizePercent*StyleGrid::gridSize);
    offsetPos.setX(dragArea.center().x()-boundingRectText.center().x());
    boundingRectText.moveTo(offsetPos);
    painter->drawText(offsetPos,name);

    painter->restore();
}

void Block::drawPortHint(QPainter *painter, const Block::Port &port)
{
    painter->save();
    painter->setPen(StyleText::blockHintColor);
    painter->setFont(StyleText::blockHintFont);
    QFontMetrics fontMetrics = QFontMetrics(StyleText::blockHintFont);
    if( !port.name.isEmpty() )
    {
        QRectF boundingRectText = fontMetrics.boundingRect(port.name);
        QPointF offsetPos;
        offsetPos.setY(dragArea.bottom()+fontMetrics.capHeight()*1.75+0*boundingRectText.height());
        offsetPos.setX(dragArea.center().x()-boundingRectText.center().x());
        painter->drawText(offsetPos,port.name);
    }
    if( !port.type.isEmpty() )
    {
        QString portType = "(" + port.type + ")";
        QRectF boundingRectText = fontMetrics.boundingRect(portType);
        QPointF offsetPos;
        offsetPos.setY(dragArea.bottom()+2.0f*fontMetrics.capHeight()*1.75+0*2.0*boundingRectText.height());
        offsetPos.setX(dragArea.center().x()-boundingRectText.center().x());
        painter->drawText(offsetPos,portType);
    }
    painter->restore();
}

void Block::drawBlockShape(QPainter *painter)
{
    painter->save();

    QLinearGradient gradient(dragArea.width()*StyleBlockShape::customFillGradient.start().x(),
                             dragArea.height()*StyleBlockShape::customFillGradient.start().y(),
                             dragArea.width()*StyleBlockShape::customFillGradient.finalStop().x(),
                             dragArea.height()*StyleBlockShape::customFillGradient.finalStop().y());

    gradient.setColorAt(0.0,StyleBlockShape::blockRectFillColor1);
    gradient.setColorAt(0.7,StyleBlockShape::blockRectFillColor2);

    painter->setBrush(gradient);
    if( hover )
        painter->setPen(QPen(StyleBlockShape::blockRectBorderColorOnHover,2));
    else
        painter->setPen(QPen(StyleBlockShape::blockRectBorderColor,1));
    painter->drawRoundedRect(dragArea,
                             dragArea.width()*StyleBlockShape::roundingXWidthPercent,
                             dragArea.height()*StyleBlockShape::roundingYWidthPercent);

    painter->restore();
}

void Block::drawConnectors(QPainter *painter)
{
    for( int i=0 ; i<nInputs ; i++ )
        drawConnector(painter,PortDir::Input,i);
    for( int i=0 ; i<nOutputs ; i++ )
        drawConnector(painter,PortDir::Output,i);
}

/*void Block::drawShadowPlace(QPainter *painter)
{
//    qDebug() << pos() << scenePos() << mapToItem(this,nextGridPosition(pos(),gridSize));
    QRectF rect;
    qDebug() << (nextGridPosition(mapToScene(pos()),gridSize));
    rect.setSize(dragArea.size());
    rect.moveCenter(pos()-nextGridPosition(mapToScene(pos()),gridSize));
    if( enableDrag )
        painter->drawRect(rect);
}*/

/**
@brief Draw a connector on the current block

This is the top level method that should be called to
draw a connector on the block.
*/
void Block::drawConnector(QPainter *painter, Block::PortDir dir, int connectorIndex)
{
    if( nInputs == 0 && nOutputs == 0 )
        return;
    if( dir == PortDir::Input )
    {
        if( connectorIndex >= nInputs )
            return;
        float offset;
        float gap;
        computeConnetorGapAndOffset(nInputs,gap,offset);
        Port &port = getPort(dir,connectorIndex);
        port.connectorShape = QPainterPath();   //resets the QPainterPath
        if( blockOrientation == BlockOrientation::West )
            port.connectorShape.moveTo(dragArea.left(),dragArea.top()+offset+gap*float(connectorIndex));
        if( blockOrientation == BlockOrientation::East )
            port.connectorShape.moveTo(dragArea.right(),dragArea.top()+offset+gap*float(connectorIndex));
        drawPortConnectorShape(painter,port);
    }
    if( dir == PortDir::Output )
    {
        if( connectorIndex >= nOutputs )
            return;
        float offset;
        float gap;
        computeConnetorGapAndOffset(nOutputs,gap,offset);
        Port &port = getPort(dir,connectorIndex);
        port.connectorShape = QPainterPath();   //resets the QPainterPath
        if( blockOrientation == BlockOrientation::West )
            port.connectorShape.moveTo(dragArea.right(),dragArea.top()+offset+gap*float(connectorIndex));
        if( blockOrientation == BlockOrientation::East )
            port.connectorShape.moveTo(dragArea.left(),dragArea.top()+offset+gap*float(connectorIndex));
        drawPortConnectorShape(painter,port);
    }
}

void Block::drawPortConnectorShape(QPainter *painter, Block::Port &port)
{
    QSizeF size = StyleBlockShape::connectorSizeGridSizePercent*StyleGrid::gridSize;
    QPointF arrowTip = port.connectorShape.currentPosition();
    if( blockOrientation == BlockOrientation::West )
    {
        if( port.dir == PortDir::Output )
        {
            arrowTip.setX(arrowTip.x()+size.width());
            port.connectorShape.moveTo(arrowTip);
        }

        port.connectorShape.lineTo(arrowTip.x()-size.width(),
                                   arrowTip.y()-size.height()/2.0f );
        if( port.connected )
            port.connectorShape.lineTo(arrowTip.x()-size.width(),
                                       arrowTip.y()+size.height()/2.0f );
        else
            port.connectorShape.moveTo(arrowTip.x()-size.width(),
                                       arrowTip.y()+size.height()/2.0f );
        port.connectorShape.lineTo(arrowTip);
    }
    if( blockOrientation == BlockOrientation::East )
    {
        if( port.dir == PortDir::Output )
        {
            arrowTip.setX(arrowTip.x()-size.width());
            port.connectorShape.moveTo(arrowTip);
        }

        port.connectorShape.lineTo(arrowTip.x()+size.width(),
                                   arrowTip.y()+size.height()/2.0f );
        if( port.connected )
            port.connectorShape.lineTo(arrowTip.x()+size.width(),
                                       arrowTip.y()-size.height()/2.0f );
        else
            port.connectorShape.moveTo(arrowTip.x()+size.width(),
                                       arrowTip.y()-size.height()/2.0f );
        port.connectorShape.lineTo(arrowTip);
    }
    painter->save();
    QColor fillColor;
    QColor borderColor;
    if( port.dir == PortDir::Input )
    {
        borderColor = StyleBlockShape::inputConnectorBorderColor;
        if( port.connected )
            fillColor = StyleBlockShape::inputConnectorFillColor;
    }
    else
    {
        borderColor = StyleBlockShape::outputConnectorBorderColor;
        if( port.connected )
            fillColor = StyleBlockShape::outputConnectorFillColor;
    }
    painter->setPen(borderColor);
    if( port.connected )
        painter->fillPath(port.connectorShape,fillColor);
    painter->drawPath(port.connectorShape);
    painter->restore();
}

Block::Port &Block::getPort(Block::PortDir dir, int connectorIndex)
{
    //handle errors
    if( connectorIndex >= ports.length() || connectorIndex < 0 )
        goto error;
    if( (dir == PortDir::Input) && (connectorIndex>nInputs) )
        goto error;
    if( (dir == PortDir::Output) && (connectorIndex>nOutputs) )
        goto error;

    if( dir == PortDir::Input )
        return (*ports[connectorIndex]);
    return (*ports[nInputs+connectorIndex]);

    error:
    throw("getPort(): index out of range: QVector<Port> ports.");
}

void Block::computeConnetorGapAndOffset(const int &nPorts,float &gap, float &offset) const
{
    //  *** Algorithm ***
    //
    //  This algorithm equally distribute the height
    //  of the block on every port.
    //
    //  if( Height/(nPorts+1) == Integer )
    //  {
    //      Offset = Height / (nPorts+1)
    //      Gap    = Height / (nPorts+1)
    //  }
    //  else
    //  {
    //      Offset =  ceil( Height / nPorts / 2 )
    //      Gap    = floor( Height / nPorts )
    //  }
    if( isInteger( dragArea.height()/StyleGrid::gridSize/float(nPorts+1) ) )
    {
        offset = dragArea.height() / float(nPorts+1);
        gap    = offset;
    }
    else
    {
        offset = std::ceil(  dragArea.height()/StyleGrid::gridSize/float(nPorts) / 2.0f )*StyleGrid::gridSize;
        gap    = std::floor( dragArea.height()/StyleGrid::gridSize/float(nPorts) )*StyleGrid::gridSize;
    }
}

void Block::setBlockEffect(const QColor &color)
{
    auto effect = new QGraphicsDropShadowEffect;
    //effect->boundingRectFor(dragArea);
    effect->setOffset(2, 2);
    effect->setBlurRadius(15);
    effect->setColor(color);
    setGraphicsEffect(effect);
}

Block::Port::Port(Block *parent,Block::PortDir dir, QString type, QString name)
    : parent(parent),
      dir(dir),
      type(type),
      name(name)
{
    connected = false;
    if( dir == Block::PortDir::Input )
    {
        multipleConnections = false;
    }
    else
    {
        multipleConnections = true;
    }
}


} // namespace GuiBlocks
