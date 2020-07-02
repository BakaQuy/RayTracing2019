#include "mapgenerator.h"

MapGenerator::MapGenerator(QRectF mapBoundary):m_mapBoundary(mapBoundary)
{

}

void MapGenerator::generateMap(){
//    for(int i=0;i<round(m_mapBoundary.height()/50);i++){
//        QLineF horizontalLine(0,i*50,m_mapBoundary.right(),i*50);
//        m_scene->addLine(horizontalLine);
//    }

//    for(int i=0;i<round(m_mapBoundary.height()/50);i++){
//        QLineF verticalLine(i*50,m_mapBoundary.top(),i*50,m_mapBoundary.bottom());
//        m_scene->addLine(verticalLine);
//    }

    m_horizontalStreets.clear();
    m_verticalStreets.clear();

    int streetsDistance = 200;
    for(int i=0;i<round(m_mapBoundary.height()/streetsDistance);i++){
        int random1 = rand()%100;
        int random2 = rand()%100;
        QLineF *horizontalLine = new QLineF(0,i*streetsDistance+random1,m_mapBoundary.right(),i*streetsDistance+random2);

        int random3 = rand()%100;
        int random4 = rand()%100;
        QLineF *verticalLine = new QLineF(i*streetsDistance+random3,m_mapBoundary.top(),i*streetsDistance +random4,m_mapBoundary.bottom());

//        m_scene->addLine(*horizontalLine);
//        m_scene->addLine(*verticalLine);

        m_horizontalStreets.push_back(horizontalLine);
        m_verticalStreets.push_back(verticalLine);

    }

    int streetWidth = 30;

    for(int i =0;i<round(m_mapBoundary.height()/streetsDistance)-1;i++){
        for(int j =0;j<round(m_mapBoundary.width()/streetsDistance)-1;j++){
            QPointF intersectionPoint1;
            m_horizontalStreets.at(j)->intersect(*m_verticalStreets.at(i),&intersectionPoint1);

            QPointF intersectionPoint2;
            m_horizontalStreets.at(j+1)->intersect(*m_verticalStreets.at(i),&intersectionPoint2);

            QPointF intersectionPoint3;
            m_horizontalStreets.at(j)->intersect(*m_verticalStreets.at(i+1),&intersectionPoint3);

            QPointF intersectionPoint4;
            m_horizontalStreets.at(j+1)->intersect(*m_verticalStreets.at(i+1),&intersectionPoint4);



//            intersectionPoint1.setX(round(intersectionPoint1.x()+streetWidth));
//            intersectionPoint1.setY(round(intersectionPoint1.y()+streetWidth));

//            intersectionPoint2.setX(round(intersectionPoint2.x()+streetWidth));
//            intersectionPoint2.setY(round(intersectionPoint2.y()-streetWidth));

//            intersectionPoint3.setX(round(intersectionPoint3.x()-streetWidth));
//            intersectionPoint3.setY(round(intersectionPoint3.y()+streetWidth));

//            intersectionPoint4.setX(round(intersectionPoint4.x()-streetWidth));
//            intersectionPoint4.setY(round(intersectionPoint4.y()-streetWidth));

            intersectionPoint1.setX(intersectionPoint1.x()+streetWidth);
            intersectionPoint1.setY(intersectionPoint1.y()+streetWidth);

            intersectionPoint2.setX(intersectionPoint2.x()+streetWidth);
            intersectionPoint2.setY(intersectionPoint2.y()-streetWidth);

            intersectionPoint3.setX(intersectionPoint3.x()-streetWidth);
            intersectionPoint3.setY(intersectionPoint3.y()+streetWidth);

            intersectionPoint4.setX(intersectionPoint4.x()-streetWidth);
            intersectionPoint4.setY(intersectionPoint4.y()-streetWidth);

            QPolygonF buildingForm;
            buildingForm<<intersectionPoint1<<intersectionPoint2<<intersectionPoint4<<intersectionPoint3<<intersectionPoint1;

            MathematicalProduct* building = m_buildingFactory->createMathematicalProduct(buildingForm);
            //MathematicalProduct* building = m_buildingFactory->createGraphicsProduct(buildingForm);

            m_products.push_back(building);



            //m_scene->addPolygon(buildingForm);

        }
        //addCars();
    }
}

void MapGenerator::addCars(){
    QPointF carPosition;
    for(int i=0;i<m_horizontalStreets.size();i++){
        int random = rand()%(int)m_horizontalStreets.at(i)->length();
        QLineF line = *m_horizontalStreets.at(i);
        line.setLength(random);
        carPosition = line.p2();
        MathematicalProduct* car = m_carFactory->createMathematicalProduct(carPosition.x(),carPosition.y());
        m_products.push_back(car);
        m_cars.push_back((MathematicalCarProduct*)car);
        ((MathematicalCarProduct*)car)->setRoad(line);

    }

    for(int j=0;j<m_verticalStreets.size();j++){
        int random = rand()%(int)m_verticalStreets.at(j)->length();
        QLineF line = *m_verticalStreets.at(j);
        line.setLength(random);
        carPosition = line.p2();
        MathematicalProduct* car = m_carFactory->createMathematicalProduct(carPosition.x(),carPosition.y());
        m_products.push_back(car);
        m_cars.push_back((MathematicalCarProduct*)car);
        ((MathematicalCarProduct*)car)->setRoad(line);

        //thread *thread_obj = new thread (&MapGenerator::moveCar,ref(*(MathematicalCarProduct*)car),ref(*m_verticalStreets.at(j)));
        //thread_obj->join();
        //thread_obj.detach();

    }
}

void MapGenerator::moveCar(MathematicalCarProduct& car, QLineF& street){
    while(true){
        car.setPosX(car.getPosX() + street.dx()/street.length());
        car.setPosY(car.getPosY() + street.dy()/street.length());
        this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void MapGenerator::moveCars(MapGenerator &mapGenerator){

    vector<MathematicalCarProduct*> cars = mapGenerator.getCars();
    while(true){
        for(int i=0;i< cars.size();i++){
            cars.at(i)->moveCar();
        }
        this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

}

vector<MathematicalBuildingProduct*> MapGenerator::getBuildings() const{
    return m_buildings;
}

vector<MathematicalCarProduct*> MapGenerator::getCars() const{
    return m_cars;
}

vector<MathematicalProduct*> MapGenerator::getProducts() const{
    return m_products;
}

void MapGenerator::setBuildingFactory(BuildingFactory *buildingFactory){
    m_buildingFactory = buildingFactory;
}

void MapGenerator::setCarFactory(CarFactory *carFactory){
    m_carFactory = carFactory;
}
