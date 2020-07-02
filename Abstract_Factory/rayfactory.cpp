#include "rayfactory.h"

RayFactory::RayFactory(bool visible, QGraphicsScene *scene)
{
    m_visibleRays = visible;
    m_scene = scene;
}

MathematicalRayProduct* RayFactory::createRay(const QPointF &p1, const QPointF &p2, double Tetai, int indWall){
    MathematicalRayProduct* mathematicalComponent;
    if(m_visibleRays){

        //cout<<"Ray created in factory"<<endl;
        mathematicalComponent = new MathematicalRayProduct(p1, p2, Tetai, indWall);
        GraphicsRayProduct* graphicsComponent = new GraphicsRayProduct(p1, p2,m_scene);
        mathematicalComponent->attachObservable(graphicsComponent);
    }
    else{
        mathematicalComponent = new MathematicalRayProduct();
    }
    return mathematicalComponent;
}
