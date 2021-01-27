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
      scene(parent)
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

    QGraphicsView::setScene(&scene);
}

void View::addBlock()
{
    static int countType = 0;
    Block *block = nullptr;
    switch(countType)
    {
    case 0:
        {
            static int blockIdx = 0;
            block = new Block("FIR","LowPassFilter "+QString::number(blockIdx++));
            block->addPort(Block::PortDir::Input,"Double","In");
            block->addPort(Block::PortDir::Input,"Int","Order");
            block->addPort(Block::PortDir::Output,"Double","Out");
            countType++;
        }
        break;
    case 1:
        {
            static int blockIdx = 0;
            block = new Block("Downsampler","Fractional "+QString::number(blockIdx++));
            block->addPort(Block::PortDir::Input,"Float","Sample");
            block->addPort(Block::PortDir::Input,"Int","N");
            block->addPort(Block::PortDir::Input,"Float","SampleFrequency");
            block->addPort(Block::PortDir::Output,"Float","Output");
            countType++;
        }
        break;
    case 2:
        {
            static int blockIdx = 0;
            block = new Block("PI","Pi "+QString::number(blockIdx++));
            block->addPort(Block::PortDir::Input,"Float","In");
            block->addPort(Block::PortDir::Output,"Float","Out");
            countType++;
        }
        break;
    case 3:
        {
            static int blockIdx = 0;
            block = new Block("PID","Pid "+QString::number(blockIdx++));
            block->addPort(Block::PortDir::Input ,"Float","In 1");
            block->addPort(Block::PortDir::Input ,"Float","In 2");
            block->addPort(Block::PortDir::Input ,"Float","In 3");
            block->addPort(Block::PortDir::Input ,"Float","In 4");
            block->addPort(Block::PortDir::Input ,"Float","In 5");
            block->addPort(Block::PortDir::Output,"Float","Out 1");
            block->addPort(Block::PortDir::Output,"Float","Out 2");
            block->addPort(Block::PortDir::Output,"Float","Out 3");
            block->addPort(Block::PortDir::Output,"Float","Out 4");
            block->addPort(Block::PortDir::Output,"Float","Out 5");
            countType++;
        }
        break;
    default:
        {
            static int blockIdx = 0;
            block = new Block("Threshold","OverVoltage "+QString::number(blockIdx++));
            block->addPort(Block::PortDir::Input,"Int","One");
            block->addPort(Block::PortDir::Input,"DQ","PLL Input");
            block->addPort(Block::PortDir::Input,"Float","Input Voltage");
            block->addPort(Block::PortDir::Output,"Double","Output Voltage");
            block->addPort(Block::PortDir::Output,"char","Letter");
            countType = 0;
        }
    }
    if( block )
        scene.addItem( block );
}

void View::showLastTwoIndexes()
{
    uint16_t last,prev;
    links[0]->getLastTwoIndexes(prev,last);
    qDebug() << "prev: " << prev << " last: " << last;
}

void View::drawBackground(QPainter* painter, const QRectF &r)
{
    QGraphicsView::drawBackground( painter , r );

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
    drawGrid(StyleGrid::gridSize);

    QPen p(StyleGrid::coarseGridColor,
           StyleGrid::coarseGridWidth);

    painter->setPen(p);
    drawGrid(10*StyleGrid::gridSize);
}

void View::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);

    //This apply panView: middle mouse btn, or Alt + Left Click
    if( ((event->modifiers() == Qt::NoModifier)  && (event->buttons() == Qt::MidButton))  ||
        ((event->modifiers() == Qt::AltModifier) && (event->buttons() == Qt::LeftButton)) )
    {
        panViewClicPos = mapToScene(event->pos());
        return;
    }

    //right clic switchs the trajectory of the link while drawing:
    if( ((event->modifiers() == Qt::NoModifier) && (event->buttons() == Qt::RightButton)) )
    {
    }

    //this moves the block, creates a link, or draw selection rectangle
    if( (event->modifiers() == Qt::NoModifier) && (event->buttons() == Qt::LeftButton) )
    {
        auto clicPos = nextGridPosition(mapToScene(event->pos()),StyleGrid::gridSize);

        //if clic over a block/port
        if( blockMousePressHandler(pointConvertion(clicPos)) )
            return;

        //if clic over a link
        if( linkMousePressHandler(clicPos) )
            return;
        else
        {
            auto l = new Link(clicPos);
            links.push_back(l);
            linkSM.setActiveLink(links.back());
            linkSM.mousePress(clicPos);
            scene.addItem(links.back());
            qDebug() << "Link address: " << links.back();
        }
        debug_msg(links.size());
        //handle here the selection rectangle:
        return;
    }
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    //this apply panView
    if( ((event->modifiers() == Qt::NoModifier)  && (event->buttons() == Qt::MidButton)) ||
        ((event->modifiers() == Qt::AltModifier) && (event->buttons() == Qt::LeftButton)) )
    {
        QPointF difference = panViewClicPos - mapToScene(event->pos());
        setSceneRect(sceneRect().translated(difference.x(), difference.y()));
    }
    if( linkSM.isReadyForMoveEvents() )
        linkSM.mouseMove(nextGridPosition(mapToScene(event->pos()),StyleGrid::gridSize));
}

