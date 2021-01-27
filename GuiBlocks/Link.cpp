#include "Link.h"

#include <QDebug>
#include <QPainter>
#include "Utils.h"
#include <cmath>

namespace GuiBlocks {

#ifdef LINK_V2
//--------------------------------------
//------ Link::VectorTree  -------------
//usually the sequence size needed is half of point size
//(in worst case, the sizes will be equals and the appends
//functions and alike will manage the allocations)
Link::VectorTree::VectorTree(uint16_t len)
    : activePointIdx(invalid_index),
      rawSize(0),
      pIdx(0),
      sIdx(0),
      pIter(0),
      sIter(0),
      repeatPrev(false),
      containerRect(),
      rawBuf(nullptr),
      points(nullptr),
      sequence(nullptr),
      seqIter({.idx=0,.count=0})
{
    if( len == 0 )
        throw("");
    //in most cases the amount of sequence
    //is half the amount the points
    rawSize = sizeof(QPointF)*len + sizeof(Sequence)*len/2;
    rawBuf  = new uint8_t[ rawSize ];
    points  = reinterpret_cast<QPointF*>(rawBuf);
    sequence  = reinterpret_cast<Sequence*>(rawBuf+rawSize-sizeof(Sequence));
}

Link::VectorTree::VectorTree(const Link::VectorTree &v)
    : activePointIdx(v.activePointIdx),
      rawSize(v.rawSize),
      pIdx(v.pIdx),
      sIdx(v.sIdx),
      pIter(v.pIter),
      sIter(v.sIter),
      repeatPrev(v.repeatPrev),
      containerRect(),
      rawBuf(nullptr),
      points(nullptr),
      sequence(nullptr),
      seqIter(v.seqIter)
{
    rawBuf = new uint8_t[rawSize];
    points  = reinterpret_cast<QPointF*>(rawBuf);
    sequence  = reinterpret_cast<Sequence*>(rawBuf+rawSize-sizeof(Sequence));
}

//Link::VectorTree::VectorTree(Link::VectorTree &&v)
//    : activePointIdx(v.activePointIdx),
//      rawSize(v.rawSize),
//      pIdx(v.pIdx),
//      sIdx(v.sIdx),
//      pIter(v.pIter),
//      sIter(v.sIter),
//      repeatPrev(v.repeatPrev),
//      containerRect(),
//      rawBuf(v.rawBuf),
//      points(v.points),
//      sequence(v.sequence),
//      seqIter(v.seqIter)
//{
//    v.rawBuf = nullptr;
//    v.rawSize = 0;
//}

Link::VectorTree::~VectorTree()
{
    if( rawBuf != nullptr )
        delete []rawBuf;

    //this should not be needed...
    rawBuf = nullptr;
    points = nullptr;
    sequence  = nullptr;
}

void Link::VectorTree::append(const QPointF &point)
{
    //if this is the first point to be appended, it will
    //be added without a sequence
    if( pIdx == 0 )
    {
        points[0] = point;
        activePointIdx = pIdx;  //now the last point added is the active one
        pIdx++;
        return;
    }
    //it the point already is part of the tree, it will
    //be set as active point (and no re-appended)
    if( setAsActiveIfExists(point) )
        return;

    //if the points is not part ot the tree, it will be appended.
    //if the tree is full it will be resized and then appended.
    if( sizeof(QPointF)*(pIdx+1)+sizeof(Sequence)*(sIdx+1) > rawSize )
        expand();
    points[pIdx] = point;
    bool appendSequence = true;
    if( auto sec = getSequence(sIdx-1) )
    {
        if( pIdx-1 == activePointIdx )
        {
            sec->count++;
            appendSequence = false;
        }
    }
    if( appendSequence )
    {
        setSequence(Sequence{ .idx=activePointIdx , .count=1} );
        sIdx++;
    }
    activePointIdx = pIdx;  //now the last point added is the active one
    pIdx++;

    if( !containerRect.contains(point) )
        updateContainerRect();
}

void Link::VectorTree::updateLast(const QPointF &point)
{
    if( pIdx > 0 )
    {
        points[activePointIdx] = point;

        if( !containerRect.contains(point) )
            updateContainerRect();
    }
}

void Link::VectorTree::getLastTwoIndexes(uint16_t &prev, uint16_t &last)
{
    if( pIdx <= 1 )
    {
        prev = invalid_index;
        last = invalid_index;
        return;
    }
    last = invalid_index;
    prev = invalid_index;
    for( uint16_t s=0 ; s<sIdx ; s++ )
    {
        auto seq = getSequence(s);
        for( uint16_t i=seq->idx ; i<seq->count ; i++ )
        {
            if( points[i] != points[activePointIdx] )
                prev = i;
            else
                goto out;
        }
    }
    out:
    if( prev != invalid_index )
        last = activePointIdx;
}

void Link::VectorTree::deleteLastPoint()
{
    if( pIdx > 0 )
    {
        pIdx--;
        auto seq = getSequence(sIdx-1);
        if( seq )
        {
            seq->count--;
            if( seq->count == 0 )
                sIdx--;
        }
    }
}

void Link::VectorTree::optimizeMem()
{
    if( sizeof(QPointF)*(pIdx)+sizeof(Sequence)*(sIdx) < rawSize )
    {
        auto newrawSize = sizeof(QPointF)*(pIdx) + sizeof(Sequence)*(sIdx);
        auto newbuf = new uint8_t[newrawSize];
        auto pPtr = reinterpret_cast<QPointF*>(newbuf);
        auto sPtr = reinterpret_cast<Sequence*>(newbuf+newrawSize-sizeof(Sequence));
        for( uint16_t i=0 ; i<pIdx ; i++ )
            pPtr[i] = points[i];
        for( uint16_t i=0 ; i<sIdx ; i++ )
            *(sPtr-i) = *(sequence-i);

        //frees mem and update ptrs
        delete []rawBuf;
        rawBuf   = newbuf;
        rawSize  = newrawSize;
        points   = pPtr;
        sequence = sPtr;
    }
}

void Link::VectorTree::optimizeSequence()
{

}

const QPointF *Link::VectorTree::iterate()
{
    if( repeatPrev )
    {
        repeatPrev = false;
        return points+pIter;
    }
    if( seqIter.count == 0 )
    {
        if( getSequence(sIter) == nullptr )
        {
            resetIterator();
            return nullptr;
        }
        seqIter = *getSequence(sIter++);
        return points + seqIter.idx;
    }
    if( seqIter.count-- > 0 )
    {
        pIter++;
        auto point = points + pIter;
        if( seqIter.count != 0 )
            repeatPrev = true;
        return point;
    }
    return nullptr;
}

void Link::VectorTree::resetIterator()
{
    repeatPrev = false;
    seqIter.count = 0;
    pIter = 0;
    sIter = 0;
}

uint16_t Link::VectorTree::findIndex(const QPointF *point)
{
    for( uint16_t i=0 ; i<pIdx ; i++ )
        if( points+i == point )
            return i;
    return invalid_index;
}

void Link::VectorTree::printAllData()
{
    qDebug("------ Raw data:");
    uint16_t pidx = 1;
    for( uint16_t i=0 ; i<sIdx ; i++ )
    {
        auto seq = getSequence(i);
        pidx += seq->count;
        qDebug() << seq->idx << "-" << seq->count << " (" << pidx << ")";
    }
    printNodes();
    printEndpoints();
}

void Link::VectorTree::printNodes()
{
    qDebug("-- Nodes:");
}

void Link::VectorTree::printEndpoints()
{
    qDebug("-- Endpoints:");
    for( uint16_t i=0 ; i<sIdx ; i++ )
    {
        auto seq = getSequence(i);
    }
}

Link::VectorTree::Sequence* Link::VectorTree::getSequence(uint16_t idx) const
{
    if( idx == invalid_index )
        return nullptr;
    if( idx < sIdx )
        return (sequence-idx);
    return nullptr;
}

void Link::VectorTree::setSequence(const Sequence &sec)
{
    *(sequence-sIdx) = sec;
}

uint16_t Link::VectorTree::find(const QPointF &point) const
{
//    qDebug() << "exists: " << point;
    for( uint16_t i=0 ; i<pIdx ; i++ )
    {
//        qDebug() << "  " << points[i];
        if( points[i] == point )
            return i;
    }
    return invalid_index;
}

bool Link::VectorTree::setAsActiveIfExists(const QPointF &point)
{
    auto idx = find(point);
    if( idx != invalid_index )
    {
        activePointIdx = idx;
        return true;
    }
    return false;
}

void Link::VectorTree::expand()
{
    auto growSize = 3;
    auto newrawSize = sizeof(QPointF)*(pIdx+growSize) + sizeof(Sequence)*(sIdx+growSize);
    auto newbuf = new uint8_t[newrawSize];
    auto pPtr = reinterpret_cast<QPointF*>(newbuf);
    auto sPtr = reinterpret_cast<Sequence*>(newbuf+newrawSize-sizeof(Sequence));
    for( uint16_t i=0 ; i<pIdx ; i++ )
        pPtr[i] = points[i];
    for( uint16_t i=0 ; i<sIdx ; i++ )
        *(sPtr-i) = *(sequence-i);

    //frees mem and update ptrs
    delete []rawBuf;
    rawBuf   = newbuf;
    rawSize  = newrawSize;
    points   = pPtr;
    sequence = sPtr;
}

void Link::VectorTree::updateContainerRect()
{
    auto ptl = points[0];
    auto pbr = points[0];
    for( uint16_t idx=1 ; idx<pIdx ; idx++ )
    {
        auto x = points[idx].x();
        auto y = points[idx].y();
        if( ptl.x() > x )
            ptl.setX(x);
        if( ptl.y() < y )
            ptl.setY(y);
        if( pbr.x() < x )
            pbr.setX(x);
        if( pbr.y() > y )
            pbr.setY(y);
    }
    ptl.setX(ptl.x()-StyleGrid::gridSize);
    ptl.setY(ptl.y()+StyleGrid::gridSize);
    pbr.setX(pbr.x()+StyleGrid::gridSize);
    pbr.setY(pbr.y()-StyleGrid::gridSize);
    containerRect = QRectF(ptl,pbr);
}

//--------------------------------------
//------ Link
Link::Link(const QPointF &startPos,LinkPath linkPath)
    : points(3)
{
    points.append(startPos);
    this->linkPath = linkPath;

//    setFlags(QGraphicsItem::ItemIsMovable);
    auto effect = new QGraphicsDropShadowEffect;
    effect->setOffset(2, 2);
    effect->setBlurRadius(15);
    effect->setColor(StyleLink::shadowColor);
    setGraphicsEffect(effect);
}

Link::Link(const Link &l)
    : QGraphicsItem(nullptr),
      points(l.points)
{
    activePort  = l.activePort;
    pasivePorts = l.pasivePorts;
    activePort  = l.activePort;
    linkPath = l.linkPath;
}

QRectF Link::boundingRect() const
{
    return points.getContainerRect();
}

void Link::paint(QPainter *painter,
                 const QStyleOptionGraphicsItem *option,
                 QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    points.resetIterator();
    painter->setPen(QPen(QBrush(StyleLink::normalColor),
                         StyleLink::width,
                         StyleLink::normalLine,
                         StyleLink::normalCap));
    painter->setFont(StyleText::blockHintFont);
    while( auto p1 = points.iterate() )
    {
        //draw line
        auto p2 = points.iterate();
        painter->drawLine(*p1,*p2);
        //draw nodes
        painter->drawEllipse(*p1,2,2);
        //draw index
        painter->save();
        painter->setPen(QPen(QBrush(Qt::red),2));
        painter->drawText(*p1+QPointF(0,0),QString::number(points.findIndex(p1)));
        painter->restore();
    }
    painter->setPen(QPen(QBrush(Qt::blue),
                         1,
                         Qt::DashLine,
                         StyleLink::normalCap));
    painter->drawRect(boundingRect());
}

#elif defined LINK_V1
Link::Link(const QPointF &pos)
{
    qDebug() << "Link() " << this;
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    setLineAttributes();

    debug.genPoints(1000);

    auto line = new QLineF(pos,pos);
    lines.push_back(line);
    line = new QLineF(pos,pos);
    lines.push_back(line);

    //remove this, added for testing
    setAcceptHoverEvents(true);
}

Link::~Link()
{
    qDebug() << "~Link() " << this;
    for( uint32_t idx = 0; idx < lines.size() ; idx++ )
        delete lines[idx];
}

QRectF Link::boundingRect() const
{
    //return QRectF(QPointF(-300,300),QPointF(300,-300));

    auto rect = shapePath.controlPointRect();
    rect.setWidth(rect.width() + 8*StyleLink::width);
    rect.setHeight(rect.height() + 8*StyleLink::width);
    rect.moveCenter(rect.center() - 4*QPointF(StyleLink::width,StyleLink::width));
    return rect;
}

void Link::paint(QPainter *painter,
                 const QStyleOptionGraphicsItem *option,
                 QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);



