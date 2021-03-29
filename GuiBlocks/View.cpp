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
    case 4:
        {
            static int blockIdx = 0;
            block = new Block("OUT","Out "+QString::number(blockIdx++));
            block->addPort(Block::PortDir::Output,"Float","Out");
            countType++;
        }
        break;
    case 5:
        {
            static int blockIdx = 0;
            block = new Block("IN","In "+QString::number(blockIdx++));
            block->addPort(Block::PortDir::Input,"Double","In");
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
    {
        debug.lastBlock = block;
        scene.addItem( block );
    }
}

void View::flipLastBlock()
{
    debug.lastBlock->toggleBlockOrientation();
    update();
}

void View::showCurrentLinkData()
{
    uiSM.showCurrentLinkData();
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
    emit updateCoords(mapToScene(event->pos()));
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
//    if( event->key() == Qt::Key::Key_S )
//        if( links.size() > 0 )
//        {
//            qDebug() << "+ + + + + + + + + +";
//            links[links.size()-1]->show();
//            qDebug() << "+ + + + + + + + + +";
//        }
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    //if( (event->modifiers() == Qt::NoModifier) && (event->button() == Qt::LeftButton) )
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

//----------------------------------------------
//------ View::UserInterfaceStateMachine  ------
/* TO FIX:
 * TODO:
 * 0- [done] Avoid selfconnection
 * 1- Cancel key
 * 2- Auto start line when ending the last
 */
void View::UserInterfaceStateMachine::mousePress(QMouseEvent *event)
{
    if( st == States::waitPress )
    {
        if( event->button() != Qt::LeftButton )
            return;
        //stores the start pos to undo the action if a cancel operation is requested
        auto pos = nextGridPosition(parent->mapToScene(event->pos()),StyleGrid::gridSize);
        startPos = pos;
        prevPos = pos;
        activeItem = getItemUnderMouse(event->pos(),false);
        if( activeItem )
            switch( static_cast<ActiveItemIdx>(activeItem.value().index()) )
            {
                case ActiveItemIdx::PortIdx:
                    break;
                case ActiveItemIdx::LinkIdx:
                    lastLink = std::get<ActiveItemIdx::LinkIdx>(activeItem.value());
                    break;
                case ActiveItemIdx::BlockIdx:
                    parent->moveBlockToFront(std::get<ActiveItemIdx::BlockIdx>(activeItem.value()));
                    break;
            }
        st = States::triggerAction;
        return;
    }
    if( st == States::updateEndPoint )
    {
        if( event->button() == Qt::LeftButton )
        {
            if( !activeItem )
                qDebug() << "********************** this message represents a THROW (uiSM->mousePress->updateEndPoint) [activeItem should not be empty]";
            if( static_cast<ActiveItemIdx>(activeItem.value().index()) != ActiveItemIdx::LinkIdx )
                qDebug() << "********************** this message represents a THROW (uiSM->mousePress->updateEndPoint) [activeItem should be a link]";

            auto &src_link = std::get<ActiveItemIdx::LinkIdx>(activeItem.value());
            src_link->simplifyLastInsertedLine();

            auto pos = nextGridPosition(parent->mapToScene(event->pos()),StyleGrid::gridSize);

            //avoid self connexion
            const auto &links = getLinksUnderMouse(event->pos());
            if( links.size() != 0 )
            {
                if( !links[0]->isPosOnlyEndPoint(pos) )
                {
                    links[0]->removeLastInsertedLine();
                    parent->debug.activeItem = activeItem;  //for debug
                    activeItem.reset();
                    st = States::waitRelease;
                    return;
                }
            }
            //check if is trying to connect to another link
            if( links.size() > 1 )
            {
                qDebug() << "connection to another link";
                parent->debug.activeItem = activeItem;  //for debug
                activeItem.reset();
                st = States::waitRelease;
                return;
//                auto &dest_link  = std::get<ActiveItemIdx::LinkIdx>(link.value());
//                src_link->joinLink(dest_link);
//                parent->scene.removeItem(src_link);
//                for( size_t idx=0 ; idx<parent->links.size() ; ++idx )
//                    if( parent->links[idx] == src_link )
//                        parent->links.erase(parent->links.begin()+idx);
            }
            parent->debug.activeItem = activeItem;  //for debug
//            activeItem.reset();
//            st = States::waitRelease;
            st = States::startNewLink;
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
        if( event->button() == Qt::RightButton )
            switchLinkPath();
        return;
    }
}

void View::UserInterfaceStateMachine::mouseMove(QMouseEvent *event)
{
    if( st == States::startNewLink )
    {
        auto pos = nextGridPosition(parent->mapToScene(event->pos()),StyleGrid::gridSize);
        if( prevPos == pos )  //move in ?
            return; //move in -> return
        else
        {
            //move out:
            if( !activeItem )
            {
                //start a new line not connected
                activeItem = new Link(prevPos);
                auto &link = std::get<ActiveItemIdx::LinkIdx>(activeItem.value());
                link->insertLineAt(prevPos,pos,linkPath);
                parent->links.push_back(link);
                parent->scene.addItem(link);
                lastLink = link;
            }
            else
            {
                switch( static_cast<ActiveItemIdx>(activeItem.value().index()) )
                {
                    case ActiveItemIdx::LinkIdx:
                        {
                            auto &link = std::get<ActiveItemIdx::LinkIdx>(activeItem.value());
                            link->insertLineAt(prevPos,pos,linkPath);
                            lastLink = link;
                            //link->insertLine(prevPos,pos,linkPath);
                        }
                        break;
                    case ActiveItemIdx::PortIdx:
                        {
                            auto port = std::get<ActiveItemIdx::PortIdx>(activeItem.value());
                            port->connected = true;
                            auto link = new Link(prevPos);
                            link->appendPort(port);
                            activeItem = link;
                            lastLink = link;
                            link->insertLineAt(prevPos,pos,linkPath);
                            parent->links.push_back(link);
                            parent->scene.addItem(link);
                        }
                        break;
                    default:
                        qDebug() << "********************** this message represents a THROW (uiSM->mouseMove) [startNewLink should not be reached with activeItem different to Link and Port]]";
                        break;
                }
            }
        }
        prevPos = pos;
        st = States::updateEndPoint;
        return;
    }
    if( st == States::updateEndPoint )
    {
        auto pos = nextGridPosition(parent->mapToScene(event->pos()),StyleGrid::gridSize);
        if( prevPos == pos )    //move in?
            return; //move in -> return
        else
        {
            prevPos = pos;
            updateActiveLine(pos);  //move out:
        }
        return;
    }
    if( st == States::triggerAction )
    {
        auto pos = nextGridPosition(parent->mapToScene(event->pos()),StyleGrid::gridSize);
        if( prevPos == pos )    //move in?
            return; //move in -> return

        prevPos = pos;
        //if no item under cursor, then a selection rectangle is started
        if( !activeItem )
        {
            //clears the shape
            selectionShape = QPainterPath();
            selectionShape.moveTo(startPos);
            selectionShape.addRect(QRectF(startPos,pos));
            if( selectionShapePtr == nullptr )
            {
                selectionShapePtr = parent->scene.addPath(selectionShape);
                QBrush brush(QBrush(StyleSelection::normalFillColor,StyleSelection::fillStyle));
                QPen   pen(QBrush(StyleSelection::normalFillColor),StyleSelection::width,StyleSelection::normalLine);
                selectionShapePtr->setPen(pen);
                selectionShapePtr->setBrush(brush);
                selectionShapePtr->setOpacity(StyleSelection::opacity);
            }
            selectionShapePtr->setPath(selectionShape);
            selectionShapePtr->update();
            st = States::drawSelectionRect;
            return;
        }
        //a link, a block or a port is under mouse:
        auto &item_ptr = activeItem.value();
        switch( item_ptr.index() )  //not completed
        {
            case ActiveItemIdx::PortIdx:  //Port*  -> nothing
            case ActiveItemIdx::BlockIdx: //Block* -> nothing
                parent->debug.activeItem = activeItem;
                activeItem.reset();
                st = States::waitRelease;
                return;
            case ActiveItemIdx::LinkIdx:  //Link* -> move link
                {
                    //identify the position where the line was grabbed
                    auto link = std::get<ActiveItemIdx::LinkIdx>(item_ptr);
                    lastLink = link;
                    link->selectAreaNearestItem(parent->mapToScene(event->pos()));
                    //link->getGrabbedIndexs(parent->mapToScene(event->pos()));
                    QPainterPath shape;
                    QRectF rect;
                    rect.setWidth (qreal(StyleGrid::gridSize));
                    rect.setHeight(qreal(StyleGrid::gridSize));
                    rect.moveCenter(startPos);
                    shape.addRect(rect);

                    auto pos = nextGridPosition(parent->mapToScene(event->pos()),StyleGrid::gridSize);
                    link->displaceSelectedArea(pos-startPos);
                }
                st = States::moveLine;
                return;
            default:
                qDebug() << "********************** this message represents a THROW (uiSM->mouseRelease) [variant value without switch handler]";
                break;
        }
        return;
    }
    if( st == States::moveLine )
    {
        auto pos = nextGridPosition(parent->mapToScene(event->pos()),StyleGrid::gridSize);
        auto link = std::get<ActiveItemIdx::LinkIdx>(activeItem.value());
        link->displaceSelectedArea(pos-prevPos);
        prevPos = pos;
        return;
    }
    if( st == States::drawSelectionRect )
    {
        auto pos = nextGridPosition(parent->mapToScene(event->pos()),StyleGrid::gridSize);
        selectionShape = QPainterPath();
        selectionShape.moveTo(startPos);
        selectionShape.addRect(QRectF(startPos,pos));
        selectionShapePtr->setPath(selectionShape);
        selectionShapePtr->update();
        return;
    }
}

void View::UserInterfaceStateMachine::mouseRelease(QMouseEvent *event)
{
    if( st == States::triggerAction )
    {
        if( activeItem )
        {
            switch( static_cast<ActiveItemIdx>(activeItem.value().index()) )
            {
                case ActiveItemIdx::PortIdx:
                    break;
                case ActiveItemIdx::LinkIdx:
                    break;
                case ActiveItemIdx::BlockIdx:
                    parent->debug.activeItem = activeItem;
                    activeItem.reset();
                    st = States::waitPress;
                    return;
            }
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
    if( st == States::moveLine )
    {
        auto link = std::get<ActiveItemIdx::LinkIdx>(activeItem.value());
        link->simplifySelectedArea();

        parent->debug.activeItem = activeItem;
        activeItem.reset();
        st = States::waitPress;
        return;
    }
    if( st == States::drawSelectionRect )
    {
        st = States::waitPress;
        return;
    }
}

void View::UserInterfaceStateMachine::keyPress(QKeyEvent *event)
{
    if( event->key() == Qt::Key::Key_Escape )
    {
        //cancel action (link drawing, selection area or block move)
        if( static_cast<ActiveItemIdx>(activeItem.value().index()) == ActiveItemIdx::LinkIdx )
        {
            auto link = std::get<ActiveItemIdx::LinkIdx>(activeItem.value());
            if( st == States::moveLine )
                link->displaceSelectedArea(startPos-prevPos);
            if( st == States::updateEndPoint )
            {
                link->removeLastInsertedLine();
                if( link->isEmpty() )
                    removeLinkFromScene();
            }
            activeItem.reset();
            st = States::waitPress;
        }
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

void View::UserInterfaceStateMachine::showCurrentLinkData() const
{
    qDebug() << "showCurrentLinkData() ----------------- ";
    if( parent->debug.activeItem )
        if( static_cast<ActiveItemIdx>(parent->debug.activeItem.value().index()) == ActiveItemIdx::LinkIdx )
            std::get<ActiveItemIdx::LinkIdx>(parent->debug.activeItem.value())->showRawData();
    qDebug() << "--------------------------------------- ";
}

std::optional<std::variant<Block::Port*,Block*,Link*>>
View::UserInterfaceStateMachine::getItemUnderMouse(const QPoint &mousePos,
                                                   bool gridPosition) const
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
            if( gridPosition )
            {
                if( link->isPartOfLink(nextGridPosition(parent->mapToScene(mousePos),StyleGrid::gridSize)) /*link->containsPoint(parent->mapToScene(mousePos))*/ )
                    return link;
            }
            else
            {
                QRectF rect;
                rect.setHeight(StyleGrid::gridSize);
                rect.setWidth(StyleGrid::gridSize);
                rect.moveCenter(parent->mapToScene(mousePos));
                if( link->isPartOfLink(rect) )
                    return link;
            }
        }
    }
    return {};
}

std::vector<Link*> View::UserInterfaceStateMachine::getLinksUnderMouse(const QPoint &mousePos,bool gridPosition) const
{
    qDebug() << parent->mapToScene(mousePos);
    auto items = parent->scene.items(parent->mapToScene(mousePos));
    qDebug() << "items.size()" << items.size();
    std::vector<Link*> links;
    for( auto& item : items )
        if( auto link = dynamic_cast<Link*>(item) )
        {
            if( gridPosition )
            {
                if( link->isPartOfLink(nextGridPosition(parent->mapToScene(mousePos),StyleGrid::gridSize)) /*link->containsPoint(parent->mapToScene(mousePos))*/ )
                    links.push_back(link);
            }
            else
            {
                if( link->isPartOfLink( parent->mapToScene(mousePos)) )
                    links.push_back(link);
            }
        }
    return links;
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
//    link->updateLastLine(pos,linkPath);
    link->updateLastInsertedLine(pos,linkPath);
}

void View::UserInterfaceStateMachine::removeLinkFromScene()
{
    //if the activeItem is a line, then it will be removed from parent
    if( activeItem )
        switch( static_cast<ActiveItemIdx>(activeItem.value().index()) )
        {
            case ActiveItemIdx::PortIdx:
                break;
            case ActiveItemIdx::LinkIdx:
            {
                auto link = std::get<ActiveItemIdx::LinkIdx>(activeItem.value());
                parent->scene.removeItem(link);
                for( auto idx=parent->links.size() ; idx>0 ; idx--)
                    if( parent->links[idx] == link )
                    {
                        parent->links.erase(parent->links.begin()+idx);
                        delete link;
                        break;
                    }
                activeItem.reset();
                if( lastLink == link )
                    lastLink = nullptr;
                break;
            }
            case ActiveItemIdx::BlockIdx:
                break;
        }
}

} // namespace GuiBlocks
