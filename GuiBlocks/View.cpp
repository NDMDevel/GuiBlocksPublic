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
      uiSM(this)
{
    //setMouseTracking(true);
    //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setOptimizationFlag(QGraphicsView::DontSavePainterState);
    setRenderHints(QPainter::Antialiasing |
                   QPainter::SmoothPixmapTransform);

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
               qreal(StyleGrid::fineGridWidth));

    painter->setPen(pfine);
    drawGrid(qreal(StyleGrid::gridSize));

    QPen p(StyleGrid::coarseGridColor,
           qreal(StyleGrid::coarseGridWidth));

    painter->setPen(p);
    drawGrid(10.0*qreal(StyleGrid::gridSize));
}

void View::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);

    //This apply panView: middle mouse btn, or Alt + Left Click
    if( ((event->modifiers() == Qt::NoModifier)  && (event->buttons() == Qt::MiddleButton))  ||
        ((event->modifiers() == Qt::AltModifier) && (event->buttons() == Qt::LeftButton)) )
    {
        panViewClicPos = mapToScene(event->pos());
        return;
    }

    uiSM.mousePress(event);
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    //this apply panView
    if( ((event->modifiers() == Qt::NoModifier)  && (event->buttons() == Qt::MiddleButton)) ||
        ((event->modifiers() == Qt::AltModifier) && (event->buttons() == Qt::LeftButton)) )
    {
        QPointF difference = panViewClicPos - mapToScene(event->pos());
        setSceneRect(sceneRect().translated(difference.x(), difference.y()));
    }

    uiSM.mouseMove(event);
}

void View::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
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
    uiSM.keyPress(event);
    if( event->key() == Qt::Key::Key_S )
        if( links.size() > 0 )
        {
            qDebug() << "+ + + + + + + + + +";
            links[links.size()-1]->show();
            qDebug() << "+ + + + + + + + + +";
        }
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    if( (event->modifiers() == Qt::NoModifier) && (event->button() == Qt::LeftButton) )
        uiSM.mouseRelease(event);
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
//void View::syncLinkPosToBlockPort(const Block *block,
//                                  const Block::Port *port,
//                                  const QPointF& mousePos,
//                                  QPointF &targetPos) const
//{
//    (void)mousePos;
//    if( port->dir == Block::PortDir::Input )
//    {
//        if( block->getBlockOrientation() == Block::BlockOrientation::East )
//            targetPos.setX(port->connectorShape.boundingRect().width());
//        else
//            targetPos.setX(0);
//    }
//    else
//    {
//        if( block->getBlockOrientation() == Block::BlockOrientation::East )
//            targetPos.setX(0);
//        else
//            targetPos.setX(port->connectorShape.boundingRect().width());
//    }
//    targetPos.setY(port->connectorShape.boundingRect().height()/2.0);
//    targetPos = mapToScene(int(targetPos.x()),
//                           int(targetPos.y()));
//    //    targetPos = nextGridPosition(mousePos-targetPos,StyleGrid::StyleGrid::gridSize);
//}

void View::moveBlockToFront(Block *block) const
{
    auto itemlist = items();
    auto maxZ = itemlist.size()-1;
    block->setZValue(qreal(maxZ));
    auto z = 0;
    for( auto item = itemlist.end()-1 ; item >= itemlist.begin() ; --item )
        if( *item != block )
            (*item)->setZValue(z++);
}

QPointF View::mapToBlock(const Block *block, const QPoint &mousePos) const
{
    return mapToScene(mousePos) - block->pos();
}

//--------------------------------------
//------ View::MouseStateMachine  ------
void View::UserInterfaceStateMachine::mousePress(QMouseEvent *event)
{
    if( st == States::waitPress )
    {
        if( event->button() != Qt::LeftButton )
            return;
        //stores the start pos to undo the action if a cancel operation is requested
        auto pos = nextGridPosition(parent->mapToScene(event->pos()),StyleGrid::gridSize);
        startOrPrevMousePos = pos;
        activeItem = getItemUnderMouse(event->pos());
        st = States::triggerAction;
        return;
    }
    if( st == States::updateEndPoint )
    {
        if( event->button() == Qt::LeftButton )
        {
            activeItem.reset();
            st = States::waitRelease;
            return;
        }
        if( event->button() == Qt::RightButton )
        {
            auto pos = nextGridPosition(parent->mapToScene(event->pos()),StyleGrid::gridSize);
            switchLinkPath();
            updateActiveLine(pos);
            return;
        }
        return;
    }
    if( st == States::startNewLink )
    {
        if( event->button() == Qt::LeftButton )
        {
            activeItem.reset();
            st = States::waitRelease;
            return;
        }
        if( event->button() == Qt::RightButton )
            switchLinkPath();
        return;
    }
}

