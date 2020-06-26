#include "mathematicaltransmitterproduct.h"
MathematicalTransmitterProduct::MathematicalTransmitterProduct(int posX, int posY):QPointF(posX,posY)
{
    //setX(posX);
    //setY(posY);
    m_type = "Transmitter";
    m_radius = 500;
    m_zone = buildCoverage();

}

MathematicalTransmitterProduct::~MathematicalTransmitterProduct(){

}



double MathematicalTransmitterProduct::computeGain(double theta,double phi, double frequency, int row,int column,double antennaDistance){
    complex <double> dipolefactor(0.0,0.0);
    complex <double> xarray(0.0,0.0);
    complex <double> yarray(0.0,0.0);
    complex <double> arrayfactor(0.0,0.0);
    complex <double> i(0.0,1.0);
    complex <double> psy(0.0,0.0);
    complex <double> qsy(0.0,0.0);

    double c = 3.0e9;
    double lambda = c/frequency;
    double d = lambda*antennaDistance;
    double k = 2.0*M_PI/lambda;
    double gain;

    psy = i*k*d*sin(phi*M_PI/180)*sin(theta*M_PI/180);
    qsy = i*k*d*sin(phi*M_PI/180)*cos(theta*M_PI/180);
    xarray = (exp(qsy*double(row))-1.0)/(exp(qsy)-1.0);
    yarray = (exp(psy*double(column))-1.0)/(exp(psy)-1.0);

    if (psy == 0.0+0.0*i){
        yarray = double(column) +0.0*i;
    }
    if(qsy == 0.0+0.0*i){
        xarray = double(row)+0.0*i;
    }

    arrayfactor = xarray*yarray;
    if (phi == 0.0 || phi == 180.0) dipolefactor = 0.0;
    else dipolefactor = cos(0.5*M_PI*cos(phi*M_PI/180))/sin(phi*M_PI/180);
    gain = 16.0/(3.0*M_PI)*abs(arrayfactor)*abs(dipolefactor);
    return gain;
}

void MathematicalTransmitterProduct::drawRays(){

}

float MathematicalTransmitterProduct::getOrientation(){
    return m_orientation;
}


unsigned long MathematicalTransmitterProduct::getFrequency(){
    return m_frequency;
}

double MathematicalTransmitterProduct::getPower(){
    return m_power;
}

int MathematicalTransmitterProduct::getRow(){
    return m_row;
}

int MathematicalTransmitterProduct::getColumn() {
    return m_column;
}

void MathematicalTransmitterProduct::setOrientation(float orientation){
    m_orientation = orientation;
}

void MathematicalTransmitterProduct::setPower(double power){
    m_power = power;
}

void MathematicalTransmitterProduct::setFrequency(unsigned long frequency){
    m_frequency = frequency;
}

void MathematicalTransmitterProduct::setRow(int row){
    m_row = row;
}

void MathematicalTransmitterProduct::setColumn(int column) {
    m_column = column;
}

QPolygonF MathematicalTransmitterProduct::buildCoverage(){
    QPolygonF coverage;
    for(int i=0;i<16;i++){
        coverage<<QPointF(x()+m_radius*cos(M_PI*i/8),y()+m_radius*sin(M_PI*i/8));
    }
    return coverage;
}

void MathematicalTransmitterProduct::update(QGraphicsItem* graphic){
    QPointF direction(graphic->scenePos().x() - x(),graphic->scenePos().y() - y());
    m_zone.translate(direction);
    setX(graphic->scenePos().x());
    setY(graphic->scenePos().y());
    notifyObservables();

//    for(int i =0;i<m_zone.size();i++){
//        cout<<m_zone.at(i).x()<<", "<<m_zone.at(i).y()<<endl;
//    }

}

void MathematicalTransmitterProduct::notifyObservables(){
    for(int i=0; i<m_productObservable.size();i++){
        m_productObservable.at(i)->notify();
    }
}

