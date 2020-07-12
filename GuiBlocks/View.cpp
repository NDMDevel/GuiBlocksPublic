#include "View.h"

//GuiBlocks includes
#include "Block.h"
#include "Utils.h"

//std includes
#include <cmath>

//Qt includes
#include <QMouseEvent>
#include <QDebug>

namespace GuiBlocks {

View::View(QWidget *parent)
    : QGraphicsView(parent),
      scene(parent),
      fontMetrics(viewport()->fontMetrics())
{
    //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setOptimizationFlag(QGraphicsView::DontSavePainterState);
    setRenderHints(QPainter::Antialiasing |
                   QPainter::SmoothPixmapTransform |
                   QPainter::HighQualityAntialiasing);

    QBrush brush;
    brush.setColor(StyleGrid::backgroundGridColor);
    brush.setStyle(Qt::BrushStyle::SolidPattern);
    setBackgroundBrush(brush);

    gridSize = StyleGrid::gridSize;

    QGraphicsView::setScene(&scene);
}

void View::addBlock()
{
    Block *b = new Block("Threshold","OverVoltage",/*fontMetrics,*/gridSize);
    last = b;
    b->setFlags(QGraphicsItem::ItemIsMovable);
    b->setAcceptDrops(true);
    b->setAcceptHoverEvents(true);

    b->addPort(Block::PortDir::Input,"Int","One");
//    b->addPort(Block::PortDir::Input,"");
//    b->addPort(Block::PortDir::Output,"");
//    b->addPort(Block::PortDir::Output,"");
//    b->addPort(Block::PortDir::Output,"");
    b->addPort(Block::PortDir::Input,"DQ","PLL Input");
    b->addPort(Block::PortDir::Input,"Float","Input Voltage");
//    b->addPort(Block::PortDir::Input,"Float","InputVoltage");
//    b->addPort(Block::PortDir::Input,"Float","InputVoltage");
//    b->addPort(Block::PortDir::Input,"Float","InputVoltage");
//    b->addPort(Block::PortDir::Input,"Float","InputVoltage");
//    b->addPort(Block::PortDir::Output,"Bool","Alarm");
//    b->addPort(Block::PortDir::Output,"Bool","Alarm");
//    b->addPort(Block::PortDir::Output,"Bool","Alarm");
    b->addPort(Block::PortDir::Output,"Double","Output Voltage");
    b->addPort(Block::PortDir::Output,"char","Letter");
    //b->updateBoundingRect();
    scene.addItem(b);
}

void View::filpLastBlock()
{
    if( last == nullptr )
        return;
    last->toggleBlockOrientation();//Block::BlockOrientation::East);
}

void View::forcedConnectedLastBlock()
{
    if( last == nullptr )
        return;
    static int port = 0;
    last->toggleConnectionPortState(port);
    port++;
}

void View::drawBackground(QPainter* painter, const QRectF& r)
{
    QGraphicsView::drawBackground(painter, r);

    auto drawGrid = [&](double gridStep)
    {
        QRect   windowRect = rect();
        QPointF tl = mapToScene(windowRect.topLeft());
        QPointF br = mapToScene(windowRect.bottomRight());

        double left   = std::floor(tl.x() / gridStep - 0.5);
        double right  = std::floor(br.x() / gridStep + 1.0);
        double bottom = std::floor(tl.y() / gridStep - 0.5);
        double top    = std::floor (br.y() / gridStep + 1.0);

        // vertical lines
        for (int xi = int(left); xi <= int(right); ++xi)
        {
            QLineF line(xi * gridStep, bottom * gridStep,
                        xi * gridStep, top * gridStep );

            painter->drawLine(line);
        }

        // horizontal lines
        for (int yi = int(bottom); yi <= int(top); ++yi)
        {
            QLineF line(left * gridStep, yi * gridStep,
                        right * gridStep, yi * gridStep );
            painter->drawLine(line);
        }
    };

    //  auto const &flowViewStyle = StyleCollection::flowViewStyle();


    QPen pfine(StyleGrid::fineGridColor,
               StyleGrid::fineGridWidth);

    painter->setPen(pfine);
    drawGrid(gridSize);

    QPen p(StyleGrid::coarseGridColor,
           StyleGrid::coarseGridWidth);

    painter->setPen(p);
    drawGrid(10*gridSize);
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
//    qDebug() << "Pos: " << mapToScene(event->pos()).x() << " - " << mapToScene(event->pos()).y()
//             << "Grid: "<< nextGridPosition(mapToScene(event->pos()),gridSize).x() << " - "
//                        << nextGridPosition(mapToScene(event->pos()),gridSize).y();
}

void View::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
}

} // namespace GuiBlocks
