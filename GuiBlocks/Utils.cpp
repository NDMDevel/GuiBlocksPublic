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

double squareDistance(const QPointF &p1, const QPointF &p2)
{
    return std::pow(p1.x()-p2.x(),2.0) + std::pow(p1.y()-p2.y(),2.0);
}

bool equals(double x1, double x2, double maxDiff)
{
    return std::abs(x1-x2) < maxDiff;
}

template<typename T>
std::tuple<T,T> minmax(const std::vector<T>& args)
{
    if( args.size() == 0 )
        throw("minmax empty arg");

    T min = args[0];
    T max = args[0];
    if( args.size() != 1 )
        for( uint32_t idx=1 ; idx<args.size() ; idx++  )
        {
            min = std::min(min,args[idx]);
            max = std::max(max,args[idx]);
        }
    return {min,max};
}

} // namespace GuiBlocks
