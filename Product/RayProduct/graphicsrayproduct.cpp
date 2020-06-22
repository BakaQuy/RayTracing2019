#include "graphicsrayproduct.h"

GraphicsRayProduct::GraphicsRayProduct(const QPointF &p1, const QPointF &p2, QGraphicsScene *scene):
    QGraphicsLineItem(p1.x(),p1.y(),p2.x(),p2.y())
{

}

void GraphicsRayProduct::draw(){}

void GraphicsRayProduct::setMathematicalComponent(MathematicalComponent *mathematicalComponent){
    m_mathematicalComponent = mathematicalComponent;
}

MathematicalComponent* GraphicsRayProduct::toMathematicalComponent(){
    return m_mathematicalComponent;
}