void View::UserInterfaceStateMachine::mouseMove(QMouseEvent *event) noexcept
{
    if( st == States::startNewLink )
    {
        auto pos = nextGridPosition(parent->mapToScene(event->pos()),StyleGrid::gridSize);
        if( startOrPrevMousePos == pos )  //move in ?
            return; //move in -> return
        else
        {
            //move out:
            if( !activeItem )
            {
                //start a new line not connected
                activeItem = new Link(startOrPrevMousePos);
                auto &link = std::get<ActiveItemIdx::LinkIdx>(activeItem.value());
//                link->insertLine(startOrPrevMousePos,pos,linkPath);
                link->appendLine(pos,linkPath);
                parent->links.push_back(link);
                parent->scene.addItem(link);
            }
            else
            {
                switch( static_cast<ActiveItemIdx>(activeItem.value().index()) )
                {
                    case ActiveItemIdx::LinkIdx:
                        {
                            auto &link = std::get<ActiveItemIdx::LinkIdx>(activeItem.value());
                            link->insertLine(startOrPrevMousePos,pos,linkPath);
                        }
                        break;
                    case ActiveItemIdx::PortIdx:
                        break;
                    default:
                        qDebug() << "********************** this message represents a THROW (uiSM->mouseMove) [startNewLink should not be reached with activeItem different to Link and Port]]";
                        break;
                }
            }
        }
        startOrPrevMousePos = pos;
        st = States::updateEndPoint;
        return;
    }
    if( st == States::updateEndPoint )
    {
        auto pos = nextGridPosition(parent->mapToScene(event->pos()),StyleGrid::gridSize);
        if( startOrPrevMousePos == pos )    //move in?
            return; //move in -> return
        else
        {
            startOrPrevMousePos = pos;
            updateActiveLine(pos);  //move out:
        }
        return;
    }
    if( st == States::triggerAction )
    {
        auto pos = nextGridPosition(parent->mapToScene(event->pos()),StyleGrid::gridSize);
        if( startOrPrevMousePos == pos )    //move in?
            return; //move in -> return

        //if no item under cursor, then a new link is started
        if( !activeItem )
        {
            st = States::startNewLink;
            return;
        }
        //a link, a block or a port is under mouse:
        auto &item_ptr = activeItem.value();
        switch( item_ptr.index() )  //not completed
        {
            case 0: //Port*
                break;
            case 1: //Block* -> nothing
                activeItem.reset();
                st = States::waitPress;
                return;
            case 2: //Link* -> start new link
                qDebug() << "over a link";
                //auto &link = std::get<ItemIdx::LinkIdx>(item_ptr);
                //link->appendLine(event->pos(),event->pos(),linkPath);
                return;
            default:
                qDebug() << "********************** this message represents a THROW (uiSM->mouseRelease) [variant value without switch handler]";
                break;
        }
        return;
    }
}

void View::UserInterfaceStateMachine::mouseRelease(QMouseEvent *event)
{
    //auto pos = nextGridPosition(parent->mapToScene(event->pos()),StyleGrid::gridSize);
    if( st == States::triggerAction )
    {
        if( activeItem )
            if( activeItem.value().index() == ActiveItemIdx::BlockIdx )
            {
                activeItem.reset();
                st = States::waitPress;
            }
        st = States::startNewLink;
        return;
    }
    if( st == States::waitRelease )
    {
        if( event->button() == Qt::LeftButton )
        {
            st = States::waitPress;
            return;
        }
        return;
    }
}

