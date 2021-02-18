#include "Style.h"
#include <QColor>
#include <QSize>

namespace GuiBlocks {

float  StyleText::gapTypeToBorderGridSizePercent = 0.7f;
float  StyleText::gapTextToBorderGridSizePercent = 0.25f;
QFont  StyleText::blockTypeFont = QFont("Ubuntu", 11);
QColor StyleText::blockTypeColor = Qt::red;
QFont  StyleText::blockNameFont = QFont("Ubuntu", 11);
QColor StyleText::blockNameColor = Qt::blue;
QFont  StyleText::blockHintFont = QFont("Ubuntu", 11);
QColor StyleText::blockHintColor = Qt::black;

QSizeF StyleBlockShape::connectorSizeGridSizePercent(0.7,0.7);
QColor StyleBlockShape::blockRectBorderColor        = "#202020";//Qt::gray;
QColor StyleBlockShape::blockRectBorderColorOnHover = Qt::black;
QColor StyleBlockShape::shadowColor                 = "#202020";//Qt::gray;
QColor StyleBlockShape::shadowColorOnHover          = Qt::black;
QLinearGradient StyleBlockShape::customFillGradient = QLinearGradient(0.5,0,0.5,1);
QColor StyleBlockShape::blockRectFillColor1         = Qt::white;
QColor StyleBlockShape::blockRectFillColor2         = "#CECECE";
float  StyleBlockShape::roundingXWidthPercent       = 0.02f;
float  StyleBlockShape::roundingYWidthPercent       = 0.02f;
float  StyleBlockShape::opacity                     = 1.0f;
QColor StyleBlockShape::inputConnectorBorderColor   = Qt::black;
QColor StyleBlockShape::outputConnectorBorderColor  = Qt::black;
QColor StyleBlockShape::inputConnectorFillColor     = Qt::green;
QColor StyleBlockShape::outputConnectorFillColor    = Qt::red;

float  StyleGrid::gridSize = 20.0f;
QColor StyleGrid::backgroundGridColor = Qt::white;
float  StyleGrid::fineGridWidth       = 1.0f;
QColor StyleGrid::fineGridColor       = Qt::lightGray;
float  StyleGrid::coarseGridWidth     = 1.0f;
QColor StyleGrid::coarseGridColor     = Qt::gray;

QColor StyleLink::normalColor        = Qt::blue;
//QColor StyleLink::selectedColor    = Qt::magenta;
float  StyleLink::width              = 4.0f;
QColor StyleLink::shadowColor   = "#202020";
Qt::PenStyle    StyleLink::normalLine = Qt::SolidLine;
Qt::PenCapStyle StyleLink::normalCap  = Qt::RoundCap;

} // namespace GuiBlocks
