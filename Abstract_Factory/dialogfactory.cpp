#include "dialogfactory.h"

DialogFactory::DialogFactory(SceneObservable *graphicsfactory)
{
    m_graphicsfactory = graphicsfactory;
}

DialogFactory::~DialogFactory(){

}

void DialogFactory::receiveTransmitterProduct(TransmitterProduct* transmitterproduct){
    dynamic_cast<TransmitterProduct*>(m_graphiccomponent)->setPosX(transmitterproduct->getPosX());
    dynamic_cast<TransmitterProduct*>(m_graphiccomponent)->setPosY(transmitterproduct->getPosY());
    dynamic_cast<TransmitterProduct*>(m_graphiccomponent)->setOrientation(transmitterproduct->getOrientation());
    dynamic_cast<TransmitterProduct*>(m_graphiccomponent)->setFrequency(transmitterproduct->getFrequency());
    dynamic_cast<TransmitterProduct*>(m_graphiccomponent)->setPower(transmitterproduct->getPower());
}

void DialogFactory::receiveBuildingProduct(BuildingProduct* buildingproduct){
    dynamic_cast<BuildingProduct*>(m_graphiccomponent)->setPosX(buildingproduct->getPosX());
    dynamic_cast<BuildingProduct*>(m_graphiccomponent)->setPosY(buildingproduct->getPosY());
    dynamic_cast<BuildingProduct*>(m_graphiccomponent)->setConductivity(buildingproduct->getConductivity());
    dynamic_cast<BuildingProduct*>(m_graphiccomponent)->setPermittivity(buildingproduct->getPermittivity());
    dynamic_cast<BuildingProduct*>(m_graphiccomponent)->setModel(buildingproduct->getModel());
    dynamic_cast<BuildingProduct*>(m_graphiccomponent)->setExtremities(buildingproduct->getExtremities());
}

void DialogFactory::receiveReceiverProduct(ReceiverProduct* receiverproduct){
    dynamic_cast<ReceiverProduct*>(m_graphiccomponent)->setPosX(receiverproduct->getPosX());
    dynamic_cast<ReceiverProduct*>(m_graphiccomponent)->setPosY(receiverproduct->getPosY());
    dynamic_cast<ReceiverProduct*>(m_graphiccomponent)->setFrequency(receiverproduct->getFrequency());
}

void DialogFactory::receiveTreeProduct(TreeProduct *treeproduct){
    dynamic_cast<TreeProduct*>(m_graphiccomponent)->setPosX(treeproduct->getPosX());
    dynamic_cast<TreeProduct*>(m_graphiccomponent)->setPosY(treeproduct->getPosY());
}

void DialogFactory::receiveCarProduct(CarProduct *carproduct){
    dynamic_cast<CarProduct*>(m_graphiccomponent)->setPosX(carproduct->getPosX());
    dynamic_cast<CarProduct*>(m_graphiccomponent)->setPosY(carproduct->getPosY());
    dynamic_cast<CarProduct*>(m_graphiccomponent)->setOrientation(carproduct->getOrientation());
    dynamic_cast<CarProduct*>(m_graphiccomponent)->setSpeed(carproduct->getSpeed());
}

TransmitterProduct* DialogFactory::createTransmitterProduct(){
    TransmitterProduct* graphic = dynamic_cast<TransmitterProduct*>(m_graphiccomponent);
    TransmitterProduct* dialog  = new DialogTransmitterProduct(graphic,this);
    return dialog;
}

ReceiverProduct* DialogFactory::createReceiverProduct(){
    ReceiverProduct* graphic = dynamic_cast<ReceiverProduct*>(m_graphiccomponent);
    ReceiverProduct* dialog  = new DialogReceiverProduct(graphic,this);
    return dialog;
}

BuildingProduct* DialogFactory::createBuildingProduct(){
    BuildingProduct* graphic = dynamic_cast<BuildingProduct*>(m_graphiccomponent);
    BuildingProduct* dialog  = new DialogBuildingProduct(graphic,this);
    return dialog;
}

TreeProduct* DialogFactory::createTreeProduct(){
    TreeProduct* graphic = dynamic_cast<TreeProduct*>(m_graphiccomponent);
    TreeProduct* dialog  = new DialogTreeProduct(graphic,this);
    return dialog;
}

CarProduct* DialogFactory::createCarProduct(){
    CarProduct* graphic = dynamic_cast<CarProduct*>(m_graphiccomponent);
    CarProduct* dialog  = new DialogCarProduct(graphic,this);
    return dialog;
}

void DialogFactory::updateNewProperties(GraphicsComponent* graphicscomponent){
    m_graphiccomponent = graphicscomponent;
    switch(int (m_graphiccomponent->getType())){
        case int(GraphicsComponent::TransmitterProduct):
            createTransmitterProduct();
        break;
        case int(GraphicsComponent::BuildingProduct):
            createBuildingProduct();
        break;
        case int(GraphicsComponent::ReceiverProduct):
            createReceiverProduct();
        break;
        case int(GraphicsComponent::TreeProduct):
            createTreeProduct();
        break;
        case int(GraphicsComponent::CarProduct):
            createCarProduct();
        break;
    }
}

void DialogFactory::updateChangeProperties(GraphicsComponent* graphicscomponent){

}