void MathematicalTransmitterProduct::notify(const QPointF &pos){
    //m_EMfield = 0;
    //m_power = 0;
    //cout<<"Transmitter position: "<<x()<<", "<< y() <<endl;
    //cout<<"Receiver position: "<<pos.x()<<", "<< pos.y() <<endl;
    cout<<"Transmitter notified !"<<endl;

//    for(int i =0; i<m_wholeRays.size();i++){
//        for(int j=0;j<m_wholeRays.at(i)->size();j++){
//            delete &m_wholeRays.at(i)->at(j);
//        }
//    }
//    m_wholeRays.erase(m_wholeRays.begin(),m_wholeRays.end());
    //m_wholeRays.shrink_to_fit();

    if(m_zone.containsPoint(pos,Qt::OddEvenFill)){
        vector<MathematicalRayProduct> *wholeRay = new vector<MathematicalRayProduct>;
        QPointF m_pos(int(this->x()),int(this->y()));
        //MathematicalRayProduct newRay = *(m_rayFactory->createRay(*this,pos));
        wholeRay->push_back(*m_rayFactory->createRay(*this,pos));
        m_wholeRays.push_back(wholeRay);
        m_EMfield += computeEMfield(wholeRay);
        //m_power = m_scene->computePrx(m_EMfield,this);
        //m_receiver->addWholeRay(wholeRay);

        cout<<"My number of whoeRays: "<<m_wholeRays.size()<<endl;

        //m_model->notify(this);
    }
    //}

        //cout<<"Ray: "<<newRay.x1()<<", "<<newRay.y1()<<", "<<" and "<<newRay.x2()<<", "<<newRay.y2()<<endl;
}

void MathematicalTransmitterProduct::notifyParent(const QPointF &point, vector<MathematicalRayProduct> *wholeRay) {
    MathematicalRayProduct newRay = *m_rayFactory->createRay(*this,point);
    wholeRay->push_back(newRay);
    m_wholeRays.push_back(wholeRay);
    //m_receiver->addWholeRay(wholeRay);
    m_EMfield += computeEMfield(wholeRay);
    //m_power = computePrx(m_EMfield,this);
    m_model->notify(this);

}


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

complex <double> MathematicalTransmitterProduct::computeEMfield(vector<MathematicalRayProduct> *rayLine){
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
//    MathematicalRayProduct* currentRay;
    for (int i=0; i<amountSegment; i++){
//        currentRay = &rayLine->at(i);
//        theta = currentRay->getTheta();

        theta = rayLine->at(i).getTheta();

        if((i != amountSegment-1)){   // The last segment, the one that reach the receptor does not have a rebound
            //double thetaI = abs(currentRay->getTetai());
            double thetaI = abs(rayLine->at(i).getTetai());

            R *= computeReflexionPer(thetaI,epsilonWallRel);
        }
        completeLength += rayLine->at(i).getMeterLength(); // Get each length of each ray segment after the meter conversion (1px == 1dm)
        //completeLength += currentRay->getMeterLength(); // Get each length of each ray segment after the meter conversion (1px == 1dm)

    }
    double Ia = sqrt(2.0*m_power/Ra); // Ia could be changed for Beamforming application (add exp)
    double a = R * ((Zvoid*Ia)/(2.0*M_PI))/completeLength;
    Efield = i * a * exp(-i*(2.0*M_PI/lambda)*completeLength);

    /*
     * This part needs to be reviewed for the new architecture
     *
    if(amountSegment==1){
        this->minLength = completeLength; // for delay spread computation
        this->LOS = pow(a,2);
    } else{
        this->NLOS += pow(a,2);
    }
    if(completeLength > this->maxLength){
        this->maxLength = completeLength; // for delay spread computation
    }
    // Store ray parameter for Physical impulse response
    if(computePhysicalResponse){
        // Store attenuation a and distance completeLength
        channelData[rayNumber] = R/completeLength;
        channelData[rayNumber+20] = completeLength;
        double degangle = round((theta+direction)*180/M_PI);
        double radangle = degangle*M_PI/180;
        spectrumField[specNumber] = Efield;
        spectrumAngle[specNumber] = (2.0*M_PI/lambda)*speedReal*cos(radangle);
        int save = specNumber;
        for (int j = 0; j < save; j++){
            if (spectrumAngle[j] == spectrumAngle[save]){
                spectrumField[j] += Efield;
                specNumber -= 1;
            }
        }
        specNumber += 1;
        rayNumber += 1;
    }*/
    return Efield;
}

vector<vector<MathematicalRayProduct> *> MathematicalTransmitterProduct::getRays(){
    return m_wholeRays;
    }

//void MathematicalTransmitterProduct::setRayFactory(AbstractRayFactory *rayFactory){
//    m_rayFactory = rayFactory;
//}

void MathematicalTransmitterProduct::attachObservable(ModelObservable *modelObservable){
    m_model = modelObservable;
}

void MathematicalTransmitterProduct::attachObservable(ProductObservable* productObservable){
    m_productObservable.push_back(productObservable);
}

void MathematicalTransmitterProduct::openDialog(){

}


//--------------------------

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


void MathematicalTransmitterProduct::setSceneBoundary(const QRectF &rect){
    m_sceneBoundary = rect;
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




void MathematicalTransmitterProduct::setIlluminatedZone(const QPolygonF &zone){
    m_zone = zone;
}
