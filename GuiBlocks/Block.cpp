#include "Block.h"
#include <QPainter>
#include "Utils.h"
#include <cmath>

#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneHoverEvent>

namespace GuiBlocks {

Block::Block(const QString &type,
             const QString &name,
             //QFontMetrics  &fontMetrics,
             float &gridSize,
             QGraphicsItem *parent)
    : QGraphicsItem(parent),
      type(type),
      name(name),
      fontMetrics(QFont()),
      dir(BlockOrientation::East),
      gridSize(gridSize),
      nInputs(0),
      nOutputs(0),
      center(0.0f,0.0f),
      blockOrientation(BlockOrientation::West),
      portHintToDraw(-1)
{
    //QUuid::createUuid()
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    {
      auto effect = new QGraphicsDropShadowEffect;
      effect->setOffset(2, 2);
      effect->setBlurRadius(15);
      effect->setColor(StyleBlockShape::shadowColor);

      setGraphicsEffect(effect);
    }

    setOpacity(StyleBlockShape::opacity);
}

void Block::addPort(Block::PortDir dir,QString type,QString name)
{
    switch( dir )
    {
    case PortDir::Input:
        ports.insert(nInputs++,Port(dir,name,type));
//        ports.push_back(Port(dir,name,type));
        break;
    case PortDir::Output:
        ports.insert(nInputs+nOutputs++,Port(dir,name,type));
//        ports.push_front(Port(dir,name,type));
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

void Block::toggleConnectionPortState(int &indexPort)
{
    if( ports.length() == 0 )
        return;
    if( indexPort >= ports.length() || indexPort < 0 )
        indexPort = 0;
    ports[indexPort].connected = !ports[indexPort].connected;
    update();
}

void Block::paint(QPainter *painter,
                  const QStyleOptionGraphicsItem *option,
                  QWidget *widget)
{
    //painter->setClipRect(option->exposedRect);

    Q_UNUSED(option);
    Q_UNUSED(widget);

    static int count = 0;
    qDebug() << "paint" << count++ << painter->font();

    //just for debug:
    drawBoundingRect(painter);
    drawBlockShape(painter);
    drawType(painter);
    drawName(painter);
    for( int i=0 ; i<nInputs ; i++ )
        drawConnector(painter,PortDir::Input,i);
    for( int i=0 ; i<nOutputs ; i++ )
        drawConnector(painter,PortDir::Output,i);


    if( portHintToDraw != -1 )
        drawPortHint(painter,ports[portHintToDraw]);
}

void Block::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
    //    qDebug() << "mouseMove";
}

void Block::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItem::hoverMoveEvent(event);
    int currentHintIndex = portHintToDraw;
    for( int i=0 ; i<ports.length() ; i++ )
        if( ports[i].connectorShape.boundingRect().contains(event->pos()) )
        {
            portHintToDraw = i;
            break;
        }
        else
            portHintToDraw = -1;
    if( currentHintIndex != portHintToDraw )
        update();
}

void Block::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItem::hoverLeaveEvent(event);

    //when the mouse move too fast going outside of the block
    //the hoverMoveEvent may fail to set the hint index to -1,
    //to avoid that this method is implemented
    if( portHintToDraw != -1 )
    {
        portHintToDraw = -1;
        update();
    }
}