    //debug and tests:
    {
//        static int count = 0;
//        qDebug() << "paint()" << count++;
//        painter->setPen(QPen(QBrush(Qt::blue),0));
//        painter->drawRect(debug.rect);
//        painter->setPen(QPen(QBrush(Qt::yellow),0));
//        QRectF rect;
//        rect.setWidth(10);
//        rect.setHeight(10);
//        rect.moveCenter(debug.nearestPoint);
//        painter->drawRect(rect);
//        painter->setPen(QPen(QBrush(Qt::green),0));
//        painter->drawLine(debug.line);
//        for( auto line : debug.lines )
//            painter->drawLine(line);
        for( uint32_t idx=1 ; idx<debug.points.size() ; idx++ )
            painter->drawLine(debug.points[idx-1],debug.points[idx]);
    }

    painter->setPen(QPen(QBrush(StyleLink::normalColor),StyleLink::width));
    //if( debug.first )
    {
        for( auto line : lines)
        {
            if( debug.toggleLineColor )
                debug.pen = QPen(QBrush(Qt::red),StyleLink::width);
            else
                debug.pen = QPen(QBrush(Qt::blue),StyleLink::width);
            debug.toggleLineColor = !debug.toggleLineColor;
            painter->setPen(debug.pen);
            painter->drawLine(*line);
        }
        debug.first = false;
    }
//    else
//    {
//        for( uint32_t idx=lines.size()-2 ; idx<lines.size() ; idx++ )
//        {
//            if( debug.toggleLineColor )
//                debug.pen = QPen(QBrush(Qt::red),StyleLink::width);
//            else
//                debug.pen = QPen(QBrush(Qt::blue),StyleLink::width);
//            debug.toggleLineColor = !debug.toggleLineColor;
//            painter->setPen(debug.pen);
//            painter->drawLine(*lines[idx]);
//        }
//    }
}

