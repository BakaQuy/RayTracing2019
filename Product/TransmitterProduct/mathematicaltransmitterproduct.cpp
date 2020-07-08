#include "mathematicaltransmitterproduct.h"
MathematicalTransmitterProduct::MathematicalTransmitterProduct(int posX, int posY):QPointF(posX,posY)
{
    m_type = "Transmitter";
    px_to_meter = 0.1;
    m_radius = 500;
    m_zone = buildCoverage();
    m_frequency = 26e9;
    m_bandwidth = 100e6;
    m_row = 1;
    m_column = 1;
    m_kind = dipole;
    m_power = 2;
    m_orientation = 0;
    lambda = c/m_frequency;
    epsilonWallRel = 5;
    active_pathloss = true;
    compute_pathloss = true;
}

MathematicalTransmitterProduct::~MathematicalTransmitterProduct(){

}

void MathematicalTransmitterProduct::drawRays(){

}

QPolygonF MathematicalTransmitterProduct::buildCoverage(){
    QPolygonF coverage;
    for(int i=0;i<16;i++){
        coverage<<QPointF(x()+m_radius*cos(M_PI*i/8),y()+m_radius*sin(M_PI*i/8));
    }
    return coverage;
}


void MathematicalTransmitterProduct::drawRays(ProductObservable *productObservable, bool draw){
    if(draw){
        for(int i=0;i<m_receiversRays[productObservable].size();i++){
            for(int j=0;j<m_receiversRays[productObservable].at(i)->size();j++){
                m_receiversRays[productObservable].at(i)->at(j)->draw();
            }
        }
    }
    else{
        for(int i=0;i<m_receiversRays[productObservable].size();i++){
            for(int j=0;j<m_receiversRays[productObservable].at(i)->size();j++){
                m_receiversRays[productObservable].at(i)->at(j)->erase();
            }
        }
    }
}



complex <double> MathematicalTransmitterProduct::computeEMfield(vector<MathematicalRayProduct*> *rayLine,ProductObservable* receiver){
    /* One vector<ray*> is one multi-path componant, the size of the vector determine the n-level we are in, for each ray only the power in the last ray is transmitted to
     * the receptor. As seen in the power formula, n rays -> n-1 additions to the power.
     *
     * This function gives the electrical field, generated by one MPC, at the receiver. The electric field is // to the dipole antenna since we only consider relections
     * off the buildings. The electric field is not // for the reflexion with the ground though. This is taken into account in the function computePrx.
     */

    complex <double> i(0.0, 1.0);
    int amountSegment = rayLine->size();
    double completeLength = 0.0;
    double theta = 0.0;
    double R = 1;
    complex <double> Efield = 0.0;
    MathematicalRayProduct* currentRay;
    for (int i=0; i<amountSegment; i++){
        currentRay = rayLine->at(i);
        theta = currentRay->getTheta();
        if((i != amountSegment-1)){   // The last segment, the one that reach the receptor does not have a rebound
            double thetaI = abs(currentRay->getTetai());
            R *= computeReflexionPer(thetaI,epsilonWallRel);
        }
        completeLength += currentRay->getMeterLength(); // Get each length of each ray segment after the meter conversion (1px == 1dm)
//        cout<<"lenght of the ray: "<<currentRay->length()<<endl;
        //cout<<"lenght of the ray: "<<rayLine->at(i)->length()<<endl;
    }
    double Ia = sqrt(2.0*m_power/Ra); // Ia could be changed for Beamforming application (add exp)
    double a = R * ((Zvoid*Ia)/(2.0*M_PI))/completeLength;
    Efield = i * a * exp(-i*(2.0*M_PI/lambda)*completeLength);

    if(amountSegment==1){
        // Adding the ground component
        complex <double> groundEfield = this->computeEfieldGround(receiver); // Compute the electrical field from the ray reflected off the ground
        double dist = distance(receiver);
        double thetaI = atan(antennaHeight/(dist/2))+M_PI/2;
        Efield += groundEfield*(cos(M_PI/2*cos(thetaI))/sin(thetaI));
    }
    QPointF p1 = rayLine->at(rayLine->size()-1)->p2();
    if (p1.x()-1 != this->getPosX() && p1.y() != this->getPosY()){
        vector<double> point(2);
        point[0] = p1.x();
        point[1] = p1.y();
        m_attenuation[receiver][point] = R/completeLength;
        m_raylength[receiver][point] = completeLength;
    }
//    if(amountSegment==1){
//        this->minLength = completeLength; // for delay spread computation
//        this->LOS = pow(a,2);
//    } else{
//        this->NLOS += pow(a,2);
//    }
//    if(completeLength > this->maxLength){
//        this->maxLength = completeLength; // for delay spread computation
//    }
//    // Store ray parameter for Physical impulse response
//    if(computePhysicalResponse){
//        // Store attenuation a and distance completeLength
//        channelData[rayNumber] = R/completeLength;
//        channelData[rayNumber+20] = completeLength;
//        double degangle = round((theta+direction)*180/M_PI);
//        double radangle = degangle*M_PI/180;
//        spectrumField[specNumber] = Efield;
//        spectrumAngle[specNumber] = (2.0*M_PI/lambda)*speedReal*cos(radangle);
//        int save = specNumber;
//        for (int j = 0; j < save; j++){
//            if (spectrumAngle[j] == spectrumAngle[save]){
//                spectrumField[j] += Efield;
//                specNumber -= 1;
//            }
//        }
//        specNumber += 1;
//        rayNumber += 1;
//    }
    return Efield;
}