void View::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if( event->buttons() == Qt::LeftButton )
    {
//        qDebug() << link[0]->boundingRect();
//        auto rect = new QGraphicsRectItem(link[0]->boundingRect());
//        scene.addItem(rect);
//        QGraphicsPathItem *pathItem = new QGraphicsPathItem();
//        pathItem->setFlag(QGraphicsItem::ItemIsMovable);
//        QPainterPath path;
//        path.moveTo(0,0);
//        path.lineTo(25,50);
//        path.lineTo(0,100);
//        pathItem->setPath(path);
//        scene.addItem(pathItem);
//        if( path.elementAt(2).isLineTo() )
//        {
//            path.setElementPositionAt(2,-50,-80);
//            pathItem->setPath(path);
//        }
    }
}

void View::keyPressEvent(QKeyEvent *event)
{
    QGraphicsView::keyPressEvent(event);
    if( event->key() == Qt::Key_Escape )
    {
        //cancel link
        linkSM.cancelDraw();
    }
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    if( (event->modifiers() == Qt::NoModifier) && (event->button() == Qt::LeftButton) )
    {
        linkMouseReleaseHandler(nextGridPosition(mapToScene(event->pos()),StyleGrid::gridSize));
    }
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
//    rect();
//    setSceneRect(QRectF(0,0,25000,25000));
//    qDebug() << this->sceneRect();
//    scene.addRect(sceneRect());
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
    //    targetPos = nextGridPosition(mousePos-targetPos,StyleGrid::StyleGrid::gridSize);
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

void View::switchLinePath()
{
    switch( linkpath )
    {
    case Link::LinkPath::straight:
        linkpath = Link::LinkPath::verticalThenHorizontal;
        break;
    case Link::LinkPath::verticalThenHorizontal:
        linkpath = Link::LinkPath::horizontalThenVertical;
        break;
    case Link::LinkPath::horizontalThenVertical:
        linkpath = Link::LinkPath::straightThenOrthogonal;
        break;
    case Link::LinkPath::straightThenOrthogonal:
        linkpath = Link::LinkPath::orthogonalThenStraight;
        break;
    case Link::LinkPath::orthogonalThenStraight:
        linkpath = Link::LinkPath::straight;
        break;
    }
}

void View::redrawActiveLink(const QPointF& pos)
{
    (void)pos;
//    if( link.size() == 0 )
//        return;
//    int index = link.size()-2;
//    auto line1 = link[index  ]->line();
//    auto line2 = link[index+1]->line();
//    QPointF midp;
//    switch( linkpath )
//    {
//    case LinkPath::straight:
//        midp = (pos+line1.p1())/2.0f; //equals to: p1+(endPos-p1)/2
//        break;
//    case LinkPath::verticalThenHorizontal:
//        midp = QPointF( line1.p1().x() , pos.y() );
//        break;
//    case LinkPath::horizontalThenVertical:
//        midp = QPointF( pos.x() , line1.p1().y() );
//        break;
//    case LinkPath::straightThenOrthogonal:
//    {
//        auto diffp = line1.p1()-pos;
//        auto diffx = std::abs(diffp.x());
//        auto diffy = std::abs(diffp.y());
//        if( diffx < diffy )
//        {
//            if( pos.y() > line1.p1().y() )
//                midp = QPointF( pos.x() , line1.p1().y()+diffx);
//            else
//                midp = QPointF( pos.x() , line1.p1().y()-diffx);
//        }
//        else
//        {
//            if( pos.x() > line1.p1().x() )
//                midp = QPointF( line1.p1().x()+diffy , pos.y() );
//            else
//                midp = QPointF( line1.p1().x()-diffy , pos.y() );
//        }
//    }
//        break;
//    case LinkPath::orthogonalThenStraight:
//    {
//        auto diffp = line1.p1()-pos;
//        auto diffx = std::abs(diffp.x());
//        auto diffy = std::abs(diffp.y());
//        if( diffx <= diffy )
//        {
//            if( pos.y() < line1.p1().y() )
//                midp = QPointF( line1.p1().x() , pos.y()+diffx);
//            else
//                midp = QPointF( line1.p1().x() , pos.y()-diffx);
//        }
//        else
//        {
//            if( pos.x() < line1.p1().x() )
//                midp = QPointF( pos.x()+diffy , line1.p1().y() );
//            else
//                midp = QPointF( pos.x()-diffy , line1.p1().y() );
//        }
//    }
//        break;
//    }
//    line1.setP2(midp);
//    line2.setP1(midp);
//    line2.setP2(pos);
//    link[index  ]->setLine(line1);
    //    link[index+1]->setLine(line2);
}

bool View::blockMousePressHandler(const QPoint &pos)
{
    //get block and/or port under mouse
    auto[block,port] = getBlockAndPortUnderMouse(pos);

    //click over a block??
    if( block != nullptr )
    {
        //Move the clicked block to the top of the screen
        moveBlockToFront(block);
        //click over a port, if so a new link is started
        //(but only if port is unconnected)
        if( port != nullptr )
        {
            //check if the port is unconnected
            return true;
        }

        //click over a block (but not over a port)
        //so the the block must be moved with the mouse
        //(since this is the default behavior nothing
        //needs to be done)
        return true;
    }
    return false;
}

bool View::linkMousePressHandler(const QPointF &pos)
{
    bool overLink = false;
    linkSM.setActiveLink(nullptr);
    for( auto& link : links )
        if( (overLink = link->containsPoint(pos)) )
        {
            linkSM.setActiveLink(link);
            linkSM.mousePress(pos);
            break;
        }
    return overLink;
}

void View::linkMouseMoveHandler(const QPointF &pos)
{
    linkSM.mouseMove(pos);
}

void View::linkMouseReleaseHandler(const QPointF &pos)
{
    linkSM.mouseRelease(pos);
}

//--------------------------------------
//------ View::LinkStateMachine  -------
void View::LinkStateMachine::setActiveLink(Link *link)
{
    this->link = link;
    st = States::waittingPress;
}

void View::LinkStateMachine::mousePress(const QPointF &point)
{
    if( st == States::waittingPress )
    {
        pos = point;
        link->appendPoint(point);
        st = States::waittingMoveOrRelease;
//        qDebug() << "waittingPress -> waittingMoveOrRelease";
        return;
    }
    if( st == States::drawingAndWaittingPress )
    {
        st = States::waittingMoveOrRelease;
        pos = point;
//        qDebug() << "drawingAndWaittingPress -> waittingPress";
        return;
    }
}

void View::LinkStateMachine::mouseMove(const QPointF &point)
{
    if( st == States::waittingMoveOrRelease )
    {
        if( pos != point ) //mouse has moved ?
        {
            if( link->length() == 1 )
            {
                link->deleteLastLine();
                st = States::waittingPress;
            }
            else
            {
                //this modify active point
                link->updateLastPoint(point);
                st = States::movingAndWaittingRelease;
            }
//            qDebug() << "waittingMoveOrRelease -> movingAndWaittingRelease";
        }
        return;
    }
    if( st == States::appendNewLine )
    {
        if( pos != point ) //mouse has moved ?
        {
            link->appendPoint(point);
            st = States::drawingAndWaittingPress;
        }
        return;
    }
    if( st == States::drawingAndWaittingPress )
    {
        link->updateLastPoint(point);
        return;
    }
    if( st == States::movingAndWaittingRelease )
    {
        link->updateLastPoint(point);
        return;
    }
}

void View::LinkStateMachine::mouseRelease(const QPointF &point)
{
    if( st == States::movingAndWaittingRelease )
    {
        link->updateLastPoint(point);
        link = nullptr;
        st = States::waittingPress;
//        qDebug() << "movingAndWaittingRelease -> waittingPress";
        return;
    }
    if( st == States::waittingMoveOrRelease )
    {
        //a new link is started
        //link->appendPoint(point);
        st = States::appendNewLine;
//        qDebug() << "waittingMoveOrRelease -> appendNewLine";
        return;
    }
}

void View::LinkStateMachine::cancelDraw()
{
    if( st == States::drawingAndWaittingPress ||
        st == States::waittingMoveOrRelease   )
    {
        st = States::waittingPress;
        link->deleteLastLine();
        link = nullptr;
    }
    if( st == States::appendNewLine )
    {
        st = States::waittingPress;
        link = nullptr;
    }
}

} // namespace GuiBlocks
