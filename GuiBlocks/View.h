#ifndef VIEW_H
#define VIEW_H

#include <QGraphicsView>
#include "GuiBlocks/Scene.h"
#include "GuiBlocks/Block.h"
#include "GuiBlocks/Link.h"
#include <tuple>

namespace GuiBlocks {

class View : public QGraphicsView
{
    Q_OBJECT
private:
    Block *last = nullptr;//for debug
    Link  *link = nullptr;//for debug
    QGraphicsLineItem *vline = nullptr;   //mouse tracker
    QPointF startPos,endPos;//for debug
    //QVector<Link*> lastLink;

public:
    View(QWidget *parent = Q_NULLPTR);

    void addBlock();
    void flipLastBlock();
    void forcedConnectedLastBlock();
//    void setScene(Scene *scene);


protected:
//    Scene * getScene(){ return scene; }
    void drawBackground(QPainter* painter, const QRectF &r) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;



protected slots:
    void paintEvent(QPaintEvent *event) override;
//    void setupViewport(QWidget *widget) override;

private:
    //internals methods
    void syncLinkPosToBlockPort(const Block *block,
                                const Block::Port *port,
                                const QPointF& mousePos,
                                QPointF &targetPos) const;

    std::tuple<Block*,Block::Port*> getBlockAndPortUnderMouse(const QPoint& mousePos) const;
    QPointF mapToBlock(const Block* block,const QPoint& mousePos) const;
    void moveBlockToFront(Block* block) const;

private:
    Scene   scene;
    QFontMetrics fontMetrics;
    float   gridSize;
    QPointF clicPos;
    bool    drawingLink = false;
};

} // namespace GuiBlocks

#endif // VIEW_H
