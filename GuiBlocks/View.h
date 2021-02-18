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
    virtual ~View() override{}

    void addBlock();
    void forcedConnectedLastBlock();
    void setDebugText(const QString &text);

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

private: //internals methods
//    void syncLinkPosToBlockPort(const Block *block,
//                                const Block::Port *port,
//                                const QPointF& mousePos,
//                                QPointF &targetPos) const;

    void moveBlockToFront(Block* block) const;
    Block::Port* getBlockPortUnderMouse(QList<QGraphicsItem*> &items,
                                        const QPoint& mousePos) const;

private: //internal helpers
    QPointF mapToBlock(const Block* block,const QPoint& mousePos) const;

private: //internal types
    class UserInterfaceStateMachine
    {
    public:
        enum class States
        {
            waitPress,
            triggerAction,
            startNewLink,
            updateEndPoint,
            waitRelease,
            moveLine,
            appendLine
        };
    public:
        //UserInterfaceStateMachine(GuiBlocks::View *parent,GuiBlocks::Scene &scene,std::vector<Link*> &links):parent(parent),scene(scene),links(links){}
        UserInterfaceStateMachine(GuiBlocks::View *parent):parent(parent){}
        void mousePress(QMouseEvent *event);
        void mouseMove(QMouseEvent *event)noexcept;
        void mouseRelease(QMouseEvent *event);
        void keyPress(QKeyEvent *event);

        void switchLinkPath();
    private: //internal methods
        enum ActiveItemIdx  //to be used with std::optional<...> activeItem
        {
            PortIdx = 0,
            BlockIdx,
            LinkIdx
        };
        std::optional<std::variant<Block::Port*,Block*,Link*>> getItemUnderMouse(const QPoint &mousePos) const;
        void updateActiveLine(const QPointF &pos);
    private:
        GuiBlocks::View *parent;
        std::optional<std::variant<Block::Port*,Block*,Link*>> activeItem;
        States st = States::waitPress;
        Link::LinkPath linkPath = Link::LinkPath::straightThenOrthogonal;
        QPointF startOrPrevMousePos;
    };

private:
    Scene scene;
    UserInterfaceStateMachine uiSM;
    std::vector<Link*> links;
    QPointF panViewClicPos;

    class LinkStateMachine
    {
    public:
        enum class States
        {
            waittingPress,
            validatePress,
            drag,
            waittingMove,
            update,
            waittingRelease
        };
    public:
        void setActiveLink(Link *link);
        Link* getActiveLink(){ return link; }
        void mousePress(const QPointF& pos);
        void mouseMove(const QPointF& pos);
        void mouseRelease(const QPointF& pos);
        void cancelDraw();
        void switchLinkPath();

    private:
//        void appendLine(const QPointF &pos);
//        std::optional<QPointF> computeMidPoint(const QPointF &pos);

    private:
        Link::LinkPath linkPath = Link::LinkPath::verticalThenHorizontal;
        States st = States::waittingPress;
        Link *link = nullptr;
        QPointF prevPos;
        QPointF firstPos;
    }linkSM;

    struct DebugType
    {

    }debug;
};

} // namespace GuiBlocks

#endif // VIEW_H
