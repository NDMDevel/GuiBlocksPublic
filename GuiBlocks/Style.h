#ifndef GUIBLOCKS_STYLE_H
#define GUIBLOCKS_STYLE_H

#include <QFont>
#include <QColor>
#include <QLinearGradient>

namespace GuiBlocks {

class StyleText
{
public:
    StyleText() = delete;

    static float  gapTypeToBorderGridSizePercent;
    static float  gapTextToBorderGridSizePercent;
    static QFont  blockTypeFont;
    static QColor blockTypeColor;
    static QFont  blockNameFont;
    static QColor blockNameColor;
    static QFont  blockHintFont;
    static QColor blockHintColor;
};

class StyleBlockShape
{
public:
    StyleBlockShape() = delete;

    static QSizeF connectorSizeGridSizePercent;
    static QColor blockRectBorderColor;
    static QColor blockRectBorderColorOnHover;
    static QColor shadowColor;
    static QColor shadowColorOnHover;
    static QLinearGradient customFillGradient;
    static QColor blockRectFillColor1;
    static QColor blockRectFillColor2;
    static float  roundingXWidthPercent;
    static float  roundingYWidthPercent;
    static float  opacity;
    static QColor inputConnectorBorderColor;
    static QColor outputConnectorBorderColor;
    static QColor inputConnectorFillColor;
    static QColor outputConnectorFillColor;
};

class StyleGrid
{
public:
    StyleGrid() = delete;

    static float  gridSize;
    static QColor backgroundGridColor;
    static float  fineGridWidth;
    static QColor fineGridColor;
    static float  coarseGridWidth;
    static QColor coarseGridColor;
};

class StyleLink
{
public:
    StyleLink() = delete;

    static QPen   normalPen;
    static QColor normalColor;
//    static QColor selectedColor;
    static float  width;
    static QColor shadowColor;
    static Qt::PenStyle  normalLine;
    static Qt::PenCapStyle normalCap;
};


} // namespace GuiBlocks

#endif // GUIBLOCKS_STYLE_H
