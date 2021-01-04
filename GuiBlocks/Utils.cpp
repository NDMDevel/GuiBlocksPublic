#include "Utils.h"
#include <cmath>

namespace GuiBlocks {

float nextEven(float val)
{
    if( val < 0 )
        return 0;
    if( int(val)-val < 0 )
    {
        if( int(val+1)%2 == 0 )
            return int(val+1);
        else
            return 2*int(val+1);
    }
    if( int(val)%2 == 0 )
        return val;
    return 2*int(val);
}

float max(float val1, float val2)
{
    if( val1 > val2 )
        return val1;
    return val2;
}

QPointF nextGridPosition(QPointF pos, float gridSize)
{
    pos /= gridSize;
//    pos.setX(pos.x()/gridSize);
//    pos.setY(pos.y()/gridSize);
    return pos.toPoint()*gridSize;
}

//QPointF nextGridPositionPort(QPointF pos, Block::PortDir dir, float gridSize)
//{
//
//}

float nextGridValue(float val, float gridSize)
{
    int count = val/gridSize;
    float res;
    do
        res = float(count++)*gridSize;
    while( res-val<=0.0f );
    return res;
}

QPointF nextEvenGridPosition(QPointF pos, float gridSize)
{
    return nextGridPosition(pos , 2.0f*gridSize);
}

float nextEvenGridValue(float val, float gridSize)
{
    return nextGridValue(val , 2.0f*gridSize);
}

bool isInteger(const double &val)
{
    return std::abs(std::floor(val)-val) < 1.0e-6;
}

QPoint  pointConvertion(const QPointF &pointf)
{
    QPoint point;
    point.setX(pointf.x());
    point.setY(pointf.y());
    return point;
}

QPointF pointConvertion(const QPoint  &point)
{
    QPoint pointf;
    pointf.setX(point.x());
    pointf.setY(point.y());
    return pointf;
}

//QPoint operator+(const QPoint& p1, const QPoint& p2)
//{
//    QPoint point;
//    point.setX(p1.x()+p2.x());
//    point.setY(p1.y()+p2.y());
//    return point;
//}
//
//QPointF operator+(const QPointF& p1, const QPointF& p2)
//{
//    QPointF point;
//    point.setX(p1.x()+p2.x());
//    point.setY(p1.y()+p2.y());
//    return point;
//}
//
//QPoint operator-(const QPoint& p1, const QPoint& p2)
//{
//    QPoint point;
//    point.setX(p1.x()-p2.x());
//    point.setY(p1.y()-p2.y());
//    return point;
//}
//
//QPointF operator-(const QPointF& p1, const QPointF& p2)
//{
//    QPointF point;
//    point.setX(p1.x()-p2.x());
//    point.setY(p1.y()-p2.y());
//    return point;
//}


} // namespace GuiBlocks
