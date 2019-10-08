#ifndef ANTENA_H
#define ANTENA_H
//#include "room.h"
#include <QPointF>
#include <QGraphicsEllipseItem>
#include <QPen>
#include <QBrush>

//class room;
//class QPen;

//class QBrush;

class antena: public QGraphicsEllipseItem/*, public QPointF*/
{
public:
    antena(/*room *scene = 0,*/ QPointF p = QPointF() , int type = 0);
    ~antena();
    int getPosX();
    int getPosY();
    QPointF getPos()const;
    void setPosi(QPointF posi);
private:
    //room *myRoom;
    QPointF pos;
    int antenaType;
    QPen setColor();
};

#endif // ANTENA_H
