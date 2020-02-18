#ifndef SCENEOBSERVER_H
#define SCENEOBSERVER_H

#include "Composite/graphicscomponent.h"

class SceneObserver{

public:
    virtual ~SceneObserver(){}
    virtual void updateNewProperties(GraphicsComponent*) = 0;
    virtual void updateChangeProperties(GraphicsComponent*) = 0;
};

#endif // SCENEOBSERVER_H