complex <double> MathematicalTransmitterProduct::computeEfieldGround(ProductObservable* receiver){
    // Compute the electrical field, at the receiver, induced by the ray reflected off the ground.
    // To Do: check if there is a wall between the TX and RX
    double distance = this->distance(receiver); // conversion (1px == 2cm)
    double thetaG = atan((distance/2)/antennaHeight);
    double thetaI = M_PI - thetaG;
    double R = computeReflexionPar(thetaG,epsilonWallRel);
    double completeLength = distance/sin(thetaG);
    //if(completeLength > this->maxLength) this->maxLength = completeLength; // for delay spread computation
    complex <double> i(0.0, 1.0);
    double Ia = sqrt(2*m_power/Ra); // Ia could be changed for Beamforming application
    double a = R * ((Zvoid*Ia)/(2*M_PI)) * (cos(M_PI/2*cos(thetaI))/sin(thetaI))/completeLength;
    complex <double> Efield = i * a * exp(-i*(2.0*M_PI/lambda)*completeLength);
    vector<double> point(2);
    point[0] = 0;
    point[1] = 0;
    m_attenuation[receiver][point] = R/completeLength;
    m_raylength[receiver][point] = completeLength;
//    this->NLOS += pow(a,2);

//    // Store ray parameter for Physical impulse response
//    if(computePhysicalResponse){
//        // Store attenuation a and distance completeLength
//        channelData[rayNumber] = R/completeLength;
//        channelData[rayNumber+20] = completeLength;
//        rayNumber += 1;
//    }

    return Efield;
}

double MathematicalTransmitterProduct::distance(ProductObservable* receiver){

    /*
     * For display, it calcultes the direct Euclidian distance from the receiver to the emettor, displays the results on the UI
     */

    double x1 = x();
    double y1 = y();
    double x2 = receiver->getPos()->x();
    double y2 = receiver->getPos()->y();

    return sqrt(pow((x2-x1),2)+pow((y2-y1),2))*px_to_meter; // conversion (1px == 1dm)
}

