#ifndef BUILDING_H
#define BUILDING_H

#include <QRectF>
#include <QVector>
#include <QPointF>
#include <QGraphicsRectItem>
#include <QPolygonF>
#include "Wall.h"
//#include "Visualizer.h"
//#include <QGraphicsScene>
#include "AbstractScene.h"
#include "GraphicsBuilding.h"
#include "MovableObject.h"
#include <iostream>


using namespace std;

class Building: public QRectF, public MovableObject
{
public:
    Building(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, AbstractScene *scene);
    Building(const QPointF &p1, const QPointF &p3, AbstractScene *scene);
    ~Building(void);
    void addToScene();
    void moveToPosition(const QPointF &pos);
    void moveWalls();
    Wall *getWalls() ;
    QPointF closestPoint(const QPointF &point);
    vector<Wall *> nearestWalls(const QPointF &point);
    QPolygonF shadow(const QPointF &light);
    vector <QPointF> extremities(const QPointF &light);
    Wall* cornerSecondWall(Wall *wall, const QPointF &corner)const;
    QPointF forDiffraction(Wall *wall, const QPointF &corner)const;
    vector<QPointF> getCorners();
    bool adjacentWall(const QLineF &line,Wall* &wall);

private:
    Wall *m_walls[4]; // Array of pointers.
    GraphicsBuilding *m_graphicsBuilding;
//    QPointF m_p1;
//    QPointF m_p2;
//    QPointF m_p3;
//    QPointF m_p4;
    //Visualizer *m_view;
    AbstractScene *m_scene;



};

#endif // BUILDING_H