void Block::updateBoundingRect()
{
    //compute inner block width (enough space to write the "type" of the block plus some gap)
    //This inner width will always be an even multiple of the gridSize
    fontMetrics = QFontMetrics(StyleText::blockTypeFont);
    float innerBlockWidth = 2.0f*StyleText::gapTypeToBorderGridSizePercent*fontMetrics.capHeight()
                            + fontMetrics.width(type);
    innerBlockWidth  = nextEvenGridValue(innerBlockWidth,gridSize);

    //Compute Max width of the texts:
    //This texts are the block "name" which will be displayed on top of the block.
    //Below the block will be displayed the connector "name" and "type"
    //(this one inside parentheses).
    //This text width (maxPortTextWidth) or the inner block width (innerBlockWidth),
    //whichever greater, will define the boundingRect width (maxBoundingWidth):
    fontMetrics = QFontMetrics(StyleText::blockNameFont);
    float maxPortTextWidth = fontMetrics.width(name);
    fontMetrics = QFontMetrics(StyleText::blockHintFont);
    bool hasPortType = false;
    bool hasPortName = false;
    for( auto &port : ports )
    {
        if( port.name != "" )
            hasPortName = true;
        int width = fontMetrics.width(port.name);
        maxPortTextWidth = max(width,maxPortTextWidth);
        if( port.type != "" )
        {
            hasPortType = true;
            width = fontMetrics.width("("+port.type+")");
            maxPortTextWidth = max(width,maxPortTextWidth);
        }
    }

    maxPortTextWidth = nextEvenGridValue(maxPortTextWidth,gridSize);

    float maxBoundingWidth = 0;
    if( nInputs!=0 || nOutputs!=0 )
        maxBoundingWidth = max(maxPortTextWidth,innerBlockWidth + 2.0f*StyleBlockShape::connectorSizeGridSizePercent.width()*gridSize);

    //compute text header and footer (block "name" and connectors "name" and "type"):
    if( QFontMetrics(StyleText::blockNameFont).capHeight() > fontMetrics.capHeight() )
        fontMetrics = QFontMetrics(StyleText::blockNameFont);
    float headerFooterTextHeight = 0;
    if( hasPortName || !type.isEmpty() )
        headerFooterTextHeight += 2.0f*(fontMetrics.capHeight()*1.75f+0*StyleText::gapTextToBorderGridSizePercent*gridSize);
    if( hasPortType )
        headerFooterTextHeight += 2.0f*(fontMetrics.capHeight()*1.75f+0*StyleText::gapTextToBorderGridSizePercent*gridSize);

    //compute Max height due to the IO ports:
    float maxConHeight = 2*max(nInputs,nOutputs);
    maxConHeight = max(maxConHeight,2)*gridSize;

    //Compute the inner height: it will be defined by the heigth required by the connectors
    //or by "type" displayed inside the block. Whichever greater will define the inner heigth:
    fontMetrics = QFontMetrics(StyleText::blockTypeFont);
    float innerBlockHeight = max(maxConHeight,nextEvenGridValue(fontMetrics.capHeight()*(1.0f+2.0f*StyleText::gapTypeToBorderGridSizePercent),gridSize));

    //innerBlockHeight will always be an even multiple of the gridSize,
    //this implies that the size of the dragArea (inner block) has a heigth
    //and width that is a even multple of the gridSize, and so the center
    //will always be located at a exact grid location:

    dragArea.setSize(QSizeF(innerBlockWidth,innerBlockHeight));
    dragArea.moveCenter(center);

    float maxBoundingHeigth = nextGridValue(innerBlockHeight+headerFooterTextHeight,gridSize);
    blockRect.setSize(QSizeF(maxBoundingWidth,maxBoundingHeigth));
    blockRect.moveCenter(center);
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
    fontMetrics = QFontMetrics(StyleText::blockTypeFont);
    QPointF offsetPos = fontMetrics.boundingRect(type).center();
    painter->drawText(-offsetPos,type);

    painter->restore();
}

void Block::drawName(QPainter *painter)
{
    painter->save();

    painter->setPen(StyleText::blockNameColor);
    painter->setFont(StyleText::blockNameFont);
    fontMetrics = QFontMetrics(StyleText::blockNameFont);
    QRectF boundingRectText = fontMetrics.boundingRect(name);
    QPointF offsetPos;
    offsetPos.setY(dragArea.top()-fontMetrics.capHeight()*0.75-0*StyleText::gapTextToBorderGridSizePercent*gridSize);
    offsetPos.setX(/*dragArea.center().x()*/-boundingRectText.center().x());
    boundingRectText.moveTo(offsetPos);
    painter->drawText(offsetPos,name);

    painter->restore();
}

