#include "View.h"

//GuiBlocks includes
#include "Block.h"
#include "Utils.h"

//std includes
#include <cmath>

//Qt includes
#include <QMouseEvent>
#include <QGraphicsItem>
#include <QDebug>

namespace GuiBlocks {

View::View(QWidget *parent)
    : QGraphicsView(parent),
      scene(parent),
      fontMetrics(viewport()->fontMetrics())
{
    //setMouseTracking(true);
    //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setOptimizationFlag(QGraphicsView::DontSavePainterState);
    setRenderHints(QPainter::Antialiasing |
                   QPainter::SmoothPixmapTransform |
                   QPainter::HighQualityAntialiasing);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setCacheMode(QGraphicsView::CacheBackground);

    QBrush brush;
    brush.setColor(StyleGrid::backgroundGridColor);
    brush.setStyle(Qt::BrushStyle::SolidPattern);
    setBackgroundBrush(brush);

    gridSize = StyleGrid::gridSize;

    QGraphicsView::setScene(&scene);
}

void View::addBlock()
{
    static int countType = 0;
    switch(countType)
    {
    case 0:
        {
            static int blockIdx = 0;
            Block *b = new Block("FIR","LowPassFilter "+QString::number(blockIdx++));
            b->addPort(Block::PortDir::Input,"Double","In");
            b->addPort(Block::PortDir::Input,"Int","Order");
            b->addPort(Block::PortDir::Output,"Double","Out");
            last = b;
            countType++;
        }
        break;
    case 1:
        {
            static int blockIdx = 0;
            Block *b = new Block("Downsampler","Fractional "+QString::number(blockIdx++));
            b->addPort(Block::PortDir::Input,"Float","Sample");
            b->addPort(Block::PortDir::Input,"Int","N");
            b->addPort(Block::PortDir::Input,"Float","SampleFrequency");
            b->addPort(Block::PortDir::Output,"Float","Output");
            last = b;
            countType++;
        }
        break;
    case 2:
        {
            static int blockIdx = 0;
            Block *b = new Block("PI","Pi "+QString::number(blockIdx++));
            b->addPort(Block::PortDir::Input,"Float","In");
            b->addPort(Block::PortDir::Output,"Float","Out");
            last = b;
            countType++;
        }
        break;
    case 3:
        {
            static int blockIdx = 0;
            Block *b = new Block("PID","Pid "+QString::number(blockIdx++));
            b->addPort(Block::PortDir::Input ,"Float","In 1");
            b->addPort(Block::PortDir::Input ,"Float","In 2");
            b->addPort(Block::PortDir::Input ,"Float","In 3");
            b->addPort(Block::PortDir::Input ,"Float","In 4");
            b->addPort(Block::PortDir::Input ,"Float","In 5");
            b->addPort(Block::PortDir::Output,"Float","Out 1");
            b->addPort(Block::PortDir::Output,"Float","Out 2");
            b->addPort(Block::PortDir::Output,"Float","Out 3");
            b->addPort(Block::PortDir::Output,"Float","Out 4");
            b->addPort(Block::PortDir::Output,"Float","Out 5");
            last = b;
            countType++;
        }
        break;
    default:
        {
            static int blockIdx = 0;
            Block *b = new Block("Threshold","OverVoltage "+QString::number(blockIdx++));
            b->addPort(Block::PortDir::Input,"Int","One");
            b->addPort(Block::PortDir::Input,"DQ","PLL Input");
            b->addPort(Block::PortDir::Input,"Float","Input Voltage");
            b->addPort(Block::PortDir::Output,"Double","Output Voltage");
            b->addPort(Block::PortDir::Output,"char","Letter");
            last = b;
            countType = 0;
        }
    }
    scene.addItem(last);
}

void View::flipLastBlock()
{
    if( last == nullptr )
        return;
    last->toggleBlockOrientation();//Block::BlockOrientation::East);
}

void View::forcedConnectedLastBlock()
{
    if( last == nullptr )
        return;
    static int port = 0;
    last->toggleConnectionPortState(port);
    port++;
}

void View::drawBackground(QPainter* painter, const QRectF &r)
{
    QGraphicsView::drawBackground(painter, r);

    auto drawGrid = [&](double gridStep)
    {
        QRect   windowRect = rect();
        QPointF tl = mapToScene(windowRect.topLeft());
        QPointF br = mapToScene(windowRect.bottomRight());

        double left   = std::floor(tl.x() / gridStep - 0.5);
        double right  = std::floor(br.x() / gridStep + 1.0);
        double bottom = std::floor(tl.y() / gridStep - 0.5);
        double top    = std::floor (br.y() / gridStep + 1.0);

        // vertical lines
        for (int xi = int(left); xi <= int(right); ++xi)
        {
            QLineF line(xi * gridStep, bottom * gridStep,
                        xi * gridStep, top * gridStep );

            painter->drawLine(line);
        }

        // horizontal lines
        for (int yi = int(bottom); yi <= int(top); ++yi)
        {
            QLineF line(left * gridStep, yi * gridStep,
                        right * gridStep, yi * gridStep );
            painter->drawLine(line);
        }
    };

    //  auto const &flowViewStyle = StyleCollection::flowViewStyle();

    QPen pfine(StyleGrid::fineGridColor,
               StyleGrid::fineGridWidth);

    painter->setPen(pfine);
    drawGrid(gridSize);

    QPen p(StyleGrid::coarseGridColor,
           StyleGrid::coarseGridWidth);

    painter->setPen(p);
    drawGrid(10*gridSize);
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    //this apply panView
    if( ((event->modifiers() == Qt::NoModifier)  && (event->buttons() == Qt::MidButton)) ||
        ((event->modifiers() == Qt::AltModifier) && (event->buttons() == Qt::LeftButton)) )
    {
        QPointF difference = clicPos - mapToScene(event->pos());
        setSceneRect(sceneRect().translated(difference.x(), difference.y()));
    }

    //[DEBUG] test link between connectors:
    if( vline != nullptr )
    {
        endPos = nextGridPosition(mapToScene(event->pos()),StyleGrid::gridSize);
        vline->setLine(QLineF(startPos,endPos));
    }
}

void View::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);

    //This apply panView: middle mouse btn, or Alt + Left Click
    if( ((event->modifiers() == Qt::NoModifier)  && (event->buttons() == Qt::MidButton))  ||
        ((event->modifiers() == Qt::AltModifier) && (event->buttons() == Qt::LeftButton)) )
    {
        clicPos = mapToScene(event->pos());
        return;
    }

    //this moves the block, or creates a link:
    if( (event->modifiers() == Qt::NoModifier) && (event->buttons() == Qt::LeftButton) )
    {
        auto[block,port] = getBlockAndPortUnderMouse(event->pos());
        if( block )
        {
            //Move the clicked block to the top of the screen
            moveBlockToFront(block);
            if( port )
            {
                QPointF linkPos = block->getPortConnectionPoint(*port);

                //The user is drawing a link
                drawingLink = !drawingLink;

                //now the port will be connected, and the blocks needs
                //to be updated so the graphicals changes are applied
                port->connected = true;
                block->update();

                //set the start/end point of the link in the correct position
                //whether the block is facing east or west, or the port are
                //input or output
                if( drawingLink )
                {
                    startPos = linkPos;

                    //adds the line to the scene, and to the a temporal array
                    vline = new QGraphicsLineItem;
                    //vline->setFlags( QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable );
                    QPen pen;
                    pen.setWidth(2);
                    vline->setPen(pen);
                    scene.addItem(vline);
                }
                else
                {
                    endPos = linkPos;

                    vline->setLine(QLineF(startPos,endPos));
                    vline = nullptr;
                }
            }
        }
    }
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
}

