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

    static double gapTypeToBorderGridSizePercent;
    static double gapTextToBorderGridSizePercent;
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
    static double roundingXWidthPercent;
    static double roundingYWidthPercent;
    static double opacity;
    static QColor inputConnectorBorderColor;
    static QColor outputConnectorBorderColor;
    static QColor inputConnectorFillColor;
    static QColor outputConnectorFillColor;
};

class StyleGrid
{
public:
    StyleGrid() = delete;

    static double  gridSize;
    static QColor backgroundGridColor;
    static double  fineGridWidth;
    static QColor fineGridColor;
    static double  coarseGridWidth;
    static QColor coarseGridColor;
};

class StyleLink
{
public:
    StyleLink() = delete;

    static QColor normalColor;
    static double  width;
    static QColor shadowColor;
    static Qt::PenStyle  normalLine;
    static Qt::PenCapStyle normalCap;
};

class StyleSelection
{
public:
    StyleSelection() = delete;

    static QColor normalFillColor;
    static QColor cuttedFillColor;
//    static QColor normalLineColor;
//    static QColor cuttedLineColor;
    static double width;
    static double opacity;
    static Qt::PenStyle    normalLine;
    static Qt::BrushStyle  fillStyle;
};


} // namespace GuiBlocks

#endif // GUIBLOCKS_STYLE_H
