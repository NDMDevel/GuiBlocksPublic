#include "Editor.h"

namespace GuiBlocks {

Editor::Editor(QWidget *parent)
    : View(parent)
{
    scene = new Scene(this);
    View::setScene(scene);
}

Editor::Editor(Scene *scene, QWidget *parent)
    : View(parent),
      scene(scene)
{}

Editor::~Editor()
{
    if( scene != nullptr )
        delete scene;
}

void Editor::setScene(Scene *scene)
{
    if( scene != nullptr )
        delete scene;
    View::setScene(scene);
    //this->scene = View::getScene();
}

void Editor::addRect(const QRectF &rect, const QPen &pen, const QBrush &brush)
{
    if( scene != nullptr )
        scene->addRect(rect,pen,brush);
}

void Editor::show()
{
    if( scene != nullptr )
        View::show();
}

} // namespace GuiBlocks