complex<double> MathematicalTransmitterProduct::computeDiffractedEfield(vector<MathematicalRayProduct *> *rayLine){

    // Direct distance between the receiver and the transmitter
//    double direct_dist = sqrt(pow(transmitter->getPosition().x()-m_Receiver->getPosX(),2)
//                              + pow(transmitter->getPosition().y()-m_Receiver->getPosY(),2)); //convertir px to cm?

    double direct_dist = sqrt(pow(rayLine->at(1)->p1().x()-rayLine->at(0)->p2().x(),2)
                              + pow(rayLine->at(1)->p1().y()-rayLine->at(0)->p2().y(),2)); //convertir px to cm?

    complex<double> Efield =0.0;
    complex<double> F =0.0;
    complex <double> i(0.0, 1.0);

    // The length defference between the path going through the tip of the obstacle, and the direct path.

    double delta_r = (rayLine->at(0)->length()+rayLine->at(1)->length() - direct_dist)*pow(10, -1.0);

    double nu = sqrt(2*2*M_PI/lambda*delta_r/M_PI);
    // The ITU's approximation for |F(nu)|^2

    double absF = pow(10,-6.9/40)/sqrt((sqrt(pow(nu-0.1,2)+1)+nu-0.1));
    double argF = -M_PI/4 - pow(nu,2)*M_PI/2;
    F = absF*exp(i*argF);

    //ray* directRay = new ray(m_Receiver->getPosX(),m_Receiver->getPosY(),Transmitter->getPosX(),Transmitter->getPosY(),0,0);

    //MathematicalRayProduct* directRay = new MathematicalRayProduct(rayLine->at(0).p2(),rayLine->at(1).p1(),0,0);
    Line* directRay = new Line(rayLine->at(0)->p2(),rayLine->at(1)->p1());

    //MathematicalRayProduct* directRay = m_rayFactory->createRay(rayLine->at(0).p2(),rayLine->at(1).p1(),0,0);
    double Ia = sqrt(2*m_power/Ra); // Ia could be changed for Beamforming application (add exp)
    Efield =-i  * ((Zvoid*Ia)/(2*M_PI)) * (exp(-i*(2.0*M_PI/lambda)*directRay->getMeterLength())/directRay->getMeterLength());
    Efield = F*Efield;
//    cout<<"Fresnel integral: "<<F<<endl;
//    cout<<"Diffracted field: "<<Efield<<endl;
    delete directRay;
    return Efield;
}

complex<double> MathematicalTransmitterProduct::computeDiffractedTreeEfield(vector<QLineF> rayLine){

    // Direct distance between the receiver and the transmitter

    double direct_dist = sqrt(pow(rayLine.at(1).p1().x()-rayLine.at(0).p2().x(),2)
                              + pow(rayLine.at(1).p1().y()-rayLine.at(0).p2().y(),2)); //convertir px to cm?

    complex<double> Efield =0.0;
    complex<double> F =0.0;
    complex <double> i(0.0, 1.0);

    // The length defference between the path going through the tip of the obstacle, and the direct path.

    double delta_r = (rayLine.at(0).length()+rayLine.at(1).length() - direct_dist)*pow(10, -1.0);

    double nu = sqrt(2*2*M_PI/lambda*delta_r/M_PI);
    // The ITU's approximation for |F(nu)|^2

    double absF = pow(10,-6.9/40)/sqrt((sqrt(pow(nu-0.1,2)+1)+nu-0.1));
    double argF = -M_PI/4 - pow(nu,2)*M_PI/2;
    F = absF*exp(i*argF);

    Line* directRay = new Line(rayLine.at(0).p2(),rayLine.at(1).p1());

    double Ia = sqrt(2*m_power/Ra); // Ia could be changed for Beamforming application (add exp)
    Efield =-i  * ((Zvoid*Ia)/(2*M_PI)) * (exp(-i*(2.0*M_PI/lambda)*directRay->getMeterLength())/directRay->getMeterLength());
    Efield = F*Efield;
    delete directRay;
    return Efield;
}



