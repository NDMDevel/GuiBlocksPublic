#include "Utils.h"
#include <cmath>

namespace GuiBlocks {

double nextEven(double val)
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

double max(double val1, double val2)
{
    if( val1 > val2 )
        return val1;
    return val2;
}

QPointF nextGridPosition(QPointF pos, double gridSize)
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

double nextGridValue(double val, double gridSize)
{
    int count = int(val/gridSize);
    double res;
    do
        res = double(count++)*gridSize;
    while( res-val<=0.0 );
    return res;
}

QPointF nextEvenGridPosition(QPointF pos, double gridSize)
{
    return nextGridPosition(pos , 2.0*gridSize);
}

double nextEvenGridValue(double val, double gridSize)
{
    return nextGridValue(val , 2.0*gridSize);
}

double nextOddGridValue(double val, double gridSize)
{
    int count = int(val/gridSize);
    double res;
    do
        res = double(count++)*gridSize;
    while( res-val<=0.0 || (count-1)%2==0 );
    return res;
}

bool isInteger(const double &val)
{
    return std::abs(std::floor(val)-val) < 1.0e-6;
}

QPoint  pointConvertion(const QPointF &pointf)
{
    QPoint point;
    point.setX(int(pointf.x()));
    point.setY(int(pointf.y()));
    return point;
}

QPointF pointConvertion(const QPoint  &point)
{
    QPoint pointf;
    pointf.setX(point.x());
    pointf.setY(point.y());
    return pointf;
}

double squareDistance(const QPointF &p1, const QPointF &p2)
{
    return std::pow(p1.x()-p2.x(),2.0) + std::pow(p1.y()-p2.y(),2.0);
}

bool equals(double x1, double x2, double maxDiff)
{
    return std::abs(x1-x2) < maxDiff;
}


} // namespace GuiBlocks
