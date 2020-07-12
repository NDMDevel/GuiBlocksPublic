#ifndef VIEW_H
#define VIEW_H

#include <QGraphicsView>
#include "GuiBlocks/Scene.h"
#include "GuiBlocks/Block.h"

namespace GuiBlocks {

class View : public QGraphicsView
{
    Q_OBJECT
private:
    Block *last=nullptr;//just for debug
public:
    View(QWidget *parent = Q_NULLPTR);

    void addBlock();
    void filpLastBlock();
    void forcedConnectedLastBlock();
//    void setScene(Scene *scene);


protected:
//    Scene * getScene(){ return scene; }
    void drawBackground(QPainter* painter, const QRectF& r) override;
    void mouseMoveEvent(QMouseEvent *event) override;


protected slots:
    void paintEvent(QPaintEvent *event) override;
//    void setupViewport(QWidget *widget) override;

private:
    Scene scene;
    QFontMetrics fontMetrics;
    float gridSize;
};

} // namespace GuiBlocks

#endif // VIEW_H