void Link::updateLinkEndPos(const QPointF &pos)
{
    //if no lines to draw, or no changes in the lines, inores this call
    if( lines.size()==0 || (lastLinkPath==linkPath && lastEndPos==pos) )
        return;
    auto index = lines.size()-2;
    auto line1 = lines[index  ];
    auto line2 = lines[index+1];
    QPointF midp;
    switch( linkPath )
    {
    case LinkPath::straight:
        midp = (pos+line1->p1())/2.0f; //equals to: p1+(endPos-p1)/2
        break;
    case LinkPath::verticalThenHorizontal:
        midp = QPointF( line1->p1().x() , pos.y() );
        break;
    case LinkPath::horizontalThenVertical:
        midp = QPointF( pos.x() , line1->p1().y() );
        break;
    case LinkPath::straightThenOrthogonal:
    {
        auto diffp = line1->p1()-pos;
        auto diffx = std::abs(diffp.x());
        auto diffy = std::abs(diffp.y());
        if( diffx < diffy )
        {
            if( pos.y() > line1->p1().y() )
                midp = QPointF( pos.x() , line1->p1().y()+diffx);
            else
                midp = QPointF( pos.x() , line1->p1().y()-diffx);
        }
        else
        {
            if( pos.x() > line1->p1().x() )
                midp = QPointF( line1->p1().x()+diffy , pos.y() );
            else
                midp = QPointF( line1->p1().x()-diffy , pos.y() );
        }
    }
        break;
    case LinkPath::orthogonalThenStraight:
    {
        auto diffp = line1->p1()-pos;
        auto diffx = std::abs(diffp.x());
        auto diffy = std::abs(diffp.y());
        if( diffx <= diffy )
        {
            if( pos.y() < line1->p1().y() )
                midp = QPointF( line1->p1().x() , pos.y()+diffx);
            else
                midp = QPointF( line1->p1().x() , pos.y()-diffx);
        }
        else
        {
            if( pos.x() < line1->p1().x() )
                midp = QPointF( pos.x()+diffy , line1->p1().y() );
            else
                midp = QPointF( pos.x()-diffy , line1->p1().y() );
        }
    }
        break;
    }
    lastLinkPath = linkPath;
    lastEndPos   = pos;
    line1->setP2(midp);
    line2->setP1(midp);
    line2->setP2(pos);

    //update the shape of the link
    shapePath = QPainterPath();
    shapePath.moveTo(lines[0]->p1());
    for( uint32_t idx=0 ; idx<index ; idx++ )
        shapePath.lineTo(lines[idx]->p2());
    shapePath.lineTo(line1->p2());
    shapePath.lineTo(line2->p2());

    //informs that the shape may change
    prepareGeometryChange();
    //apply the changes
    lines[index  ] = line1;
    lines[index+1] = line2;
    //ask for a redraw
    update();
}

