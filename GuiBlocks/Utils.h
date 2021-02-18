#ifndef GUIBLOCKS_UTILS_H
#define GUIBLOCKS_UTILS_H

#include <QPointF>
#include "GuiBlocks/Block.h"

namespace GuiBlocks {

float   nextEven(float val);
float   max(float val1,float val2);
QPointF nextGridPosition(QPointF pos,float gridSize);
//QPointF nextGridPositionPort(QPointF pos,Block::PortDir dir,float gridSize);
float   nextGridValue(float val,float gridSize);
QPointF nextEvenGridPosition(QPointF pos,float gridSize);
float   nextEvenGridValue(float val,float gridSize);
bool    isInteger(const double &val);
QPoint  pointConvertion(const QPointF &pointf);
QPointF pointConvertion(const QPoint  &point);
double  squareDistance(const QPointF& p1,const QPointF& p2);
bool    equals(double x1,double x2,double maxDiff=1.0e-6);
template<typename T> std::tuple<T,T> minmax(const std::vector<T>& args);

#define debug_msg(str) do{static int count=0;qDebug() << str << count++;}while(0)

} // namespace GuiBlocks

#endif // GUIBLOCKS_UTILS_H
