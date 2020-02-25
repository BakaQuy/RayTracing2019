#ifndef GRAPHICSTREEPRODUCT_H
#define GRAPHICSTREEPRODUCT_H

#include <QGraphicsPixmapItem>

#include "treeproduct.h"
#include "Composite/graphicscomponent.h"
#include "Abstract_Factory/graphicsfactory.h"

class GraphicsTreeProduct: public QGraphicsPixmapItem, public TreeProduct,public GraphicsComponent
{
public:
    GraphicsTreeProduct(int posX, int posY,GraphicsFactory *graphicsfactory);
    ~GraphicsTreeProduct() override;

    static QPixmap getImage();

    void draw() override;
    bool graphicsSelected() override;
    int getType() override;

    int getPosX() override;
    int getPosY() override;

    void setPosX(int) override;
    void setPosY(int) override;

private:
    GraphicsFactory* m_graphicsfactory;
    int m_posx,m_posy;
};

#endif // GRAPHICSTREEPRODUCT_H