void View::UserInterfaceStateMachine::keyPress(QKeyEvent *event)
{
    if( event->key() == Qt::Key::Key_Escape )
    {
        //cancel action (link drawing, selection area or block move)
    }
}

void View::UserInterfaceStateMachine::switchLinkPath()
{
    switch( linkPath )
    {
    case Link::LinkPath::straight:
        linkPath = Link::LinkPath::verticalThenHorizontal;
        break;
    case Link::LinkPath::verticalThenHorizontal:
        linkPath = Link::LinkPath::horizontalThenVertical;
        break;
    case Link::LinkPath::horizontalThenVertical:
        linkPath = Link::LinkPath::straightThenOrthogonal;
        break;
    case Link::LinkPath::straightThenOrthogonal:
        linkPath = Link::LinkPath::orthogonalThenStraight;
        break;
    case Link::LinkPath::orthogonalThenStraight:
        linkPath = Link::LinkPath::straight;
        break;
    }
}

std::optional<std::variant<Block::Port*,Block*,Link*>>
View::UserInterfaceStateMachine::getItemUnderMouse(const QPoint &mousePos) const
{
    auto items = parent->scene.items(parent->mapToScene(mousePos));
    //The top item will be retrieved in the following order:
    //1- block port
    //2- block (dragArea)
    //3- link
    //This means that if under the mousePos there are multiple objects of interest,
    //it will first return the block port, if there is not block port, then a block
    //and finally a link.
    for( auto& item : items )
    {
        Block *block = dynamic_cast<Block*>(item);
        if( block )
        {
            Block::Port* port = block->isMouseOverPort(parent->mapToBlock(block,mousePos));
            if( port )
                return port;
            if( block->isMouseOverBlock(parent->mapToBlock(block,mousePos)) )
                return block;
        }
        if( auto link = dynamic_cast<Link*>(item) )
        {
            if( link->containsPoint(parent->mapToScene(mousePos)) )
                return link;
        }
    }
    return {};
}

void View::UserInterfaceStateMachine::updateActiveLine(const QPointF &pos)
{
    if( !activeItem )
    {
        //this should never be reached, is place here to allow the compiler to remove
        //the exception handling code
        qDebug() << "********************** this message represents a THROW (uiSM->UpdateEndPoint) [optional without value]";
        return;
    }
    if( activeItem.value().index() != ActiveItemIdx::LinkIdx )
    {
        qDebug() << "********************** this message represents a THROW (uiSM->UpdateEndPoint) [variant with incorrent type (should be a Link*)]";
        return;
    }
    auto &link = std::get<ActiveItemIdx::LinkIdx>(activeItem.value());
    link->updateLastLine(pos,linkPath);
}

//--------------------------------------
//------ View::LinkStateMachine  -------
void View::LinkStateMachine::setActiveLink(Link *link)
{
    this->link = link;
    st = States::waittingPress;
}

void View::LinkStateMachine::mousePress(const QPointF &pos)
{
    if( st == States::waittingPress )
    {
        prevPos = pos;
        firstPos = pos;
//        link->setActive(prevPos);
        st = States::validatePress;
        return;
    }
    if( st == States::waittingMove )
    {
        st = States::waittingRelease;
        return;
    }
    if( st == States::update )
    {
        st = States::waittingRelease;
        return;
    }
}

void View::LinkStateMachine::mouseMove(const QPointF &pos)
{
    if( st == States::validatePress )
    {
        if( prevPos != pos )
        {
            prevPos = pos;
//            link->updateActive(pos);
            st = States::drag;
        }
        return;
    }
    if( st == States::drag )
    {
        if( prevPos != pos )
        {
            prevPos = pos;
//            link->updateActive(pos);
        }
        return;
    }
    if( st == States::waittingMove )
    {
        if( prevPos != pos)
        {
//            prevPos = pos;
//            link->appendPoint(pos);
//            link->setActive(pos);
//            appendLine(pos);
            st = States::update;
        }
        return;
    }
    if( st == States::update )
    {
        if( prevPos != pos )
        {
            prevPos = pos;
//            if( auto midp = computeMidPoint(pos) )
//                link->updateActiveParent(midp.value());
//            link->updateActive(pos);
        }
        return;
    }
}