void Link::enableMode(Link::Mode mode)
{
    this->mode = mode;
}

//void Link::updateBoundingRect()
//{
//    if( lines.size() == 0)
//    {
//        containerRect.setRect(0,0,0,0);
//        return;
//    }
//    auto xmin = lines[0]->p1().x();
//    auto xmax = lines[0]->p2().x();
//    auto ymin = lines[0]->p1().y();
//    auto ymax = lines[0]->p2().y();
//    for( uint32_t idx=0 ; idx < lines.size() ; idx++ )
//    {
//        //mins
//        xmin = std::min(xmin,lines[idx]->p1().x());
//        xmin = std::min(xmin,lines[idx]->p2().x());
//        ymin = std::min(ymin,lines[idx]->p1().y());
//        ymin = std::min(ymin,lines[idx]->p2().y());
//        //maxs
//        xmax = std::max(xmax,lines[idx]->p1().x());
//        xmax = std::max(xmax,lines[idx]->p2().x());
//        ymax = std::max(ymax,lines[idx]->p1().y());
//        ymax = std::max(ymax,lines[idx]->p2().y());
//    }
//    prepareGeometryChange();
//    xmin -= StyleLink::width;
//    xmax += StyleLink::width;
//    ymin -= StyleLink::width;
//    ymax += StyleLink::width;
//
//    containerRect.setTopLeft(QPointF(xmin,ymax));
//    containerRect.setBottomRight(QPointF(xmax,ymin));
//}

