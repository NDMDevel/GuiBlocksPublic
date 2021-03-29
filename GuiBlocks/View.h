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
    void flipLastBlock();
    void setDebugText(const QString &text);
    void showCurrentLinkData();

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

signals:
    void updateCoords(const QPointF&);

private: //internals methods
    void moveBlockToFront(Block* block) const;
    Block::Port* getBlockPortUnderMouse(QList<QGraphicsItem*> &items,
                                        const QPoint& mousePos) const;

private: //internal helpers
    QPointF mapToBlock(const Block* block,const QPoint& mousePos) const;

private: //internal types
    class UserInterfaceStateMachine
    {
        friend View;
    public:
        enum class States
        {
            waitPress,
            triggerAction,
            startNewLink,
            updateEndPoint,
            waitRelease,
            moveLine,
            appendLine,
            drawSelectionRect,
        };
    public:
        //UserInterfaceStateMachine(GuiBlocks::View *parent,GuiBlocks::Scene &scene,std::vector<Link*> &links):parent(parent),scene(scene),links(links){}
        UserInterfaceStateMachine(GuiBlocks::View *parent):parent(parent){}
        void mousePress(QMouseEvent *event);
        void mouseMove(QMouseEvent *event);
        void mouseRelease(QMouseEvent *event);
        void keyPress(QKeyEvent *event);

        void switchLinkPath();

        void showCurrentLinkData()const;
    private: //internal methods
        enum ActiveItemIdx  //to be used with std::optional<...> activeItem
        {
            PortIdx = 0,
            BlockIdx,
            LinkIdx
        };
        std::optional<std::variant<Block::Port*,Block*,Link*>> getItemUnderMouse(const QPoint &mousePos,bool gridPosition=true) const;
        std::vector<Link*> getLinksUnderMouse(const QPoint &mousePos,bool gridPosition=true) const;
        void updateActiveLine(const QPointF &pos);
        void removeLinkFromScene();
    private:
        GuiBlocks::View *parent;
        std::optional<std::variant<Block::Port*,Block*,Link*>> activeItem;
        States st = States::waitPress;
        Link::LinkPath linkPath = Link::LinkPath::straightThenOrthogonal;
        QPointF startPos;
        QPointF prevPos;
        QPainterPath selectionShape;
        QGraphicsPathItem *selectionShapePtr = nullptr;
        QList<QGraphicsItem*> itemsSelected;
        Link* lastLink = nullptr;
    };//class UserInterfaceStateMachine

private:
    Scene scene;
    UserInterfaceStateMachine uiSM;
    std::vector<Link*> links;
    QPointF panViewClicPos;

private://debug helpers
    struct DebugType
    {
        QPainterPath selection;
        Block *lastBlock;
        std::optional<std::variant<Block::Port*,Block*,Link*>> activeItem;
    }debug;
};

} // namespace GuiBlocks

#endif // VIEW_H