void View::LinkStateMachine::mouseRelease(const QPointF &)
{
    if( st == States::drag )
    {
        setActiveLink(nullptr);
        st = States::waittingPress;
        return;
    }
    if( st == States::validatePress )
    {
        st = States::waittingMove;
        return;
    }
    if( st == States::waittingRelease )
    {
        st = States::waittingPress;
        return;
    }
}

void View::LinkStateMachine::cancelDraw()
{
    if( st == States::drag )
    {
//        link->updateActive(firstPos);
        setActiveLink(nullptr);
        st = States::waittingPress;
        return;
    }
    if( st == States::update )
    {
//        link->updateActive(firstPos);
//        link->removeLastLine();
        setActiveLink(nullptr);
//        st = States::waittingRelease;
        st = States::waittingPress;
        return;
    }
}

void View::LinkStateMachine::switchLinkPath()
{
    switch( linkPath )
    {
    case Link::LinkPath::straight:
        linkPath = Link::LinkPath::verticalThenHorizontal;
        break;
    case Link::LinkPath::verticalThenHorizontal:
        linkPath = Link::LinkPath::horizontalThenVertical;
        break;
    case Link::LinkPath::horizontalThenVertical:
        linkPath = Link::LinkPath::straightThenOrthogonal;
        break;
    case Link::LinkPath::straightThenOrthogonal:
        linkPath = Link::LinkPath::orthogonalThenStraight;
        break;
    case Link::LinkPath::orthogonalThenStraight:
        linkPath = Link::LinkPath::straight;
        break;
    }
}

//void View::LinkStateMachine::appendLine(const QPointF &pos)
//{
//    prevPos = pos;
//    if( auto midp = computeMidPoint(pos) )
//    {
////        auto pidx = link->addPoint(midp.value());
////        auto idx  = link->addPoint(pos);
//    }
//    else
//        link->addPoint(pos);
//    link->setActive(pos);
//}

//std::optional<QPointF> View::LinkStateMachine::computeMidPoint(const QPointF &pos)
//{
//    QPointF midp;
////    auto[idx,pidx] = link->getActiveAndParent();
////    auto pidx = link->getActiveParentIdx();
//    auto p1 = (*link)[pidx];
//    switch( linkPath )
//    {
//        case LinkPath::straight:
//            return {};
//        case LinkPath::verticalThenHorizontal:
//            midp = QPointF( p1.x() , pos.y() );
//            break;
//        case LinkPath::horizontalThenVertical:
//            midp = QPointF( pos.x() , p1.y() );
//            break;
//        case LinkPath::straightThenOrthogonal:
//            {
//                auto diffp = p1-pos;
//                auto diffx = std::abs(diffp.x());
//                auto diffy = std::abs(diffp.y());
//                if( diffx < diffy )
//                {
//                    if( pos.y() > p1.y() )
//                        midp = QPointF( pos.x() , p1.y()+diffx);
//                    else
//                        midp = QPointF( pos.x() , p1.y()-diffx);
//                }
//                else
//                {
//                    if( pos.x() > p1.x() )
//                        midp = QPointF( p1.x()+diffy , pos.y() );
//                    else
//                        midp = QPointF( p1.x()-diffy , pos.y() );
//                }
//            }
//            break;
//        case LinkPath::orthogonalThenStraight:
//            {
//                auto diffp = p1-pos;
//                auto diffx = std::abs(diffp.x());
//                auto diffy = std::abs(diffp.y());
//                if( diffx <= diffy )
//                {
//                    if( pos.y() < p1.y() )
//                        midp = QPointF( p1.x() , pos.y()+diffx);
//                    else
//                        midp = QPointF( p1.x() , pos.y()-diffx);
//                }
//                else
//                {
//                    if( pos.x() < p1.x() )
//                        midp = QPointF( pos.x()+diffy , p1.y() );
//                    else
//                        midp = QPointF( pos.x()-diffy , p1.y() );
//                }
//            }
//            break;
//    }
//    return midp;
//}

//void View::LinkStateMachine::removeLink()
//{
////    if( link )
////        link->removeLastLine();
//}

} // namespace GuiBlocks
