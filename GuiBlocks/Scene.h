#ifndef SCENE_H
#define SCENE_H

#include <QGraphicsScene>

namespace GuiBlocks {

class Scene : public QGraphicsScene
{
    Q_OBJECT
public:
    Scene(QObject *parent = nullptr);
    virtual ~Scene(){}

};

} // namespace GuiBlocks

#endif // SCENE_H