void Link::switchLinePath()
{
    switch( linkPath )
    {
    case LinkPath::straight:
        linkPath = LinkPath::verticalThenHorizontal;
        break;
    case LinkPath::verticalThenHorizontal:
        linkPath = LinkPath::horizontalThenVertical;
        break;
    case LinkPath::horizontalThenVertical:
        linkPath = LinkPath::straightThenOrthogonal;
        break;
    case LinkPath::straightThenOrthogonal:
        linkPath = LinkPath::orthogonalThenStraight;
        break;
    case LinkPath::orthogonalThenStraight:
        linkPath = LinkPath::straight;
        break;
    }
    //since the linkpath has changed, call updateLinkEndPos
    //to reshape the link
    updateLinkEndPos(lines.back()->p2());
}

void Link::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    debug.rect.setWidth(10);
    debug.rect.setHeight(10);
    debug.rect.moveCenter(event->pos());
    if( auto ret = getTaxiDistanceAndPoint(event->pos()) )
    {
        debug.nearestPoint = std::get<0>(ret.value());
        if( std::get<1>(ret.value()) <= 75.0 )
            update();
        else
            event->ignore();
    }
    else
        event->ignore();
    QGraphicsItem::mousePressEvent(event);
}

void Link::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
//    debug_msg("mouseMoveEvent()");
    QGraphicsItem::mouseMoveEvent(event);
//    updateLinkEndPos(event->pos());
}

void Link::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
}

void Link::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItem::hoverMoveEvent(event);
    if( auto ret = getTaxiDistanceAndPoint(event->pos()) )
    {
        debug.nearestPoint = std::get<0>(ret.value());
//        qDebug() << std::get<1>(ret.value());
    }
    update();
}

void Link::setLineAttributes()
{
    setFlags(QGraphicsItem::ItemIsMovable);
    auto effect = new QGraphicsDropShadowEffect;
    effect->setOffset(2, 2);
    effect->setBlurRadius(15);
    effect->setColor(StyleLink::shadowColor);
    setGraphicsEffect(effect);
}