void Block::drawPortHint(QPainter *painter, const Block::Port &port)
{
    painter->save();
    painter->setPen(StyleText::blockHintColor);
    painter->setFont(StyleText::blockHintFont);
    fontMetrics = QFontMetrics(StyleText::blockHintFont);
    if( !port.name.isEmpty() )
    {
        QRectF boundingRectText = fontMetrics.boundingRect(port.name);
        QPointF offsetPos;
        offsetPos.setY(dragArea.bottom()+fontMetrics.capHeight()*1.75+0*boundingRectText.height());
        offsetPos.setX(-boundingRectText.center().x());
        painter->drawText(offsetPos,port.name);
    }
    if( !port.type.isEmpty() )
    {
        QString portType = "(" + port.type + ")";
        QRectF boundingRectText = fontMetrics.boundingRect(portType);
        QPointF offsetPos;
        offsetPos.setY(dragArea.bottom()+2.0f*fontMetrics.capHeight()*1.75+0*2.0*boundingRectText.height());
        offsetPos.setX(-boundingRectText.center().x());
        painter->drawText(offsetPos,portType);
    }
    painter->restore();
}

void Block::drawBlockShape(QPainter *painter)
{
    painter->save();

//    QLinearGradient gradient(dragArea.width()/2,0,
//                             dragArea.width()/2,dragArea.height());
    QLinearGradient gradient(dragArea.width()*StyleBlockShape::customFillGradient.start().x(),
                             dragArea.height()*StyleBlockShape::customFillGradient.start().y(),
                             dragArea.width()*StyleBlockShape::customFillGradient.finalStop().x(),
                             dragArea.height()*StyleBlockShape::customFillGradient.finalStop().y());

    gradient.setColorAt(0.0,StyleBlockShape::blockRectFillColor1);
    gradient.setColorAt(0.7,StyleBlockShape::blockRectFillColor2);

    painter->setBrush(gradient);
    painter->setPen(StyleBlockShape::blockRectBorderColor);
    painter->drawRoundedRect(dragArea,
                             dragArea.width()*StyleBlockShape::roundingXWidthPercent,
                             dragArea.height()*StyleBlockShape::roundingYWidthPercent);

    painter->restore();
}

void Block::drawConnectors(QPainter *painter)
{
    Q_UNUSED(painter);
    /*
    float xOffset = 0;
    float yOffset = 0;
    float iCount = 0;
    float oCount = 0;
    float offset = std::abs(float(nOutputs-nInputs));
    for( auto &port : ports )
    {
        if( port.dir == PortDir::Input )
        {
            xOffset = dragArea.left()-StyleBlockShape::connectorSizeGridSizePercent.width();
            yOffset = gridSize*(iCount+1)*dragArea.height()/(StyleBlockShape::connectorSizeGridSizePercent.height());
            if( nInputs < nOutputs )
                yOffset = gridSize*(iCount+(nOutputs-nInputs)+1)*dragArea.height()/(StyleBlockShape::connectorSizeGridSizePercent.height());
            else
                yOffset = gridSize*(iCount+1)*dragArea.height()/(StyleBlockShape::connectorSizeGridSizePercent.height());

            port.connectorShape.moveTo(xOffset,yOffset);
            port.connectorShape.lineTo(xOffset+StyleBlockShape::connectorSizeGridSizePercent.width(),
                                       yOffset+StyleBlockShape::connectorSizeGridSizePercent.height()/2.0f);
            port.connectorShape.lineTo(xOffset,
                                       yOffset+StyleBlockShape::connectorSizeGridSizePercent.height());
            painter->drawPath(port.connectorShape);
            iCount++;
        }
        else
        {

        }
    }*/
}

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
    QSizeF size = StyleBlockShape::connectorSizeGridSizePercent*gridSize;
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
        return ports[connectorIndex];
    return ports[nInputs+connectorIndex];

    error:
    throw("getPort(): index out of range: QVector<Port> ports.");
}

void Block::computeConnetorGapAndOffset(const int &nPorts,float &gap, float &offset)
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
    if( isInteger( dragArea.height()/gridSize/float(nPorts+1) ) )
    {
        offset = dragArea.height() / float(nPorts+1);
        gap    = offset;
    }
    else
    {
        offset = std::ceil(  dragArea.height()/gridSize/float(nPorts) / 2.0f )*gridSize;
        gap    = std::floor( dragArea.height()/gridSize/float(nPorts) )*gridSize;
    }
}

Block::Port::Port(Block::PortDir dir, QString name, QString type)
    : dir(dir),
      name(name),
      type(type)
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