void View::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
}

void View::wheelEvent(QWheelEvent *event)
{
    qreal scaleFactor = 1.1;
    if( event->angleDelta().y() > 0.0 )
    {
        if( transform().m11() < 2.0 )
            scale( scaleFactor , scaleFactor );
    }
    else
    {
        if( transform().m11() > 0.2 )
            scale( 1.0/scaleFactor , 1.0/scaleFactor );
    }
}

void View::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
}

/**
 * @brief Corrects the mouse pos to match a valid port position
 *
 * Since mouse could be anywhere, this method corrects the mouse
 * position to a valid one (that is properly located inside the
 * port's shape, taking into account the block orientation).
 */
void View::syncLinkPosToBlockPort(const Block *block,
                                  const Block::Port *port,
                                  const QPointF& mousePos,
                                  QPointF &targetPos) const
{
    (void)mousePos;
    if( port->dir == Block::PortDir::Input )
    {
        if( block->getBlockOrientation() == Block::BlockOrientation::East )
            targetPos.setX(port->connectorShape.boundingRect().width());
        else
            targetPos.setX(0);
    }
    else
    {
        if( block->getBlockOrientation() == Block::BlockOrientation::East )
            targetPos.setX(0);
        else
            targetPos.setX(port->connectorShape.boundingRect().width());
    }
    targetPos.setY(port->connectorShape.boundingRect().height()/2.0);
    targetPos = mapToScene(targetPos.x(),targetPos.y());
    //    targetPos = nextGridPosition(mousePos-targetPos,StyleGrid::gridSize);
}

std::tuple<Block *, Block::Port *> View::getBlockAndPortUnderMouse(const QPoint &mousePos) const
{
    auto itemlist = items(mousePos);
    for( auto item : itemlist )
    {
        Block *block = dynamic_cast<Block*>(item);
        if( block )
        {
            bool isMouseOver = block->isMouseOverBlock(mapToBlock(block,mousePos));
            if( isMouseOver )
                return {block,nullptr};
            else
            {
                Block::Port* port = block->isMouseOverPort(mapToBlock(block,mousePos));
                if( port )
                    return {block,port};
            }
        }
    }
    return {nullptr,nullptr};
}

QPointF View::mapToBlock(const Block *block, const QPoint &mousePos) const
{
    return mapToScene(mousePos) - block->pos();
}

void View::moveBlockToFront(Block *block) const
{
    auto itemlist = items();
    int maxZ = itemlist.size()-1;
    block->setZValue(maxZ);
    int z = 0;
    for( auto item = itemlist.end()-1 ; item >= itemlist.begin() ; --item )
        if( *item != block )
            (*item)->setZValue(z++);
}

} // namespace GuiBlocks