vector<vector<QLineF> > MathematicalTransmitterProduct::buildTreeRays(QPointF* Rx,MathematicalTreeProduct* tree){
    float radius = tree->getRadius();
    QPointF treeCenter;
    treeCenter.setX(tree->getPosX());
    treeCenter.setY(tree->getPosY());

    QLineF directPath(*this,*Rx);
    directPath.setAngle(directPath.angle()+90);
    directPath.translate(treeCenter - *this);
    directPath.setLength(radius);

    directPath.setAngle(directPath.angle()+45);
    QPointF treeCorner1 = directPath.p2();

    directPath.setAngle(directPath.angle()+90);
    QPointF treeCorner2 = directPath.p2();

    directPath.setAngle(directPath.angle()+90);
    QPointF treeCorner3 = directPath.p2();

    directPath.setAngle(directPath.angle()+90);
    QPointF treeCorner4 = directPath.p2();

    QLineF distance1;
    QLineF distance2;

    distance1.setPoints(*this,treeCorner1);
    distance2.setPoints(*this,treeCorner4);

    vector<vector<QLineF>> wholeRays;
    vector<QLineF> wholeRay1;
    vector<QLineF> wholeRay2;
    vector<QLineF> wholeRay3;

    if(distance1.length()<distance2.length()){

        wholeRay1.push_back(distance1);
        wholeRay1.push_back(QLineF(treeCorner1,treeCorner4));
        wholeRay1.push_back(QLineF(treeCorner4,*Rx));

        wholeRays.push_back(wholeRay1);

        wholeRay2.push_back(QLineF(*this,treeCorner2));
        wholeRay2.push_back(QLineF(treeCorner2,treeCorner3));
        wholeRay2.push_back(QLineF(treeCorner3,*Rx));

        wholeRays.push_back(wholeRay2);

        // Top ray

        QPointF edge1;
        directPath.setPoints(*this,*Rx);
        directPath.intersect(distance1,&edge1);

        QPointF edge2;
        directPath.intersect(distance2,&edge2);

        QLineF firstDiffractedRay(*this,edge1);
        QLineF lastDiffractedRay(*this,edge2);

        // Correct the lenght of the first and last diffractrd rays
        wholeRay3.push_back(firstDiffractedRay);
        wholeRay3.push_back(QLineF(edge1,edge2));
        wholeRay3.push_back(lastDiffractedRay);

    }
    else{

        wholeRay1.push_back(QLineF(distance2));
        wholeRay1.push_back(QLineF(treeCorner4,treeCorner1));
        wholeRay1.push_back(QLineF(treeCorner1,*Rx));

        wholeRays.push_back(wholeRay1);

        wholeRay2.push_back(QLineF(*this,treeCorner3));
        wholeRay2.push_back(QLineF(treeCorner3,treeCorner2));
        wholeRay2.push_back(QLineF(treeCorner2,*Rx));

        wholeRays.push_back(wholeRay2);

        // Top ray

        QPointF edge1;
        directPath.setPoints(*this,*Rx);
        directPath.intersect(distance2,&edge1);

        QPointF edge2;
        directPath.intersect(distance1,&edge2);

        QLineF firstDiffractedRay(*this,edge2);
        QLineF lastDiffractedRay(*this,edge1);

        // Correct the lenght of the first and last diffractrd rays
        wholeRay3.push_back(firstDiffractedRay);
        wholeRay3.push_back(QLineF(edge2,edge1));
        wholeRay3.push_back(lastDiffractedRay);
    }
    return wholeRays;
}

void MathematicalTransmitterProduct::computeRayThroughTree(QPointF *Rx, MathematicalTreeProduct *tree){
    vector<vector<QLineF>> wholeRays = buildTreeRays(Rx,tree);
    for(int i;i<m_wholeRays.size();i++){
        complex<double> EfieldTree = computeDiffractedTreeEfield(wholeRays.at(i));

    }

}

double MathematicalTransmitterProduct::computePrx(complex <double> totalEfield){
    // Compute the power at the receive antenna with the total electric field induced by all MPC

    complex <double> Voc = (lambda/M_PI)*(totalEfield);
    double Prx = 1/(8*Ra)*norm(Voc);
    return Prx;
}

double MathematicalTransmitterProduct::dBm(double power){return 10*(log10(power)) + 30.0;}


