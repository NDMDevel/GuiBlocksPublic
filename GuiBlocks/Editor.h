#ifndef GUIBLOCKS_BLOCKEDITOR_H
#define GUIBLOCKS_BLOCKEDITOR_H

#include "GuiBlocks/View.h"
#include "GuiBlocks/Scene.h"

namespace GuiBlocks {

using GuiBlocks::View;
using GuiBlocks::Scene;

class Editor : public View
{
public:
    Editor(QWidget *parent = nullptr);
    Editor(Scene *scene,QWidget *parent = nullptr);
    virtual ~Editor();

    void setScene(Scene *scene);
    void addRect(const QRectF &rect,
                 const QPen &pen = QPen(),
                 const QBrush &brush = QBrush());
    void show();

private:
    Scene *scene;
};

} // namespace GuiBlocks

#endif // GUIBLOCKS_BLOCKEDITOR_H
