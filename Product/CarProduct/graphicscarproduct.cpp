#include "graphicscarproduct.h"

GraphicsCarProduct::GraphicsCarProduct(int posX, int posY, double orientation, double speed, GraphicsFactory *graphicsfactory):
    m_graphicsfactory(graphicsfactory),m_posx(posX),m_posy(posY),m_orientation(orientation),m_speed(speed)
{
    QPixmap icon(":/Images/Car.png");
    setPixmap(icon);
    setRotation(m_orientation);
    setOffset(-icon.width()/2,-icon.height()/2);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    draw();
}

GraphicsCarProduct::~GraphicsCarProduct(){

}

QPixmap GraphicsCarProduct::getImage(){
    QPixmap icon(":/Images/Car.png");
    return icon;
}

int GraphicsCarProduct::getType(){
    return int(GraphicsComponent::CarProduct);
}

bool GraphicsCarProduct::graphicsSelected(){
    return isSelected();
}

void GraphicsCarProduct::draw(){
    m_graphicsfactory->addItem(this);
}

int GraphicsCarProduct::getPosX(){
    return m_posx;
}

int GraphicsCarProduct::getPosY(){
    return m_posy;
}

double GraphicsCarProduct::getOrientation(){
    return m_orientation;
}

double GraphicsCarProduct::getSpeed(){
    return m_speed;
}

void GraphicsCarProduct::setPosX(int posX){
    m_posx = posX;
    setPos(m_posx,m_posy);
}

void GraphicsCarProduct::setPosY(int posY){
    m_posy = posY;
    setPos(m_posx,m_posy);
}

void GraphicsCarProduct::setOrientation(double orientation){
    m_orientation = orientation;
    setRotation(m_orientation);
}

void GraphicsCarProduct::setSpeed(double speed){
    m_speed = speed;
}