double MathematicalTransmitterProduct::computeReflexionPer(double thetaI, double epsilonR){
    //double R = (cos(thetaI) - sqrt(epsilonR)*sqrt(1 - (1/epsilonR)*pow(sin(thetaI),2)))/(cos(thetaI) + sqrt(epsilonR)*sqrt(1 - (1/epsilonR)*pow(sin(thetaI),2)));
    /*
     * Now in the code thatai is the angle between the ray and the wall and not between the ray and the normal to the wall.
     * Basicly thetai = pi/2 - thetai.
     * Because of that cos and sin are inverted and we take their absolute value because of the angles given by Qt.
     */

    double R = (abs(sin(thetaI)) - sqrt(epsilonR)*sqrt(1 - (1/epsilonR)*pow(cos(thetaI),2)))/(abs(sin(thetaI)) + sqrt(epsilonR)*sqrt(1 - (1/epsilonR)*pow(cos(thetaI),2)));
    return R;
}

double MathematicalTransmitterProduct::computeReflexionPar(double thetaI, double epsilonR){
    double R = (cos(thetaI) - (1/sqrt(epsilonR))*sqrt(1 - (1/epsilonR)*pow(sin(thetaI),2)))/(cos(thetaI) + (1/sqrt(epsilonR))*sqrt(1 - (1/epsilonR)*pow(sin(thetaI),2)));
    return R;
}

vector<vector<MathematicalRayProduct*> *> MathematicalTransmitterProduct::getRays(){
    return m_wholeRays;
}

void MathematicalTransmitterProduct::notifyObservables(){
    for(int i=0; i<m_productObservable.size();i++){
        m_productObservable.at(i)->notify();
    }
}

QPointF MathematicalTransmitterProduct::sceneRectIntersection(const QRectF &rect, const QLineF  &line)const{
    /*
     * This function takes a bounding ray (line) of the illumination zone and gives its intersection
     * with the scene boundaries.
     */

    QLineF boundary1(rect.topLeft(),rect.bottomLeft()),
            boundary2(rect.bottomLeft(),rect.bottomRight()),
            boundary3(rect.bottomRight(),rect.topRight()),
            boundary4(rect.topRight(),rect.topLeft());

    QPointF intersectionPoint;

    if(line.intersect(boundary1,&intersectionPoint) == 1){}
    else if(line.intersect(boundary2,&intersectionPoint) == 1){}
    else if(line.intersect(boundary3,&intersectionPoint) == 1){}
    else if(line.intersect(boundary4,&intersectionPoint) == 1){}
    return intersectionPoint;
}

vector <QPointF> MathematicalTransmitterProduct::boundaryCorners(const QRectF &rect, const QPolygonF &unboundedZone)const{
    /*
     * Gives the corners of the scene bounding rectangle that lie in the ubounded illumination zone.
     * It is used to complete the bounded illumination zone polygone.
     */

    vector <QPointF> points;
    if(unboundedZone.containsPoint(rect.topLeft(),Qt::OddEvenFill)){
        points.push_back(rect.topLeft());
    }
    if(unboundedZone.containsPoint(rect.bottomLeft(),Qt::OddEvenFill)){
        points.push_back(rect.bottomLeft());
    }
    if(unboundedZone.containsPoint(rect.bottomRight(),Qt::OddEvenFill)){
        points.push_back(rect.bottomRight());
    }
    if(unboundedZone.containsPoint(rect.topRight(),Qt::OddEvenFill)){
        points.push_back(rect.topRight());
    }
    return points;
}

void MathematicalTransmitterProduct::setSceneBoundary(const QRectF &rect){
    m_sceneBoundary = rect;
}

void MathematicalTransmitterProduct::computePathLoss(QLineF direct_ray, ProductObservable* true_receiver){
    std::vector<QPointF> points;

    int number_points = 2;

    for (int i = 1; i<=number_points; i++){
        points.push_back(direct_ray.pointAt(double(i)/number_points));
    }
    m_algorithm->pathLossComputation(points,true_receiver,this);
}

// Tree transmition


double MathematicalTransmitterProduct::computeElevationScaterringAngle(float heightRx, float heightTx, float heightConopy, float distanceRxTree){
    return M_PI/2 - atan2(heightRx - (heightTx+heightConopy/2),distanceRxTree);
}

