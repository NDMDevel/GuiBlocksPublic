#ifndef GUIBLOCKS_UTILS_H
#define GUIBLOCKS_UTILS_H

#include <QPointF>

namespace GuiBlocks {

float nextEven(float val);
float max(float val1,float val2);
QPointF nextGridPosition(QPointF pos,float gridSize);
float nextGridValue(float val,float gridSize);
QPointF nextEvenGridPosition(QPointF pos,float gridSize);
float nextEvenGridValue(float val,float gridSize);
bool isInteger(const double &val);

} // namespace GuiBlocks

#endif // GUIBLOCKS_UTILS_H