//bool Link::interceptLine(const QPointF& pos)
//{
//    QRectF testRect;
//    testRect.setWidth(10);
//    testRect.setHeight(10);
//    testRect.moveCenter(pos);
//    qreal tmin;
//    qreal tminPrev;
//    QLineF *targetLine = nullptr;
//    auto minDist = squareDistance(lines[0]->p1(),pos);
//    tminPrev = 0;
//    tmin = 0;
//    for( auto line : lines )
//    {
//        for( qreal t=0 ; t<=1 ; t+=0.1 )
//        {
//            //auto val = QPointF::dotProduct(line->pointAt(t),pos);
//            auto val = squareDistance(line->pointAt(t),pos);
//            if( minDist > val )
//            {
//                minDist    = val;
//                tminPrev   = tmin;
//                tmin       = t;
//                targetLine = line;
//            }
//        }
//    }
//    qDebug() << "*****************";
//    qDebug() << "tmin"      << tmin;
//    qDebug() << "tminPrev"  << tminPrev;
//    qDebug() << "minDist"   << minDist;
//    return false;
//}

std::optional<std::tuple<QPointF,double>> Link::getTaxiDistanceAndPoint(const QPointF &pos)
{
    if( lines.size() == 0 )
        return {};  //empty return
    std::tuple<QPointF,double> retval;
    std::get<1>(retval) = qInf();
    debug.lines.clear();
    for( auto line : lines )
    {
        //slope of the perpendicular line equals to
        QPointF p1( pos.x()+line->dy() , pos.y()-line->dx() );
        QPointF p2( pos.x()-line->dy() , pos.y()+line->dx() );
        QLineF pline(p1,p2);
        debug.lines.push_back(pline);
        qDebug() << "sign: " << line->dx() << line->dy();
        if( line->intersect(pline,&p1) == QLineF::BoundedIntersection )
        {
            auto dist = squareDistance(pos,p1);
            if( std::get<1>(retval) > dist )
            {
                std::get<0>(retval) = p1;
                std::get<1>(retval) = dist;
                debug.line = pline;
            }
        }
        if( line->intersect(pline,&p1) == QLineF::UnboundedIntersection )
        {
            auto dist = squareDistance(pos,line->p1());
            auto p = line->p1();
            if( dist > squareDistance(pos,line->p2()) )
            {
                dist = squareDistance(pos,line->p2());
                p = line->p2();
            }
            if( std::get<1>(retval) > dist )
            {
                std::get<0>(retval) = p;
                std::get<1>(retval) = dist;
            }
        }
    }
    return retval;
}

void GuiBlocks::Link::Debug::genPoints(int n)
{
    auto delta_ang = 2.0*M_PI/double(n);
    points.resize(n);
    auto ang = 0.0;
    while( ang < 2.0*M_PI )
    {
        auto x = 250.0 * std::sin(ang);
        auto y = 250.0 * std::cos(ang);
//        if( ang >= M_PI/2.0 && ang < M_PI ) //Quadrant II
//            x *= -1.0;
//        if( ang >= M_PI && ang < 3.0*M_PI/2.0) //Quadrant III
//        {
//            x *= -1.0;
//            y *= -1.0;
//        }
//        if( ang >= 3.0*M_PI/2.0 ) //Quadrant IV
//            y *= -1.0;
        points.push_back(QPointF(x,y));
        ang += delta_ang;
    }
}

#elif defined LINK_V2
Link::Link()
{
//    path.connectPath()
}

void Link::setInPort(std::shared_ptr<Block::Port> port)
{
    (void)port;
}

void Link::addOutPort(std::shared_ptr<Block::Port> port)
{
    (void)port;
}

void Link::removeInPort(std::shared_ptr<Block::Port> port)
{
    (void)port;
}

void Link::removeOutPort(std::shared_ptr<Block::Port> port)
{
    (void)port;
}

void Link::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    (void)event;
    //path.lineTo(mapFromScene(event->pos()));
    qDebug() << "mouseMoveEvent()";
}

void Link::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    (void)option;
    (void)widget;
    qDebug() << "Link::paint()";
    painter->drawLine(start,end);
}
#endif
} // namespace GuiBlock
