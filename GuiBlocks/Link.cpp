#include "Link.h"

#include <QDebug>
#include <QPainter>
#include "Utils.h"
#include <cmath>

namespace GuiBlocks {

//--------------------------------------
//------ Link::LinkBinTree::Node  ------
Link::LinkBinTree::Node::Node() noexcept
{
    makeEmpty();
    resetIndexs();
}

Link::LinkBinTree::Node::Node(Link::LinkBinTree::Node &&n)
{
    this->connectionPort = n.connectionPort;
    this->firstChildIdx = n.firstChildIdx;
    this->parentNextChildIdx = n.parentNextChildIdx;
    this->point = n.point;
    this->prevNode = n.prevNode;
    //this will avoid disconnection from block port if n is destructed
    n.connectionPort.port = nullptr;
}

Link::LinkBinTree::Node::Node(const QPointF &point,
                              uint16_t parentIdx,
                              uint16_t firstChildIdx,
                              uint16_t parentNextChildIdx) noexcept
{
    setData(point,parentIdx,firstChildIdx,parentNextChildIdx);
}

Link::LinkBinTree::Node::~Node()
{
    makeEmpty();
}

//Link::LinkBinTree::Node &Link::LinkBinTree::Node::operator=(const Link::LinkBinTree::Node &n)
//{
//    this->prevNode =
//    return *this;
//}

bool Link::LinkBinTree::Node::isEmpty() const noexcept
{
    return qIsNaN(point.x());
//    return (firstChildIdx      == invalid_index) &&
//           (parentNextChildIdx == invalid_index) &&
//           (prevNode           == invalid_index);
}

void Link::LinkBinTree::Node::makeEmpty() noexcept
{
    if( connectionPort.port != nullptr )
    {
        connectionPort.port->disconnectPortFromLink();
        //connectionPort.port = nullptr;
        //connectionPort.connected = false;
    }
    point.setX(qQNaN());
//    firstChildIdx = invalid_index;
//    parentNextChildIdx = invalid_index;
//    prevNode = invalid_index;
}

void Link::LinkBinTree::Node::resetIndexs() noexcept
{
    prevNode = invalid_index;
    firstChildIdx = invalid_index;
    parentNextChildIdx = invalid_index;
}

void Link::LinkBinTree::Node::setData(const QPointF &point,
                                      uint16_t parentIdx,
                                      uint16_t firstChildIdx,
                                      uint16_t parentNextChildIdx) noexcept
{
    this->point    = point;
    this->prevNode = parentIdx;
    this->firstChildIdx      = firstChildIdx;
    this->parentNextChildIdx = parentNextChildIdx;
}

void Link::LinkBinTree::Node::show() noexcept
{
    if( isEmpty() )
        qDebug() << "empty";
    else
    {
        QString left = "X";
        if( firstChildIdx != invalid_index )
            left = QString::number(firstChildIdx);
        QString right = "X";
        if( parentNextChildIdx != invalid_index )
            right = QString::number(parentNextChildIdx);
        QString prev = "X";
        if( prevNode != invalid_index )
            prev = QString::number(prevNode);
        qDebug() << point << prev << left << right;
    }
}

//--------------------------------------
//------ Link::LinkBinTree -------------
Link::LinkBinTree::LinkBinTree(const QPointF &pos) noexcept
{
    nodes.reserve(6);
    nodes.emplace_back(pos);
    rootIdx = 0;
}

uint16_t Link::LinkBinTree::appendChild(uint16_t parentIdx,
                                        const QPointF &point) noexcept
{
    if( parentIdx >= nodes.size() )
        return invalid_index;

    //a parentIdx empty implies that the node is not being used, so
    //we can not append a child to it
    if( nodes[parentIdx].isEmpty() )
        return invalid_index;

    auto childIdx = nodes[parentIdx].firstChildIdx;
    if( childIdx == invalid_index )
    {
        nodes[parentIdx].firstChildIdx = createNode(point,parentIdx);
        return nodes[parentIdx].firstChildIdx;//parentNextChildIdx;
    }
    while( childIdx != invalid_index )
    {
        parentIdx = childIdx;
        childIdx = nodes[parentIdx].parentNextChildIdx;
    }
    nodes[parentIdx].parentNextChildIdx = createNode(point,parentIdx);
    return nodes[parentIdx].parentNextChildIdx;
}

uint16_t Link::LinkBinTree::insertBefore(uint16_t targetIdx,const QPointF &point) noexcept
{
    auto idx = createNode(point,
                          nodes[targetIdx].prevNode,
                          targetIdx,
                          nodes[targetIdx].parentNextChildIdx);
    auto prev = nodes[targetIdx].prevNode;
    if( prev == invalid_index )
        rootIdx = idx;
    else
    {
        if( nodes[prev].firstChildIdx == targetIdx )
            nodes[prev].firstChildIdx = idx;
        else
            nodes[prev].parentNextChildIdx = idx;
    }
    nodes[targetIdx].prevNode = idx;
    if( nodes[targetIdx].parentNextChildIdx != invalid_index )
        nodes[nodes[targetIdx].parentNextChildIdx].prevNode = idx;
    nodes[targetIdx].parentNextChildIdx = invalid_index;
    return idx;
}

void Link::LinkBinTree::removeSubTree(uint16_t targetIdx) noexcept
{
    resetSubTreeIterator(targetIdx);
    iterateSubTree();
    auto idx = iterateSubTree();
    while( idx != invalid_index )
    {
        nodes[idx].makeEmpty();
        idx = iterateSubTree();
    }
    nodes[targetIdx].firstChildIdx = invalid_index;
    if( nodes[targetIdx].parentNextChildIdx == invalid_index )
    {
        auto prev = nodes[targetIdx].prevNode;
        if( prev != invalid_index )
        {
            //qDebug() << getParent(targetIdx);
            if( isParent(prev,targetIdx) )
                nodes[prev].firstChildIdx = invalid_index;
            else
                nodes[prev].parentNextChildIdx = invalid_index;
        }
        nodes[targetIdx].makeEmpty();
    }
}

uint16_t Link::LinkBinTree::childrenCount(uint16_t targetIdx) noexcept
{
    if( auto idx = nodes[targetIdx].firstChildIdx; idx != invalid_index )
    {
        //nodes[targetIdx].show();
        uint16_t count = 1;
        while( nodes[idx].parentNextChildIdx != invalid_index )
        {
            count++;
            //nodes[idx].show();
            idx = nodes[idx].parentNextChildIdx;
        }
        return count;
    }
    return 0;
}

uint16_t Link::LinkBinTree::getParent(uint16_t targetIdx) const noexcept
{
    if( targetIdx == rootIdx )
        return invalid_index;
    while( targetIdx != invalid_index )
    {
        if( !prevNodeIsParent(targetIdx) )
            targetIdx = nodes[targetIdx].prevNode;
        else
            return nodes[targetIdx].prevNode;//targetIdx;
    }
    return invalid_index;
}

QPointF &Link::LinkBinTree::operator[](uint16_t idx)
{
    if( idx >= nodes.size() )
        throw("index out of range");

    if( nodes[idx].isEmpty() )
        throw("accesing to an invalid (empty/unused) node");

    return nodes[idx].point;
}

const QPointF &Link::LinkBinTree::getPoint(uint16_t idx) const
{
    if( idx >= nodes.size() )
        throw("index out of range");

    if( nodes[idx].isEmpty() )
        throw("accesing to an invalid (empty/unused) node");

    return nodes[idx].point;
}

uint16_t Link::LinkBinTree::length() const noexcept
{
    uint16_t count = 0;
    for( const auto& node : nodes )
        if( !node.isEmpty() )
            count++;
    return count;
}

void Link::LinkBinTree::resetChildIter(uint16_t parentIdx) noexcept
{
    if( parentIdx >= nodes.size() )
    {
        iterChildIdx = invalid_index;
        return;
    }
    iterChildIdx = nodes[parentIdx].firstChildIdx;
}

uint16_t Link::LinkBinTree::childIter() noexcept
{
    if( iterChildIdx >= nodes.size() )
        return invalid_index;

    if( nodes[iterChildIdx].isEmpty() )
        return invalid_index;

    uint16_t childIdx = iterChildIdx;
    iterChildIdx = nodes[childIdx].parentNextChildIdx;
    return childIdx;
}

void Link::LinkBinTree::resetSubTreeIterator(uint16_t startIdx) noexcept
{
    if( startIdx >= nodes.size() )
    {
        iterSubTreeStart = invalid_index;
        iterSubTreeIdx   = invalid_index;
        return;
    }
    if( nodes[startIdx].isEmpty() )
    {
        iterSubTreeStart = invalid_index;
        iterSubTreeIdx   = invalid_index;
        return;
    }
    iterSubTreeStart = startIdx;
    iterSubTreeIdx   = startIdx;
}

uint16_t Link::LinkBinTree::iterateSubTree() noexcept
{
    if( iterSubTreeIdx == invalid_index )
        return invalid_index;

    auto idx = iterSubTreeIdx;
    if( nodes[iterSubTreeIdx].firstChildIdx != invalid_index )
    {
        iterSubTreeIdx = nodes[iterSubTreeIdx].firstChildIdx;
    }
    else if( nodes[iterSubTreeIdx].parentNextChildIdx != invalid_index &&
             iterSubTreeIdx != iterSubTreeStart )
    {
        iterSubTreeIdx = nodes[iterSubTreeIdx].parentNextChildIdx;
    }
    else
    {
        while( iterSubTreeIdx != iterSubTreeStart )
            if( (!isParent(nodes[iterSubTreeIdx].prevNode,iterSubTreeIdx) ||
                   nodes[nodes[iterSubTreeIdx].prevNode].parentNextChildIdx == invalid_index) )
            {
                iterSubTreeIdx = nodes[iterSubTreeIdx].prevNode;
            }
            else
                break;
        if( iterSubTreeIdx == iterSubTreeStart )
            iterSubTreeIdx = invalid_index;
        else
        {
            if( isParent(nodes[iterSubTreeIdx].prevNode,iterSubTreeIdx) &&
                nodes[nodes[iterSubTreeIdx].prevNode].parentNextChildIdx != invalid_index &&
                nodes[iterSubTreeIdx].prevNode != iterSubTreeStart )
                iterSubTreeIdx = nodes[nodes[iterSubTreeIdx].prevNode].parentNextChildIdx;
            else
                if( nodes[iterSubTreeIdx].prevNode != iterSubTreeStart )
                    iterSubTreeIdx = nodes[nodes[iterSubTreeIdx].prevNode].parentNextChildIdx;
                else
                    iterSubTreeIdx = invalid_index;
        }
    }
    return idx;
}

void Link::LinkBinTree::resetIterator(IterPointers *pointers) const noexcept
{
    if( pointers == nullptr )
    {
        iterPointers.iterStart = rootIdx;
        pointers = &iterPointers;
    }
    uint16_t &iterParent = pointers->iterParent;
    uint16_t &iterChild  = pointers->iterChild;
    uint16_t &iterStart  = pointers->iterStart;
    if( iterStart == invalid_index )
    {
        iterParent = rootIdx;
        iterStart  = iterParent;
    }
    else
    {
        if( iterStart >= nodes.size() )
        {
            iterParent = invalid_index;
            iterChild  = invalid_index;
            return;
        }
        else
            iterParent = iterStart;
    }
    iterChild  = nodes[iterParent].firstChildIdx;
}

std::optional<std::tuple<const QPointF&,const QPointF&>>
Link::LinkBinTree::iterate(IterPointers *pointers) const noexcept
{
    if( auto indexs = iterateIdx(pointers) )
    {
        auto[from,to] = indexs.value();
        return {{nodes[from].point,nodes[to].point}};
    }
    return {};
}

std::optional<std::tuple<uint16_t,uint16_t>>
Link::LinkBinTree::iterateIdx(Link::LinkBinTree::IterPointers *pointers) const noexcept
{
    if( pointers == nullptr )
        pointers = &iterPointers;
    uint16_t &iterParent = pointers->iterParent;
    uint16_t &iterChild  = pointers->iterChild;
    uint16_t &iterStart  = pointers->iterStart;
    if( iterParent == invalid_index || iterChild == invalid_index )
        return {};
    const auto from = iterParent;
    const auto to   = iterChild;

    //iterChild and iterParent moves down if iterChild has children
    if( nodes[iterChild].firstChildIdx != invalid_index )
    {
        iterParent = iterChild;
        iterChild  = nodes[iterChild].firstChildIdx;
    }
    //iterChild moves down (iterParent does not move) if iterChild
    //does not have childrens
    else if( nodes[iterChild].parentNextChildIdx != invalid_index )
    {
        iterChild = nodes[iterChild].parentNextChildIdx;
    }
    //if iterParent has parent child, then iterChild jumps branch
    //to that child, and iterParent moves up until find the parent
    else if( nodes[iterParent].parentNextChildIdx != invalid_index &&
             iterParent != iterStart )
    {
        iterChild = nodes[iterParent].parentNextChildIdx;
        //iterParent moves up until find the parent of iterChild
        while( !prevNodeIsParent(iterParent) )
            iterParent = nodes[iterParent].prevNode;
        iterParent = nodes[iterParent].prevNode;
    }
    else
    {
        //if iterParent has not parent childs, then it should move up
        //until find a node with a parent child different from itself
        //(the parent child must be different to iterParent), or reaching
        //the iterStart
        while( iterParent != iterStart )
        {
            if( prevNodeIsParent(iterParent) &&
                    nodes[nodes[iterParent].prevNode].parentNextChildIdx != invalid_index)
            {
                //iterChild jumps branch
                iterParent = nodes[iterParent].prevNode;
                iterChild  = nodes[iterParent].parentNextChildIdx;
                //iterParent moves up until find the parent of iterChild
                while( !prevNodeIsParent(iterParent) )
                {
                    if( iterParent != iterStart )
                        iterParent = nodes[iterParent].prevNode;
                    else
                        return {{from,to}};
                        //goto out;
                }
                iterParent = nodes[iterParent].prevNode;
                return {{from,to}};
            }
            else
                //iterParent move up
                iterParent = nodes[iterParent].prevNode;
        }
//        out:
        if( iterParent == iterStart )
        {
            iterParent = invalid_index;
            iterChild  = invalid_index;
        }
    }
    return {{from,to}};
}

void Link::LinkBinTree::showNodes() const noexcept
{
    for( uint16_t idx=0 ; idx<nodes.size() ; idx++ )
    {
        if( nodes[idx].isEmpty() )
            qDebug() << "empty";
        else
        {
            QString left = "X";
            if( nodes[idx].firstChildIdx != invalid_index )
                left = QString::number(nodes[idx].firstChildIdx);
            QString right = "X";
            if( nodes[idx].parentNextChildIdx != invalid_index )
                right = QString::number(nodes[idx].parentNextChildIdx);
            QString parent = "X";
            if( nodes[idx].prevNode != invalid_index )
                parent = QString::number(nodes[idx].prevNode);
            QString port = "";
            if( nodes[idx].connectionPort.port != nullptr )
                port = nodes[idx].connectionPort.port->name;
            qDebug() << idx << ": " << nodes[idx].point << parent << left << right << port;
        }
    }
}

void Link::LinkBinTree::showIteration(uint16_t startIdx) noexcept
{
    IterPointers iterPointers(startIdx);
    resetIterator(&iterPointers);
    while( auto points = iterateIdx(&iterPointers) )
    {
        auto[fromIdx,toIdx] = points.value();
        auto from = nodes[fromIdx].point;//findPoint(p1);
        auto to   = nodes[toIdx].point;//findPoint(p2);
        qDebug() << fromIdx << "->" << toIdx << "(" << from << "->" << to << ")";
    }
}

void Link::LinkBinTree::showSubTreeIterations(uint16_t startIdx) noexcept
{
    qDebug() << "Sub Tree Iterations:";
    resetSubTreeIterator(startIdx);
    uint16_t idx;
    do
    {
        idx = iterateSubTree();
        if( idx != invalid_index )
            qDebug() << idx;
    }
    while( idx != invalid_index );
}

uint16_t Link::LinkBinTree::nextEmptyNode(uint16_t startIdx) const noexcept
{
    for( ; startIdx < nodes.size() ; ++startIdx )
        if( nodes[startIdx].isEmpty() )
            return startIdx;
    return invalid_index;
}

uint16_t Link::LinkBinTree::createNode(const QPointF &pos,
                                       uint16_t prevNode,
                                       uint16_t firstChildIdx,
                                       uint16_t parentNextChildIdx) noexcept
{
    auto idx = nextEmptyNode();
    //return the first empty node if there is one
    if( idx != invalid_index )
    {
        nodes[idx].setData(pos,prevNode,firstChildIdx,parentNextChildIdx);
        return idx;
    }
    //creates an empty node
    nodes.reserve(nodes.size()+3);
    nodes.emplace_back(pos,prevNode,firstChildIdx,parentNextChildIdx);
    return uint16_t(nodes.size()-1);
}

bool Link::LinkBinTree::simplifyAlignedNode(uint16_t targetIdx) noexcept
{
    //simplification can be applied only to nodes that are located in
    //between two other nodes and are not jointnodes
    if( !isBetweenTwoNodes(targetIdx) || isJointNode(targetIdx) )
        return false;
    if( isMiddleOfLine(targetIdx) )
    {
        auto prevChildIdx = nodes[targetIdx].parentNextChildIdx;
        auto next = nodes[targetIdx].firstChildIdx;
        auto prev = nodes[targetIdx].prevNode;
        nodes[next].parentNextChildIdx = prevChildIdx;
        nodes[next].prevNode = prev;
        if( isParent(prev,targetIdx) )
            nodes[prev].firstChildIdx = next;
        else
            nodes[prev].parentNextChildIdx = next;
        if( prevChildIdx != LinkBinTree::invalid_index )
            nodes[prevChildIdx].prevNode = next;
        nodes[targetIdx].makeEmpty();
        return true;
    }
    return false;
}

bool Link::LinkBinTree::isParent(uint16_t parentIdx, uint16_t childIdx) const noexcept
{
    return nodes[parentIdx].firstChildIdx == childIdx;
}

bool Link::LinkBinTree::prevNodeIsParent(uint16_t childIdx) const noexcept
{
    auto prev = nodes[childIdx].prevNode;
    return isParent(prev,childIdx);//nodes[prev].firstChildIdx == childIdx;
}

bool Link::LinkBinTree::isJointNode(uint16_t targetIdx) const noexcept
{
    if( auto childIdx = nodes[targetIdx].firstChildIdx; childIdx != invalid_index )
        if( nodes[childIdx].parentNextChildIdx != invalid_index )
            return true;
    return false;
}

std::optional<std::tuple<uint16_t, uint16_t>>
Link::LinkBinTree::isOnTrajectory(const QPointF &point) const noexcept
{
    IterPointers iterPointers;
    resetIterator(&iterPointers);
    while( auto points = iterateIdx(&iterPointers) )
    {
        auto[idxP1,idxP2] = points.value();
        auto p1 = nodes[idxP1].point;
        auto p2 = nodes[idxP2].point;
        auto[minx,maxx] = std::minmax(p1.x(),p2.x());
        auto[miny,maxy] = std::minmax(p1.y(),p2.y());

        //the rectangle with top-left = p1, and botton-right = p2 will be the working area
        //and the point should be inside this rectangle, if not, then point is not on the
        //trajectory between p1 and p2
        if( !(point.x() < minx || point.x() > maxx || point.y() < miny || point.y() > maxy) )
        {
            //if point is exactly a node, return the index of that node and invalid_index
            if( p1 == point )
                return {{idxP1,invalid_index}};
            if( p2 == point )
                return {{invalid_index,idxP2}};

            //now checks if point is on the trajectory joining p1 and p2
            auto dx = p2.x()-p1.x();
            //corner case: the line is vertical (ie, dx = 0)
            if( dx == 0.0 && equals(point.x(),p1.x()) )
                return {{idxP1,idxP2}};
            else
            {
                //the line is not vertical.
                //now we compute the coefficients of the line joining p1 and p2:
                // y = a*x + b
                auto a = (p2.y()-p1.y())/dx;
                auto b = -a*p1.x()+p1.y();
                //point is on the line if: point.y == a*point.x + b
                if( equals( point.y() , a*point.x()+b ) )
                    return {{idxP1,idxP2}};
            }
        }
    }
    return {};
}

std::optional<std::tuple<uint16_t, uint16_t>>
Link::LinkBinTree::isOnTrajectory(const QRectF &rect) const noexcept
{
    IterPointers iterPointers;
    resetIterator(&iterPointers);
    while( auto points = iterateIdx(&iterPointers) )
    {
        auto[idxP1,idxP2] = points.value();
        auto p1 = nodes[idxP1].point;
        auto p2 = nodes[idxP2].point;

        if( rect.contains(p1) && rect.contains(p2) )
            return {{idxP1,idxP2}};
        if( rect.contains(p1) )
            return {{idxP1,invalid_index}};
        if( rect.contains(p2) )
            return {{invalid_index,idxP2}};

        //if( QRectF(p1,p2).contains(rect) )
        {
            auto point = rect.center();
            //now checks if point is on the trajectory joining p1 and p2
            auto dx = p2.x()-p1.x();
            //corner case: the line is vertical (ie, dx = 0)
            if( dx == 0.0 && equals(point.x(),p1.x(),rect.width()/2.0) )
            {
                const auto[miny,maxy] = std::minmax(p1.y(),p2.y());
                if( point.y()+rect.height()/2.0 <= maxy && point.y()-rect.height()/2.0 >= miny )
                    return {{idxP1,idxP2}};
            }
            else
            {
                //the line is not vertical.
                //now we compute the coefficients of the line joining p1 and p2:
                // y = a*x + b
                auto a = (p2.y()-p1.y())/dx;
                auto b = -a*p1.x()+p1.y();
                //point is on the line if: point.y == a*point.x + b
                if( equals( point.y() , a*point.x()+b , rect.height()/2.0 ) )
                    return {{idxP1,idxP2}};
            }
        }
    }
    return {};
}

std::optional<std::tuple<uint16_t,uint16_t>>
Link::LinkBinTree::isMiddleOfLine(uint16_t targetIdx) const noexcept
{
    auto parentIdx = getParent(targetIdx);
    auto childIdx = nodes[targetIdx].firstChildIdx;
    if( parentIdx == invalid_index || childIdx == invalid_index )
        return {};

    auto point = nodes[targetIdx].point;
    auto p1 = nodes[parentIdx].point;
    auto p2 = nodes[childIdx].point;
    auto[minx,maxx] = std::minmax(p1.x(),p2.x());
    auto[miny,maxy] = std::minmax(p1.y(),p2.y());
    if( !(point.x() < minx || point.x() > maxx || point.y() < miny || point.y() > maxy) )
    {
        //if point is exactly a node, return the index of that node and invalid_index
        if( p1 == point )
            return {{parentIdx,invalid_index}};
        if( p2 == point )
            return {{invalid_index,childIdx}};

        //now checks if point is on the trajectory joining p1 and p2
        auto dx = p2.x()-p1.x();
        //corner case: the line is vertical (ie, dx = 0)
        if( dx == 0.0 && equals(point.x(),p1.x()) )
            return {{parentIdx,childIdx}};
        else
        {
            //the line is not vertical.
            //now we compute the coefficients of the line joining p1 and p2:
            // y = a*x + b
            auto a = (p2.y()-p1.y())/dx;
            auto b = -a*p1.x()+p1.y();
            //point is on the line if: point.y == a*point.x + b
            if( equals( point.y() , a*point.x()+b ) )
                return {{parentIdx,childIdx}};
        }
    }
    return {};
}

bool Link::LinkBinTree::isBetweenTwoNodes(uint16_t targetIdx) const noexcept
{
    if( nodes[targetIdx].prevNode      == invalid_index ||
        nodes[targetIdx].firstChildIdx == invalid_index )
        return false;
    return true;
}

std::optional<std::tuple<uint16_t,uint16_t>>
Link::LinkBinTree::belongsToLine(uint16_t idxP1,
                                 uint16_t idxP2,
                                 const QPointF &point) const noexcept
{
    auto p1 = nodes[idxP1].point;
    auto p2 = nodes[idxP2].point;
    auto[minx,maxx] = std::minmax(p1.x(),p2.x());
    auto[miny,maxy] = std::minmax(p1.y(),p2.y());

    //the rectangle with top-left = p1, and botton-right = p2 will be the working area
    //and the point should be inside this rectangle, if not, then point is not on the
    //trajectory between p1 and p2
    if( !(point.x() < minx || point.x() > maxx || point.y() < miny || point.y() > maxy) )
    {
        //if point is exactly a node, return the index of that node and invalid_index
        if( p1 == point )
            return {{idxP1,invalid_index}};
        if( p2 == point )
            return {{invalid_index,idxP2}};

        //now checks if point is on the trajectory joining p1 and p2
        auto dx = p2.x()-p1.x();
        //corner case: the line is vertical (ie, dx = 0)
        if( dx == 0.0 && equals(point.x(),p1.x()) )
            return {{idxP1,idxP2}};
        else
        {
            //the line is not vertical.
            //now we compute the coefficients of the line joining p1 and p2:
            // y = a*x + b
            auto a = (p2.y()-p1.y())/dx;
            auto b = -a*p1.x()+p1.y();
            //point is on the line if: point.y == a*point.x + b
            if( equals( point.y() , a*point.x()+b ) )
                return {{idxP1,idxP2}};
        }
    }
    return {};
}

//--------------------------------------
//------ Link
Link::Link(const QPointF &startPos)
    : //points(startPos),
      tree(startPos)
{

//    setFlags(QGraphicsItem::ItemIsMovable);
    auto effect = new QGraphicsDropShadowEffect;
    effect->setOffset(2, 2);
    effect->setBlurRadius(15);
    effect->setColor(StyleLink::shadowColor);
    setGraphicsEffect(effect);
}

//Link::Link(const Link &l)
//    : QGraphicsItem(nullptr),
////      points(l.points),
//      tree(l.tree)
//{
////    activePort  = l.activePort;
//    //    pasivePorts = l.pasivePorts;
//}

void Link::paint(QPainter *painter,
                 const QStyleOptionGraphicsItem *option,
                 QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    //points.resetIterator();
    //points.show();
    painter->setPen(QPen(QBrush(StyleLink::normalColor),
                         qreal(StyleLink::width),
                         StyleLink::normalLine,
                         StyleLink::normalCap));
    painter->setFont(StyleText::blockHintFont);

    tree.resetIterator();
    while( auto idx = tree.iterateIdx() )
    {
        auto[from,to] = idx.value();
        //draw line:
        painter->drawLine(tree[from],tree[to]);
        //draw node:
        if( tree.childrenCount(to) > 1 )
        painter->drawEllipse(tree[to],2,2);

        //[DEBUG] draw node index:
        #define LINK_DEBUG
        #ifdef LINK_DEBUG
        painter->save();
        if( from == tree.rootIdx )
            painter->setPen(QPen(QBrush(Qt::magenta),
                                 qreal(StyleLink::width),
                                 StyleLink::normalLine,
                                 StyleLink::normalCap));
        else
            painter->setPen(QPen(QBrush(Qt::red),
                                 qreal(StyleLink::width),
                                 StyleLink::normalLine,
                                 StyleLink::normalCap));
        painter->drawText(tree[from],QString::number(from)+"["+QString::number(tree.childrenCount(from))+"]"+"("+QString::number(tree[from].x())+","+QString::number(tree[from].y())+")");
        if( to == tree.rootIdx )
            painter->setPen(QPen(QBrush(Qt::magenta),
                                 qreal(StyleLink::width),
                                 StyleLink::normalLine,
                                 StyleLink::normalCap));
        else
            painter->setPen(QPen(QBrush(Qt::red),
                                 qreal(StyleLink::width),
                                 StyleLink::normalLine,
                                 StyleLink::normalCap));
        painter->drawText(tree[to]  ,QString::number(to)  +"["+QString::number(tree.childrenCount(to))  +"]"+"("+QString::number(tree[to].x())  +","+QString::number(tree[to]  .y())+")");
        painter->restore();
        #endif
    }
    #ifdef LINK_DEBUG
    painter->setPen(QPen(QBrush(Qt::blue),
                         1,
                         Qt::DashLine,
                         StyleLink::normalCap));
    painter->drawRect(boundingRect());
    #endif
}

std::optional<QPointF> Link::computeMidPoint(const QPointF &startPoint,
                                             const QPointF &endPoint,
                                             const Link::LinkPath &linkPath) const noexcept
{
    QPointF midp;
    switch( linkPath )
    {
        case LinkPath::straight:
            return {};
        case LinkPath::verticalThenHorizontal:
            midp = QPointF( startPoint.x() , endPoint.y() );
            break;
        case LinkPath::horizontalThenVertical:
            midp = QPointF( endPoint.x() , startPoint.y() );
            break;
        case LinkPath::straightThenOrthogonal:
            {
                auto diffp = startPoint-endPoint;
                auto diffx = std::abs(diffp.x());
                auto diffy = std::abs(diffp.y());
                if( diffx < diffy )
                {
                    if( endPoint.y() > startPoint.y() )
                        midp = QPointF( endPoint.x() , startPoint.y()+diffx);
                    else
                        midp = QPointF( endPoint.x() , startPoint.y()-diffx);
                }
                else
                {
                    if( endPoint.x() > startPoint.x() )
                        midp = QPointF( startPoint.x()+diffy , endPoint.y() );
                    else
                        midp = QPointF( startPoint.x()-diffy , endPoint.y() );
                }
            }
            break;
        case LinkPath::orthogonalThenStraight:
            {
                auto diffp = startPoint-endPoint;
                auto diffx = std::abs(diffp.x());
                auto diffy = std::abs(diffp.y());
                if( diffx <= diffy )
                {
                    if( endPoint.y() < startPoint.y() )
                        midp = QPointF( startPoint.x() , endPoint.y()+diffx);
                    else
                        midp = QPointF( startPoint.x() , endPoint.y()-diffx);
                }
                else
                {
                    if( endPoint.x() < startPoint.x() )
                        midp = QPointF( endPoint.x()+diffy , startPoint.y() );
                    else
                        midp = QPointF( endPoint.x()-diffy , startPoint.y() );
                }
            }
            break;
    }
    return midp;
}

void Link::updateContainerRect()
{
    auto ptl = tree[tree.rootIdx];
    auto pbr = tree[tree.rootIdx];
    for( uint16_t idx=0 ; idx<tree.nodes.size() ; idx++ )
    {
        if( tree.nodes[idx].isEmpty() )
            continue;

        auto x = tree[idx].x();
        auto y = tree[idx].y();
        if( ptl.x() > x )
            ptl.setX(x);
        if( ptl.y() < y )
            ptl.setY(y);
        if( pbr.x() < x )
            pbr.setX(x);
        if( pbr.y() > y )
            pbr.setY(y);
    }
    ptl.setX(ptl.x()-double(StyleGrid::gridSize));
    ptl.setY(ptl.y()+double(StyleGrid::gridSize));
    pbr.setX(pbr.x()+double(StyleGrid::gridSize));
    pbr.setY(pbr.y()-double(StyleGrid::gridSize));
    containerRect = QRectF(ptl,pbr);
}

bool Link::simplifyRootNode() noexcept
{
    if( tree.childrenCount(tree.rootIdx) != 2 )
        return false;
    const auto first  = tree.nodes[tree.rootIdx].firstChildIdx;
    const auto second = tree.nodes[first].parentNextChildIdx;
    if( tree.belongsToLine(first,second,tree[tree.rootIdx]) )
    {
        tree.nodes[first].prevNode = LinkBinTree::invalid_index;
        tree.nodes[first].parentNextChildIdx = LinkBinTree::invalid_index;
        auto childIdx = tree.nodes[first].firstChildIdx;
        if( childIdx != LinkBinTree::invalid_index )
        {
            while( tree.nodes[childIdx].parentNextChildIdx != LinkBinTree::invalid_index )
                childIdx = tree.nodes[childIdx].parentNextChildIdx;
            tree.nodes[childIdx].parentNextChildIdx = second;
        }
        else
        {
            childIdx = first;
            tree.nodes[childIdx].firstChildIdx = second;
        }
        tree.nodes[second].prevNode = childIdx;
        tree.nodes[tree.rootIdx].makeEmpty();
        tree.rootIdx = first;
        return true;
    }
    return false;
}

std::tuple<uint16_t,uint16_t> Link::getGrabbedIndexs(const QPointF &pos) const noexcept
{
    if( tree.length() < 2 )
        return {LinkBinTree::invalid_index,LinkBinTree::invalid_index};
    //Computes the distances from pos to the nearest point and line,
    //if the line is closer to pos, its indexes will be return, otherwise
    //the pos's indexes will be returned.

    //distances from pos to a node (pdist) and to a line (ldist)
    auto pdist = qInf();
    auto ldist = qInf();
    //index of the node that is closest to pos
    auto pIdx  = LinkBinTree::invalid_index;
    //indexes of the line that is closest to pos
    auto lIdx1 = LinkBinTree::invalid_index;
    auto lIdx2 = LinkBinTree::invalid_index;
    QPointF inter;

    LinkBinTree::IterPointers iterPointer;
    tree.resetIterator(&iterPointer);
    while( auto idx = tree.iterateIdx(&iterPointer) )
    {
        const auto[from,to] = idx.value();
        //compute the distances from pos to p1 and p2
        const auto& p1 = tree.getPoint(from);
        const auto& p2 = tree.getPoint(to);
        if( pdist > squareDistance(pos,p1) )
        {
            //records the min distance and the index
            pdist = squareDistance(pos,p1);
            pIdx  = from;
        }
        if( pdist > squareDistance(pos,p2) )
        {
            //records the min distance and the index
            pdist = squareDistance(pos,p2);
            pIdx  = to;
        }
        //now compute the distance from pos to the line
        //that is formed by p1 and p2.
        //This distance is taken from the perpendicular line that
        //connects pos with the target line (formed by p1 and p2)
        QLineF line(p1,p2);
        QPointF pp1( pos.x()+line.dy() , pos.y()-line.dx() );
        QPointF pp2( pos.x()-line.dy() , pos.y()+line.dx() );
        QLineF pline(pp1,pp2);

        switch( line.intersects(pline,&pp1) )
        {
            case QLineF::UnboundedIntersection:
                //if the unbounded intersection point is part of line then
                //compute the distance between pos a pp1
                if( tree.belongsToLine(from,to,pp1) )
                    if( ldist > squareDistance(pos,pp1) )
                    {
                        ldist = squareDistance(pos,pp1);
                        lIdx1 = from;
                        lIdx2 = to;
                        inter = pp1;
                    }
                break;
            case QLineF::BoundedIntersection:
                if( ldist > squareDistance(pos,pp1) )
                {
                    ldist = squareDistance(pos,pp1);
                    lIdx1 = from;
                    lIdx2 = to;
                    inter = pp1;
                }
                break;
            case QLineF::NoIntersection:
                qDebug() << "********************** this represents a throw (since we are using perpendicular lines, should allways be an interseccion)" << __LINE__;
                break;
        }
    }
    //if no line was selected, then returns the pIdx
    if( lIdx1 == LinkBinTree::invalid_index )
        return {pIdx,LinkBinTree::invalid_index};

    //only select the closest line if pos is near the center of the line:
    //Lenght of the line
    const auto len = squareDistance(tree.getPoint(lIdx1),tree.getPoint(lIdx2));
    //distance from intersecction to nearest extreme
    auto lIdx  = lIdx1;
    auto edist = squareDistance(inter,tree.getPoint(lIdx1));
    if( edist > squareDistance(inter,tree.getPoint(lIdx2)) )
    {
        edist = squareDistance(inter,tree.getPoint(lIdx2));
        lIdx = lIdx2;
    }
    if( edist < len*0.04 )  //0.2^2 = 0.04 -> this represents de 20% of the lenght
    {
        //this means that pos is closer to the extreme (lIdx) than is
        //to the center of the line.
        //Now the closest node (pIdx or lIdx) will be selected
        if( squareDistance(pos,tree.getPoint(lIdx)) < pdist )
            return {lIdx,LinkBinTree::invalid_index};
        return {pIdx,LinkBinTree::invalid_index};
    }
    if( pdist < ldist )
        return {pIdx,LinkBinTree::invalid_index};
    return {lIdx1,lIdx2};
}

bool Link::isPartOfLink(const QPointF &point) const noexcept
{
    return tree.isOnTrajectory(point).has_value();
}

bool Link::isPartOfLink(const QRectF &rect) const noexcept
{
    return tree.isOnTrajectory(rect).has_value();
}

void Link::insertLineAt(const QPointF  &start,
                        const QPointF  &end,
                        const LinkPath &linkPath) noexcept
{
    if( tree.length() == 1 )
    {
        if( const auto &validMidPoint = computeMidPoint(start,end,linkPath) )
        {
            idxMid = tree.appendChild(idxStart,validMidPoint.value());
            idxEnd = tree.appendChild(idxMid,end);
        }
        else
        {
            idxMid = LinkBinTree::invalid_index;
            idxEnd = tree.appendChild(idxStart,end);
        }
        updateContainerRect();
        prepareGeometryChange();
        update();
        return;
    }
    if( auto idx = tree.isOnTrajectory(start) )
    {

        auto[prev,next] = idx.value();
        if( prev == LinkBinTree::invalid_index || next == LinkBinTree::invalid_index )
        {
            //the line starts in a valid node:
            auto targetIdx = prev;
            if( prev == LinkBinTree::invalid_index )
                targetIdx = next;
            idxStart = targetIdx;
            if( const auto &validMidPoint = computeMidPoint(start,end,linkPath) )
            {
                idxMid   = tree.appendChild(idxStart,validMidPoint.value());
                idxEnd   = tree.appendChild(idxMid,end);
            }
            else
            {
                idxMid = LinkBinTree::invalid_index;
                idxEnd = tree.appendChild(targetIdx,end);
            }
        }
        else
        {
            //the line starts in between two nodes, a new node
            //will be inserted. After, the other nodes of the line
            //will be appended to this new node
            idxStart = tree.insertBefore(next,start);
            if( const auto &validMidPoint = computeMidPoint(start,end,linkPath) )
            {
                idxMid = tree.appendChild(idxStart,validMidPoint.value());
                idxEnd = tree.appendChild(idxMid,end);
            }
            else
            {
                idxMid = LinkBinTree::invalid_index;
                idxEnd = tree.appendChild(idxStart,end);
            }
        }
        updateContainerRect();
        prepareGeometryChange();
        update();
        return;
    }
}

void Link::updateLastInsertedLine(const QPointF &end, const Link::LinkPath &linkPath) noexcept
{
    //if no line was inserted, do nothing
    if( idxStart != LinkBinTree::invalid_index )
    {
        if( const auto& midp = computeMidPoint(tree[idxStart],end,linkPath) )
        {
            //insert midp or update if exists
            if( idxMid != LinkBinTree::invalid_index )
                //update
                tree[idxMid] = midp.value();
            else
                //insert
                idxMid = tree.insertBefore(idxEnd,midp.value());
            tree[idxEnd] = end;
        }
        else
        {
            //remove midp if exists, or update end
            if( idxMid != LinkBinTree::invalid_index )
            {
                tree.removeSubTree(idxEnd);
                tree[idxMid] = end;
                idxEnd = idxMid;
                idxMid = LinkBinTree::invalid_index;
            }
            else
                tree[idxEnd] = end;
        }

        updateContainerRect();
        prepareGeometryChange();
        update();
    }
}

void Link::simplifyLastInsertedLine() noexcept
{
    if( idxStart == LinkBinTree::invalid_index )
        return;

    if( tree[idxStart] == tree[idxEnd] )
    {
        if( idxMid != LinkBinTree::invalid_index )
        {
            tree.removeSubTree(idxMid);
            idxEnd = idxMid;
            idxMid = LinkBinTree::invalid_index;
        }
        else
        {
            tree.nodes[idxEnd].makeEmpty();
            auto prev = tree.nodes[idxEnd].prevNode;
            if( tree.isParent(prev,idxEnd) )
                tree.nodes[prev].firstChildIdx = LinkBinTree::invalid_index;
            else
                tree.nodes[prev].parentNextChildIdx = LinkBinTree::invalid_index;
        }
        //tree.simplifyAlignedNode(idxStart);
        goto out;
    }
    if( idxMid != LinkBinTree::invalid_index )
    {
        if( tree[idxMid] == tree[idxEnd] )
        {
            //simplifaction is needed
            tree.removeSubTree(idxEnd);
            idxEnd = idxMid;
            idxMid = LinkBinTree::invalid_index;
            goto out;
        }
        if( tree[idxMid] == tree[idxStart] )
        {
            auto point = tree[idxEnd];
            tree.removeSubTree(idxEnd);
            tree[idxMid] = point;
            idxEnd = idxMid;
            idxMid = LinkBinTree::invalid_index;
            goto out;
        }
    }
    out:
    tree.simplifyAlignedNode(idxStart);

    //the idxStart node may be aligned with its parent, and if the parent is
    //also aligned with its parent, the a simplification can be done
    auto idx = tree.getParent(idxStart);
    if( idx != LinkBinTree::invalid_index )
        tree.simplifyAlignedNode(idx);
    else
        simplifyRootNode();


    updateContainerRect();
    prepareGeometryChange();
    update();
}

void Link::removeLastInsertedLine() noexcept
{
    if( idxMid != LinkBinTree::invalid_index )
        tree.removeSubTree(idxMid);
    else
        tree.removeSubTree(idxEnd);
    tree.simplifyAlignedNode(idxStart);
    idxStart = LinkBinTree::invalid_index;
    updateContainerRect();
    prepareGeometryChange();
    update();
}

void Link::selectArea(const QPainterPath &shape) noexcept
{
    //here we could use an IterPointers, but since we only
    //need to determine which nodes are contained by shape
    //regardless of relationship between the nodes, we
    //will use a simple for.
    //NOTE: this may fail if some invalid nodes are not marked
    //as empty nodes.
    //NOTE: this logics relies on the correct assigment of "empty"
    //to the nodes that are unused
    selectedIdx.clear();
    for( uint16_t idx=0 ; idx<tree.nodes.size() ; idx++ )
        if( !tree.nodes[idx].isEmpty() )
            if( shape.contains(tree.nodes[idx].point) )
                selectedIdx.push_back(idx);
}

void Link::selectAreaFirstItem(const QPainterPath &shape) noexcept
{
    selectedIdx.clear();
    for( uint16_t idx=0 ; idx<tree.nodes.size() ; idx++ )
        if( !tree.nodes[idx].isEmpty() )
            if( shape.contains(tree.nodes[idx].point) )
            {
                selectedIdx.push_back(idx);
                return;
            }
}

void Link::selectAreaNearestItem(const QPointF &pos) noexcept
{
    auto[idx1,idx2] = getGrabbedIndexs(pos);
    selectedIdx.clear();
    if( idx1 != LinkBinTree::invalid_index )
        selectedIdx.push_back(idx1);
    if( idx2 != LinkBinTree::invalid_index )
        selectedIdx.push_back(idx2);
}

void Link::clearSelectedArea() noexcept
{
    selectedIdx.clear();
}

bool Link::isSelectedAreaMovable() const noexcept
{
    if( selectedIdx.size() == 0 )
        return false;
    for( const auto idx : selectedIdx )
        if( tree.nodes[idx].connectionPort.connected )
            return false;
    return true;
}

bool Link::isConnectedAtPoint(const QPointF &point) const noexcept
{
    for( auto &node : tree.nodes )
        if( node.point == point )
            return node.connectionPort.connected;
    return false;
}

void Link::displaceSelectedArea(const QPointF &offset) noexcept
{
    for( auto idx : selectedIdx )
        tree[idx] += offset;
    updateContainerRect();
    prepareGeometryChange();
    update();
}

void Link::moveSelectedNode(uint16_t nodeIdx,const QPointF &to)
{
    tree[nodeIdx] = to;
    updateContainerRect();
    prepareGeometryChange();
    update();
}

void Link::simplifySelectedArea() noexcept
{
    if( selectedIdx.size() == 0 )
        return;
    for( auto idx : selectedIdx )
        if( !tree.nodes[idx].isEmpty() )
        {
            if( auto parent = tree.getParent(idx); parent != LinkBinTree::invalid_index )
            {
                tree.simplifyAlignedNode(parent);
                //tree.simplifyOverlap(parent);
            }
            else
            {
                //if no parent, then idx is rootIdx
                //if rootIdx has two childs, and are alligned,
                //then the rootIdx should be removed
                if( tree.childrenCount(idx) == 2 )
                {
                    simplifyRootNode();
                    idx = tree.nodes[tree.rootIdx].firstChildIdx;
                }
            }

            tree.resetChildIter(idx);
            while( true )
            {
                if( auto childIdx = tree.childIter(); childIdx != LinkBinTree::invalid_index )
                    tree.simplifyAlignedNode(childIdx);
                else
                    break;
            }
            tree.simplifyAlignedNode(idx);
        }
    updateContainerRect();
    prepareGeometryChange();
    update();
}

bool Link::isPosOnlyEndPoint(const QPointF &pos) noexcept
{
//    (void)pos;
    LinkBinTree::IterPointers iter;
    tree.resetIterator(&iter);
    while( const auto& idx = tree.iterateIdx(&iter) )
    {
        auto[idxP1,idxP2] = idx.value();
        if( idxP1 == idxEnd || idxP2 == idxEnd )
            continue;
        if( const auto& idx = tree.belongsToLine(idxP1,idxP2,pos) )
        {
            (void)idx;
//            const auto[idxP1,idxP2] = idx.value();
//            if( idxP1 == LinkBinTree::invalid_index || idxP2 == LinkBinTree::invalid_index )
//            {
                return false;
//            }
        }
    }
    return true;
}

void Link::jointLink(Link &other)
{

}

void Link::showRawData() const noexcept
{
    tree.showNodes();
}

void Link::appendPort(const Block::Port *port)
{
    if( port->dir == Block::PortDir::Input )
        pasivePorts.push_back(port->getCopy());
    else
        activePort = port->getCopy();
    port->parent->update();
}

void Link::connectLinkToPortAtLastInsertedLine(Block::Port *port,
                                               bool connectAtStart)
{
    if( idxStart == LinkBinTree::invalid_index )
        throw "port connection can not be done (connection point does not exist)";
    auto idx = idxStart;
    if( !connectAtStart )
        idx = idxEnd;
    if( tree.nodes[idx].isEmpty() )
        throw "port connection can not be done (connection point is empty/invalid)";
    tree.nodes[idx].connectionPort.port = port;
    tree.nodes[idx].connectionPort.connected = true;
    port->connectionLink.link = this;
    port->connectionLink.nodeIdx = idx;
    //port->connected = true;
}

void Link::connectLinkToPort(const QPointF &pos, Block::Port *port)
{
    if( port == nullptr )
        throw "connectLinkToPort can not connect to a null port";

    tree.resetChildIter(tree.rootIdx);
    while( auto idx = tree.childIter() )
        if( tree.nodes[idx].point == pos )
        {
            tree.nodes[idx].connectionPort.connected = true;
            tree.nodes[idx].connectionPort.port = port;
            //port->connected = true;
            port->connectionLink.link = this;
            port->connectionLink.nodeIdx = idx;
            return;
        }
}

void Link::connectLinkToPort(uint16_t idx, Block::Port *port)
{
    if( idx == LinkBinTree::invalid_index || port == nullptr )
        throw "connectLinkToPort can not connect to an invalid node or null port";

    if( tree.nodes[idx].isEmpty() )
        throw "connectLinkToPort can not connect to an empty node";

    tree.nodes[idx].connectionPort.connected = true;
    tree.nodes[idx].connectionPort.port = port;
    //port->connected = true;
    port->connectionLink.link = this;
    port->connectionLink.nodeIdx = idx;
}

void Link::disconnectLinkFromPort(uint16_t idx)
{
    if( idx == LinkBinTree::invalid_index )
        throw "disconnectLinkFromPort invalid index";

    if( tree.nodes[idx].isEmpty() )
        throw "disconnectLinkFromPort invalid node";

    auto &port = tree.nodes[idx].connectionPort.port;
    port->connectionLink.link = nullptr;
    port->connectionLink.nodeIdx = LinkBinTree::invalid_index;
    //port->connected = false;
    tree.nodes[idx].connectionPort.connected = false;
    tree.nodes[idx].connectionPort.port = nullptr;
}

} // namespace GuiBlock