map<string,double> MathematicalTransmitterProduct::computeIncidenceDepartureAngles(float angleIncidenceConopy, float angleAzimuth, float angleElevation){
    map<string,double> anglesScttering;
    anglesScttering["incidenceElevation"] = acos(cos(angleElevation)*cos(angleIncidenceConopy) - sin(angleElevation)*cos(angleAzimuth));
}

// ---------------------------------------------------- TransmitterProduct -------------------------------------------------------------------

void MathematicalTransmitterProduct::newProperties(){
    m_graphic->notifyToGraphic(this, m_orientation);
}

// ---------------------------------------------------- MathematicalProduct -------------------------------------------------------------------

void MathematicalTransmitterProduct::update(QGraphicsItem *graphic){
    QPointF direction(graphic->scenePos().x() - x(),graphic->scenePos().y() - y());
    m_zone.translate(direction);
    setX(graphic->scenePos().x());
    setY(graphic->scenePos().y());
    notifyObservables();

//    for(int i =0;i<m_zone.size();i++){
//        cout<<m_zone.at(i).x()<<", "<<m_zone.at(i).y()<<endl;
//    }

}

void MathematicalTransmitterProduct::openDialog(){
    new DialogTransmitterProduct(this);
}

// ---------------------------------------------------- ProductObserver -------------------------------------------------------------------


void MathematicalTransmitterProduct::update(ProductObservable* receiver, const float speed, const float direction){
    //m_EMfield = 0;
    //m_power = 0;
    //cout<<"Transmitter position: "<<x()<<", "<< y() <<endl;
    //cout<<"Receiver position: "<<pos.x()<<", "<< pos.y() <<endl;
    //cout<<"Transmitter notified !"<<endl;

    m_receiversField[receiver] = 0;
    m_receiversPowers[receiver].erase(m_receiversPowers[receiver].begin(),m_receiversPowers[receiver].end());
    m_attenuation.erase(receiver);
    m_raylength.erase(receiver);

    QPointF* pos = receiver->getPos();
    //vector<vector<MathematicalRayProduct*>*> *wholeRays;
    if(m_receiversRays.count(receiver)){
        for(int i =0; i<m_receiversRays[receiver].size();i++){
            for(int j=0;j<m_receiversRays[receiver].at(i)->size();j++){
                delete m_receiversRays[receiver].at(i)->at(j);
            }
        }
        m_receiversRays[receiver].erase(m_receiversRays[receiver].begin(),m_receiversRays[receiver].end());
        m_receiversRays[receiver].shrink_to_fit();
    }

//    for(int i =0; i<m_wholeRays.size();i++){
//        for(int j=0;j<m_wholeRays.at(i)->size();j++){
//            delete m_wholeRays.at(i)->at(j);
//        }
//    }
//    m_wholeRays.erase(m_wholeRays.begin(),m_wholeRays.end());
//    cout<<"Are you ok Annie?"<<endl;
    //m_wholeRays.shrink_to_fit();

//    cout<< "Mathtrans rec pos: "<<pos->x()<<", "<<pos->y()<<" m_zone size: "<<m_zone.size()<<endl;

    if(m_zone.containsPoint(*pos,Qt::OddEvenFill)){

//        cout<<"In da zone"<<endl;
        vector<MathematicalRayProduct*> *wholeRay = new vector<MathematicalRayProduct*>;
        QPointF m_pos(int(this->x()),int(this->y()));
        //MathematicalRayProduct newRay = *(m_rayFactory->createRay(*this,pos));
        wholeRay->push_back(m_rayFactory->createRay(*this,*pos));
//        m_wholeRays.push_back(wholeRay);

        m_receiversRays[receiver].push_back(wholeRay);
        QPointF p1 = wholeRay->at(0)->p1();
        QPointF p2 = wholeRay->at(0)->p2();
        QLineF direct_ray = QLineF(p1,p2);
        if (active_pathloss){
            computePathLoss(direct_ray, receiver);
        }
        if(wholeRay->at(0)->getDiffracted()){
            complex<double>EMfield = computeDiffractedEfield(wholeRay);
            m_receiversField[receiver] += EMfield;
            double power = computePrx(EMfield);
            m_receiversPowers[receiver].push_back(power);
        }
        else{
            complex<double>EMfield = computeEMfield(wholeRay,receiver);
            m_receiversField[receiver] += EMfield;
            double power = computePrx(EMfield);
            m_receiversPowers[receiver].push_back(power);
        }
        double totalPower = computePrx(m_receiversField[receiver]);

        //m_model->notify(this);

//        cout<< "MathTrans power: "<< totalPower<< endl;

//        cout<<"EM field: "<<m_receiversField[receiver]<<endl;
        double powerDBm = dBm(totalPower);
//        cout<< "MathTrans power dBm : "<< powerDBm<< endl;
        if (!compute_pathloss){
            m_algorithm->sendData(this,dynamic_cast<MathematicalProduct*>(receiver));
            receiver->answer(this,powerDBm,&m_receiversPowers[receiver],m_receiversField[receiver]);
        }
    }


    //}

        //cout<<"Ray: "<<newRay.x1()<<", "<<newRay.y1()<<", "<<" and "<<newRay.x2()<<", "<<newRay.y2()<<endl;
}


