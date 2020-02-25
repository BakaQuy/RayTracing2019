#ifndef DIALOGFACTORY_H
#define DIALOGFACTORY_H

#include "scenefactory.h"
#include "Observer/sceneobserver.h"
#include "Observer/sceneobservable.h"
#include "Product/TransmitterProduct/transmitterproduct.h"
#include "Product/TransmitterProduct/dialogtransmitterproduct.h"
#include "Product/ReceiverProduct/dialogreceiverproduct.h"
#include "Product/BuildingProduct/dialogbuildingproduct.h"
#include "Product/TreeProduct/dialogtreeproduct.h"
#include "Product/CarProduct/dialogcarproduct.h"
#include "Composite/graphicscomponent.h"

class DialogFactory : public SceneFactory, public SceneObserver
{
public:
    DialogFactory(SceneObservable *graphicsfactory);
    ~DialogFactory() override;

    void receiveTransmitterProduct(TransmitterProduct* transmitterproduct);
    void receiveBuildingProduct(BuildingProduct* buildingproduct);
    void receiveReceiverProduct(ReceiverProduct* receiverproduct);
    void receiveTreeProduct(TreeProduct* treeproduct);
    void receiveCarProduct(CarProduct* carproduct);

    TransmitterProduct* createTransmitterProduct() override;
    ReceiverProduct* createReceiverProduct() override;
    BuildingProduct * createBuildingProduct() override;
    TreeProduct * createTreeProduct() override;
    CarProduct *createCarProduct() override;

    void updateNewProperties(GraphicsComponent*) override;
    void updateChangeProperties(GraphicsComponent*) override;

private:
    SceneObservable *m_graphicsfactory;
    GraphicsComponent *m_graphiccomponent;
};

#endif // DIALOGFACTORY_H
