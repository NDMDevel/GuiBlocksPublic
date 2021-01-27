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
public:
    View(QWidget *parent = Q_NULLPTR);
    virtual ~View() {}

    void addBlock();
    void showLastTwoIndexes();
    void forcedConnectedLastBlock();
    void setDebugText(const QString &text);
    void printSeq(){if(links.back())links.back()->printSeq();}

protected:
    void drawBackground(QPainter* painter, const QRectF &r) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

protected slots:
    void paintEvent(QPaintEvent *event) override;

private:
    //internals methods
    void syncLinkPosToBlockPort(const Block *block,
                                const Block::Port *port,
                                const QPointF& mousePos,
                                QPointF &targetPos) const;

    std::tuple<Block*,Block::Port*> getBlockAndPortUnderMouse(const QPoint& mousePos) const;
    QPointF mapToBlock(const Block* block,const QPoint& mousePos) const;
    void moveBlockToFront(Block* block) const;
    void switchLinePath();
    void redrawActiveLink(const QPointF& pos);
    bool blockMousePressHandler(const QPoint& pos);

    bool linkMousePressHandler(const QPointF& pos);
    void linkMouseMoveHandler(const QPointF& pos);
    void linkMouseReleaseHandler(const QPointF& pos);

private:
    Scene   scene;
    QPointF panViewClicPos;
    std::vector<Link*> links;
    Link::LinkPath linkpath = Link::LinkPath::straight;

    class LinkStateMachine
    {
    public:
        enum class States
        {
            waittingPress,
            waittingMoveOrRelease,
            movingAndWaittingRelease,
            appendNewLine,
            drawingAndWaittingPress
        };
    public:
        void setActiveLink(Link *link);
        void mousePress(const QPointF& point);
        void mouseMove(const QPointF& point);
        void mouseRelease(const QPointF& point);
        bool isReadyForMoveEvents(){ return st != States::waittingPress; }
        void cancelDraw();
    private:
        States st = States::waittingPress;
        Link *link = nullptr;
        QPointF pos;
    }linkSM;

    struct DebugType
    {

    }debug;
};

} // namespace GuiBlocks

#endif // VIEW_H