void MathematicalTransmitterProduct::attachObservable(ProductObservable* productObservable){
    m_productObservable.push_back(productObservable);
}

// ---------------------------------------------------- ModelObserver -------------------------------------------------------------------

void MathematicalTransmitterProduct::attachObservable(ModelObservable *modelObservable){
    m_model = modelObservable;
}


// ---------------------------------------------------- AbstractAntenna -------------------------------------------------------------------


void MathematicalTransmitterProduct::notifyParent(ProductObservable *receiver, const float speed, const float direction, const QPointF &point, vector<MathematicalRayProduct*> *wholeRay) {
    MathematicalRayProduct *newRay = m_rayFactory->createRay(*this,point);
    wholeRay->push_back(newRay);
//    m_wholeRays.push_back(wholeRay);
    m_receiversRays[receiver].push_back(wholeRay);

    if(wholeRay->at(0)->getDiffracted()){
        complex<double>EMfield = computeDiffractedEfield(wholeRay);
        m_receiversField[receiver] += EMfield;
        double power = computePrx(EMfield);
        m_receiversPowers[receiver].push_back(power);
    }
    else{
        complex<double>EMfield = computeEMfield(wholeRay, receiver);
        m_receiversField[receiver] += EMfield;
        double power = computePrx(EMfield);
        m_receiversPowers[receiver].push_back(power);
    }
    double totalPower = computePrx(m_receiversField[receiver]);

    //m_model->notify(this);

    double powerDBm = dBm(totalPower);
    if (compute_pathloss){
        QPointF* point_receiver = dynamic_cast<QPointF*>(receiver);
        QLineF line = QLineF(*this,*point_receiver);
        m_pathloss[receiver][line.length()*px_to_meter] = powerDBm;
    }
    else {
        m_algorithm->sendData(this,dynamic_cast<MathematicalProduct*>(receiver));
        receiver->answer(this,powerDBm,&m_receiversPowers[receiver],m_receiversField[receiver]);
    }
}

QPointF MathematicalTransmitterProduct::getPosition()const{
    return *this;
}

QPolygonF MathematicalTransmitterProduct::getIlluminationZone(const QRectF &rect)const{
    return QPolygonF(rect);
}

QPolygonF MathematicalTransmitterProduct::getIlluminationZone()const{

    /*
     * This method is used right now in the code BUT we've got to make sure that the user don't add
     * any element to the scene once the antena is set.
     */

//    return QPolygonF(m_sceneBoundary);
    return m_zone;
}

QPolygonF MathematicalTransmitterProduct::getIlluminatedZone()const{
    return m_zone;
}


void MathematicalTransmitterProduct::setIlluminatedZone(const QPolygonF &zone){
    m_zone = zone;
}
