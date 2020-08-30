#include "mathematicaltransmitterproduct.h"
#include <chrono>

MathematicalTransmitterProduct::MathematicalTransmitterProduct(int posX, int posY) : QPointF(posX, posY)
{
    m_type              = "Transmitter";
    m_zone              = buildCoverage();
    m_frequency         = 26e9;
    m_bandwidth         = 100e6;
    m_row               = 1;
    m_column            = 1;
    m_orientation       = 0;
    m_pr_orientation    = 0;
    lambda              = c / m_frequency;
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!                                      !!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!! Relative Permittivity is on the wall !!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!                                      !!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    epsilonWallRel      = 5;
}

MathematicalTransmitterProduct::~MathematicalTransmitterProduct()
{
    cout << "Mathematical Transmitter Product Deleted." << endl;
}


void MathematicalTransmitterProduct::appendTree(MathematicalTreeProduct *tree)
{
    m_trees.push_back(tree);
}


void MathematicalTransmitterProduct::drawRays()
{

}


QPolygonF MathematicalTransmitterProduct::buildCoverage()
{
    QPolygonF coverage;
    for (int i = 0; i < 16; i++) {
        coverage << QPointF(x() + m_radius * cos(M_PI * i / 8), y() + m_radius * sin(M_PI * i / 8));
    }
    return coverage;
}

complex<double>
MathematicalTransmitterProduct::computeImpulseReflection(WholeRay *ray_line, QLineF local_region)
{
    complex <double> i(0.0, 1.0);
    int amountSegment = ray_line->size();
    double completeLength = 0.0;
    double R = 1;
    MathematicalRayProduct *current_ray;

    for (int i = 0; i < amountSegment; i++) {
        current_ray = ray_line->at(i);
        if (i != amountSegment - 1) {
            double thetaI = abs(current_ray->getTetai());
            R *= computeReflexionPar(thetaI, epsilonWallRel);
        }
        completeLength += current_ray->getMeterLength();
    }
    double a = R / completeLength;

    // Angle in degrees
    double angle_transmitter = ray_line->back()->angle();
    double angle_receiver = ray_line->front()->angle();
    QLineF beta(QPointF(.0,.0),QPointF(2.0 * M_PI / lambda,.0));
    beta.setAngle(angle_receiver);
    double scalar_beta_r = beta.p2().x()*local_region.p2().x() + beta.p2().y()*local_region.p2().y();
    complex<double> array_factor = totaleArrayFactor(angle_transmitter, 90);
    double tau = completeLength/c;

    complex<double> impulse_r = a * array_factor
                                * exp(-i * scalar_beta_r)
                                * exp(-i * 2.0*M_PI * std::complex<double>(m_frequency * tau));
    return impulse_r;
}

complex <double> MathematicalTransmitterProduct::computeEMfield(const not_null<WholeRay*> rayLine,
                                                               const ProductObservable */*receiver*/,
                                                               const bool /*properties*/)
{
    //  One vector<ray*> is one multi-path componant, the size of the vector determine the n-level
    //  we are in, for each ray only the power in the last ray is transmitted to
    //  the receptor. As seen in the power formula, n rays -> n-1 additions to the power.

    //  This function gives the electrical field, generated by one MPC, at the receiver.
    //  The electric field is // to the dipole antenna since we only consider relections
    //  off the buildings. The electric field is not // for the reflexion with the ground
    //  though. This is taken into account in the function computePrx.

    complex <double> i(0.0, 1.0);
    double completeLength = rayLine->totalLength();
    double R = computeR(rayLine);
    complex <double> Efield = 0.0;

    // Angle in degrees
    double angle_transmitter = rayLine->angleTx();

    double Ia = sqrt(2.0 * m_power / (m_row * m_column * Ra)); // Ia could be changed for Beamforming application (add exp)
    complex<double> array_factor = totaleArrayFactor(angle_transmitter, 90);
    complex<double> a = R * array_factor * exp(-i * (2.0 * M_PI / lambda) * completeLength) / completeLength;

    Efield = - i * ((Zvoid * Ia) * a / (2.0 * M_PI));

    return Efield;
}


double MathematicalTransmitterProduct::computeR(WholeRay *wholeRay) const
{
    double R = 1;
    for (WholeRay::iterator ray = wholeRay->begin(); ray != wholeRay->end() - 1; ray++ ) {
        double thetaI = abs((*ray)->getTetai());
        R *= computeReflexionPer(thetaI, epsilonWallRel);
    }
    return R;
}

void MathematicalTransmitterProduct::estimateCh(ProductObservable *rx)
{
    complex <double> i(0.0, 1.0);
    m_los_factor[rx] = 0;
    double pwr = 0;
    double hMax = 0;

    for (const auto wholeRay: m_receiversRays[rx]) {

        const double completeLength = wholeRay->totalLength();
        const double angleRx = wholeRay->angleRx();
        const double angleTx = wholeRay->angleTx();
        Data &chData = m_chsData.at(rx);
        complex<double> array_factor = totaleArrayFactor(angleTx, 90);
        const double R = computeR(wholeRay);
        complex<double> a =  R * array_factor / completeLength;
        complex<double> h = a * exp(-i * (2.0 * M_PI / lambda) * completeLength);
        pwr += norm(h);
        hMax = abs(h) > hMax ? abs(h): hMax;

        m_los_factor[rx] += norm(h);

        // Tau
        double tau = completeLength * 1e9/c; // [ns]
        tau = round(tau*1e4)/1e4; // [precision of 0.1 ps]

        // Impulse response
        m_receiversImpulse[rx][tau] += h; // To change
        chData.impulseResp[tau] += h;

        double u = uMPC(angleRx);
        complex<double> angularDistr = angDistrMPC(h, u);
        double prxAngSpctr = pasMPC(angularDistr);

        // Save Data
        chData.u.push_back(u);
        chData.angularDistr.push_back(angularDistr);
        chData.prxAngularSpctr.push_back(prxAngSpctr);

        m_receiver_speed.translate(- m_receiver_speed.p1());
        m_ray_speed.translate(- m_ray_speed.p1());
        QLineF resultant_speed(QPointF(0.0, 0.0), m_receiver_speed.p2() - m_ray_speed.p2());
        QLineF beta(QPointF(.0, .0), QPointF(2.0 * M_PI / lambda, 0.0));
        beta.setAngle(angleRx);
        double omega = - (beta.p2().x() * resultant_speed.p2().x() + beta.p2().y() * resultant_speed.p2().y());
        omega = round(omega * 1e4) / 1e4;
        m_dopplerSpectrum[rx][omega] += h;
        m_chsData[rx].dopplerSpctr[omega] += h;

    }

//    normalizePAS(m_chsData.at(rx).prxAngularSpctr, pow(hMax, 2));

}

complex <double>
MathematicalTransmitterProduct::computeImpulseGroundReflection(ProductObservable *copy_receiver,
                                                               double direction,
                                                               QLineF local_region)
{
    double distance = this->distance(copy_receiver);
    double thetaG = atan((distance / 2) / antennaHeight);
    double thetaI = M_PI - thetaG;
    double R = computeReflexionPer(thetaG, epsilonWallRel);
    double completeLength = sqrt(4*pow(antennaHeight,2)+pow(distance,2));
    complex <double> i(0.0, 1.0);
    double a = R  / completeLength;
    complex<double> array_factor = totaleArrayFactor(direction,thetaI*180.0/M_PI);
    double tau = completeLength/c;
    double local_region_angle = local_region.angle() + 180.0;
    local_region_angle = fmod(local_region_angle, 360.0);
    double angle = (direction - local_region_angle)*M_PI/180.0;
    complex<double> impulse_r = a * array_factor
                                * exp(-i * (2.0 * M_PI / lambda) * local_region.length() * cos(angle) * cos(M_PI/2-thetaG))
                                * exp(-i * 2.0*M_PI * std::complex<double>(m_frequency * tau));
    return impulse_r;
}

complex <double> MathematicalTransmitterProduct::computeEfieldGround(const ProductObservable *receiver,
                                                                    const double direction,
                                                                    const bool properties)
{
    // Compute the electrical field, at the receiver, induced by the ray reflected off the ground.
    // To Do: check if there is a wall between the TX and RX
    double distance = this->distance(receiver); // conversion (1px == 2cm)
    double thetaG = atan((distance / 2) / antennaHeight);
    double thetaI = M_PI - thetaG;
    double R = computeReflexionPar(thetaG, epsilonWallRel);
    double completeLength = sqrt(4 * pow(antennaHeight, 2) + pow(distance, 2)); //distance / sin(thetaG);

    complex <double> i(0.0, 1.0);

    double Ia = sqrt(2 * m_power / (m_row*m_column*Ra)); // Ia could be changed for Beamforming application
    complex<double> array_factor = totaleArrayFactor(direction,thetaI*180.0/M_PI);
    complex<double> a = R * array_factor * exp(-i*(2.0*M_PI/lambda)*completeLength) / completeLength;
    complex <double> Efield = -i * a * (Zvoid * Ia) / (2*M_PI);



    if (properties)
    {
        double tau = completeLength * 1e9/c; // [ns]
        tau = round(tau*1e4)/1e4; // [precision of 0.1 ps]
        m_receiversImpulse[receiver][tau] += a;
        m_chsData[receiver].impulseResp[tau] += a;
        m_nlos_factor[receiver] += pow(abs(a),2);
        double shift = (direction - m_receiver_speed.angle()) * M_PI / 180.0;
        double omega = -2.0 * M_PI * m_receiver_speed.length() * cos(shift) * cos(M_PI/2-thetaG) / lambda;
        m_dopplerSpectrum[receiver][omega] += a;
        m_chsData[receiver].dopplerSpctr[omega] += a;

        double angleRx = 360 - direction;
        Data &chData = m_chsData[receiver];

        // Put computed data into channels data.
        double u = uMPC(angleRx);
        complex<double> angularDistr = angDistrMPC(a, u);
        double prxAngSpctr = pasMPC(angularDistr);

        // Save Data
        chData.u.push_back(u);
        chData.angularDistr.push_back(angularDistr);
        chData.prxAngularSpctr.push_back(prxAngSpctr);

    }

    return Efield;
}

double MathematicalTransmitterProduct::uMPC(angle angleRx)
{
    double beta = 2 * M_PI / lambda;
    double theta = angleRx > 90 ? angleRx - 90: angleRx + 270;
    return beta * cos(theta * M_PI / 180);
}

double MathematicalTransmitterProduct::pasMPC(complex<double> &angDistr)
{
    return norm(angDistr / (2 * M_PI));
}

complex<double> MathematicalTransmitterProduct::angDistrMPC(complex<double> &h, double u)
{
    double beta = 2 * M_PI / lambda;
    return 2 * M_PI * h / (beta * sqrt(1 - pow(u / beta, 2)));

}

void MathematicalTransmitterProduct::normalizePAS(vector<double> &pas, double normTerm)
{
    for (auto &p: pas) {
        p = p / normTerm;
    }
}

double MathematicalTransmitterProduct::distance(const ProductObservable *receiver)
{

    // ??????
    //  For display, it calcultes the direct Euclidian distance from the receiver
    //  to the emettor, displays the results on the UI
    //

    double x1 = x();
    double y1 = y();
    double x2 = receiver->getPos()->x();
    double y2 = receiver->getPos()->y();

    return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2)) * px_to_meter; // conversion (1px == 1dm)
}

complex<double>
MathematicalTransmitterProduct::computeImpulseDiffraction(WholeRay *ray_line, QLineF local_region)
{
    double direct_dist = sqrt(pow(ray_line->at(1)->p1().x() - ray_line->at(0)->p2().x(), 2)
                              + pow(ray_line->at(1)->p1().y() - ray_line->at(0)->p2().y(), 2)); //convertir px to cm?
    complex<double> F = 0.0;
    complex <double> i(0.0, 1.0);

    // The length defference between the path going through the tip of the obstacle, and the direct path.
    double delta_r = (ray_line->at(0)->length() + ray_line->at(1)->length() - direct_dist) * px_to_meter;
    double nu = sqrt(2 * 2 * M_PI / lambda * delta_r / M_PI);

    // The ITU's approximation for |F(nu)|^2
    double absF = pow(10, -6.9 / 40) / sqrt((sqrt(pow(nu - 0.1, 2) + 1) + nu - 0.1));
    double argF = -M_PI / 4 - pow(nu, 2) * M_PI / 2;
    F = absF * exp(i * argF);

    Line *directRay = new Line(ray_line->at(0)->p2(), ray_line->at(1)->p1());
    directRay->setScale(px_to_meter);
    double angle = ray_line->at(1)->angle();
    QLineF beta(QPointF(.0,.0),QPointF(2.0 * M_PI / lambda,.0));
    beta.setAngle(ray_line->at(0)->angle());
    double scalar_beta_r = beta.p2().x()*local_region.p2().x() + beta.p2().y()*local_region.p2().y();
    double tau = (ray_line->at(0)->getMeterLength()+ray_line->at(1)->getMeterLength())/c;
    complex<double> array_factor = totaleArrayFactor(angle,90);

    complex<double> impulse_r = array_factor*F
                                *exp(-i * scalar_beta_r)
                                *exp(-i * 2.0*M_PI * std::complex<double>(m_frequency * tau))
                                /directRay->getMeterLength();
    return impulse_r;
}

complex<double> MathematicalTransmitterProduct::computeDiffractedEfield(ProductObservable *receiver,
                                                                        WholeRay *rayLine,
                                                                        bool properties)
{

    // Direct distance between the receiver and the transmitter
//    double direct_dist = sqrt(pow(transmitter->getPosition().x()-m_Receiver->getPosX(),2)
//                              + pow(transmitter->getPosition().y()-m_Receiver->getPosY(),2)); //convertir px to cm?

    double direct_dist = rayLine->directDistance(); //convertir px to cm?

    complex<double> Efield = 0.0;
    complex<double> F = 0.0;
    complex <double> i(0.0, 1.0);

    // The length defference between the path going through the tip of the obstacle, and the direct path.

    double delta_r = (rayLine->at(0)->length() + rayLine->at(1)->length() - direct_dist) * px_to_meter;

    double nu = sqrt(2 * 2 * M_PI / lambda * delta_r / M_PI);
    // The ITU's approximation for |F(nu)|^2

    double absF = pow(10, -6.9 / 40) / sqrt((sqrt(pow(nu - 0.1, 2) + 1) + nu - 0.1));
    double argF = -M_PI / 4 - pow(nu, 2) * M_PI / 2;
    F = absF * exp(i * argF);

    Line *directRay = new Line(rayLine->at(0)->p2(), rayLine->at(1)->p1());
    double angle = rayLine->at(1)->angle();
    complex<double> array_factor = totaleArrayFactor(angle,90);
    directRay->setScale(px_to_meter);
    complex<double> a = F * array_factor * (exp(-i * (2.0 * M_PI / lambda) * directRay->getMeterLength())
                        / directRay->getMeterLength());
    double Ia = sqrt(2 * m_power / (m_row*m_column*Ra)); // Ia could be changed for Beamforming application (add exp)
    Efield = -i  * a * ((Zvoid * Ia) / (2 * M_PI));
    delete directRay;
    if (properties){
        double completeLength = rayLine->at(1)->getMeterLength() + rayLine->at(0)->getMeterLength();
        double tau = completeLength * 1e9/c; // [ns]
        tau = round(tau*1e4)/1e4; // [precision of 0.1 ps]
        m_receiversImpulse[receiver][tau] = a;
        m_chsData[receiver].impulseResp[tau] = a;

    }
    return Efield;
}


void MathematicalTransmitterProduct::chooseBeam(ProductObservable *receiver)
{

    double powerAtReceiver = 0;
    complex<double> emField = 0;
    complex<double> groundField = 0;
    char chosenBeam = -5;

    vector<WholeRay *> wholeRays = m_receiversRays[receiver];

    for (int i = -5; i < 5; i++) {
        m_pr_orientation = i;

        emField = 0;

//        for (vector<WholeRay *>::iterator j = wholeRays.begin(); j != wholeRays.end(); j++) {

//        }

        for (unsigned j = 0; j < wholeRays.size(); j++) {
            WholeRay *wholeRay  = wholeRays.at(j);

            if (wholeRay->at(0)->getDiffracted()) {

                //
                //      The ray is a diffracted one.
                //

                emField += computeDiffractedEfield(receiver,wholeRay,false);

            } else {

                //
                //      The ray was reflected.
                //
                complex<double> EMfield = computeEMfield(wholeRay, receiver,false);
                if (wholeRay->size() == 1) {
                    // Adding the ground component
                    double angle_transmitter = wholeRay->back()->angle();
                    groundField = computeEfieldGround(receiver,angle_transmitter,false); // Compute the electrical field from the ray reflected off the ground
                }
                emField += EMfield;

            }
        }

        if (computePrx(emField,groundField,receiver) > powerAtReceiver) {

            powerAtReceiver = computePrx(emField,groundField,receiver);
            chosenBeam = i;
            m_receiversField[receiver] = emField;
            m_receiversGroundField[receiver] = groundField;

        }
    }

    m_pr_orientation = chosenBeam;
    m_chosenBeams[receiver] = true;
}


void
MathematicalTransmitterProduct::comput4FixedBeam(ProductObservable *receiver)
{
    bool diffracted = false;
    m_receiversField[receiver] = 0;
    m_receiversGroundField[receiver] = 0;
    for (unsigned i = 0; i < m_receiversRays[receiver].size(); i++)
    {
        WholeRay *wholeRay = m_receiversRays[receiver].at(i);

        if (wholeRay->at(0)->getDiffracted())
        {
            map<ProductObservable *, map<double, double>>::iterator it;
            m_ray_speed = ray_speeds[wholeRay];
            complex<double>EMfield = computeDiffractedEfield(receiver,wholeRay,true);
            m_receiversField[receiver] += EMfield;

            diffracted = true;

        } else {
            m_ray_speed = ray_speeds[wholeRay];
            m_ray_speed.setLength(m_ray_speed.length()/3.6);
            complex<double> EMfield = computeEMfield(wholeRay, receiver,true);
            if (wholeRay->size() == 1) {
                // Adding the ground component
                double angle_transmitter = wholeRay->back()->angle();
                m_receiversGroundField[receiver] = computeEfieldGround(receiver,angle_transmitter,true); // Compute the electrical field from the ray reflected off the ground
            }
            m_receiversField[receiver] += EMfield;

        }
    }

    estimateCh(receiver);
}

void MathematicalTransmitterProduct::dontChoseBeam(ProductObservable *receiver)
{
    m_chosenBeams[receiver] = true;
}


void MathematicalTransmitterProduct::freazeBeams()
{
    m_beamsFrozen = true;
}


double MathematicalTransmitterProduct::computePrx(complex <double> totalEfield, complex<double> groundField, ProductObservable* receiver)
{
    // Compute the power at the receive antenna with the total electric field induced by all MPC
    double distance = this->distance(receiver);
    double thetaI = atan(antennaHeight/(distance/2))+M_PI/2;
    complex <double> Voc = (lambda/M_PI)*(totalEfield + groundField*(cos(M_PI/2*cos(thetaI))/sin(thetaI)));
    double Prx = 1.0 / (8.0 * Ra) * norm(Voc);
    return Prx;
}


double MathematicalTransmitterProduct::dBm(double power)
{
    return 10 * (log10(power)) + 30.0;
}


double MathematicalTransmitterProduct::computeReflexionPer(double thetaI, double epsilonR) const
{
//    double R = (cos(thetaI) - sqrt(epsilonR)*sqrt(1 - (1/epsilonR)*pow(sin(thetaI),2)))
//               /(cos(thetaI) + sqrt(epsilonR)*sqrt(1 - (1/epsilonR)*pow(sin(thetaI),2)));

    //  Now in the code thatai is the angle between the ray and the wall and not between
    //  the ray and the normal to the wall.
    //  Basicly thetai = pi/2 - thetai.
    //  Because of that cos and sin are inverted and we take their absolute value because of the angles given by Qt.

    double R = (abs(sin(thetaI)) - sqrt(epsilonR) * sqrt(1 - (1 / epsilonR) * pow(cos(thetaI), 2))) /
               (abs(sin(thetaI)) + sqrt(epsilonR) * sqrt(1 - (1 / epsilonR) * pow(cos(thetaI), 2)));
    return R;
}


double MathematicalTransmitterProduct::computeReflexionPar(double thetaI, double epsilonR) const
{
    double R = (cos(thetaI) - (1 / sqrt(epsilonR)) * sqrt(1 - (1 / epsilonR) * pow(sin(thetaI), 2))) /
               (cos(thetaI) + (1 / sqrt(epsilonR)) * sqrt(1 - (1 / epsilonR) * pow(sin(thetaI), 2)));
    return R;
}


vector<WholeRay *> MathematicalTransmitterProduct::getRays()
{
    // ???
    return m_wholeRays;
}


void MathematicalTransmitterProduct::notifyObservables()
{
    for (unsigned int i = 0; i < m_productObservable.size(); i++) {
        m_productObservable.at(i)->notify();
    }
}


void MathematicalTransmitterProduct::clearAll(){
    //map<ProductObservable *,vector<vector<MathematicalRayProduct *>*>>::iterator rays;
    vector<WholeRay *> whole_rays;
    WholeRay *tmp;
    for (const auto  &rays: m_receiversRays){
        whole_rays = rays.second;
        for (unsigned i = 0; i<whole_rays.size(); i++){
            tmp = whole_rays.at(i);
            for (unsigned j = 0; j< tmp->size();j ++) {
                delete tmp->at(j);
            }
            delete whole_rays.at(i);
        }
    }
    m_receiversRays.clear();
}

// ---------------------------------------------------- TransmitterProduct -------------------------------------------------------------------


void MathematicalTransmitterProduct::newProperties(QPointF new_position, double orientation)
{
    m_graphic->notifyToGraphic(&new_position, orientation);
}


// ---------------------------------------------------- MathematicalProduct -------------------------------------------------------------------
void MathematicalTransmitterProduct::setPosX(int posX) {
    setX(posX);
}
void MathematicalTransmitterProduct::setPosY(int posY) {
    setY(posY);
}

void MathematicalTransmitterProduct::update(QGraphicsItem *graphic)
{
    QPointF direction(graphic->scenePos().x() - x(), graphic->scenePos().y() - y());
    m_zone.translate(direction);
    setX(graphic->scenePos().x());
    setY(graphic->scenePos().y());
    setOrientation(graphic->rotation());
    notifyObservables();

}


void MathematicalTransmitterProduct::openDialog()
{
    new DialogTransmitterProduct(this);
}

void MathematicalTransmitterProduct::clearChData(ProductObservable *rx)
{
    m_chsData[rx].dopplerSpctr.clear();
    m_chsData[rx].impulseResp.clear();
    m_chsData[rx].pathLossP = 0;
    m_chsData[rx].interference = 0;
    m_chsData[rx].angularSpred = 0;
    m_chsData[rx].u.clear();
    m_chsData[rx].angularDistr.clear();
    m_chsData[rx].prxAngularSpctr.clear();
    m_chsData[rx].riceFactor = 0;

}

// ---------------------------------------------------- ProductObserver -------------------------------------------------------------------


void MathematicalTransmitterProduct::update(ProductObservable *receiver,
                                            const QLineF movement)
{
    //      The trasnmitter is updated every time an receiver moves

    // Clear data corresponding to the receiver calling the update
    clearChData(receiver);

    m_receiversField[receiver] = 0;
    m_los_factor[receiver] = 0.0;
    m_nlos_factor[receiver] = 0.0;
    ray_speeds.erase(ray_speeds.begin(), ray_speeds.end());
    m_receiversImpulse.erase(receiver);
    m_dopplerSpectrum.erase(receiver);
    m_receiver_speed = movement;
    m_receiver_speed.setLength(m_receiver_speed.length()/3.6);
    m_ray_speed = QLineF(QPointF(0.0,0.0),QPointF(0.0,0.0));

    m_chosenBeams[receiver] = false;

    if (m_receiversRays.count(receiver)) {
        for (unsigned int i = 0; i < m_receiversRays[receiver].size(); i++) {
            for (unsigned int j = 0; j < m_receiversRays[receiver].at(i)->size(); j++) {
                delete m_receiversRays[receiver].at(i)->at(j);
            }
        }
        m_receiversRays[receiver].erase(m_receiversRays[receiver].begin(),
                                        m_receiversRays[receiver].end());
        m_receiversRays[receiver].shrink_to_fit();
    }

    const QPointF &pos = *receiver->getPos();

    if (m_zone.containsPoint(pos, Qt::OddEvenFill)) {
        //      The receiver is in the illumination zone

        WholeRay *wholeRay = new WholeRay;
//        QPointF m_pos(int(this->x()), int(this->y()));
        wholeRay->push_back(m_rayFactory->createRay(*this, pos));
        m_receiversRays[receiver].push_back(wholeRay);
    }
}

void MathematicalTransmitterProduct::attachObservable(ProductObservable *productObservable)
{
    m_productObservable.push_back(productObservable);
    m_chosenBeams[productObservable] = false;
}


void MathematicalTransmitterProduct::drawRays(ProductObservable *productObservable, bool draw)
{
    if (draw) {
        for (unsigned int i = 0; i < m_receiversRays[productObservable].size(); i++) {
            for (unsigned j = 0 ; j < m_receiversRays[productObservable].at(i)->size(); j++) {
                m_receiversRays[productObservable].at(i)->at(j)->draw();
            }
        }
    } else {
        for (unsigned int i = 0; i < m_receiversRays[productObservable].size(); i++) {
            for (unsigned int j = 0; j < m_receiversRays[productObservable].at(i)->size(); j++) {
                m_receiversRays[productObservable].at(i)->at(j)->erase();
            }
        }
    }
}


Data * MathematicalTransmitterProduct::getChData(ProductObservable *rx)
{
    // Rice Factor
    m_chsData[rx].riceFactor = 10*log10(m_los_factor[rx]/m_nlos_factor[rx]);

    // Angular Spread
    angularSpread(rx);

    return &m_chsData[rx];
}

void MathematicalTransmitterProduct::angularSpread(ProductObservable *rx)
{
    double prx = 0;
    double variance = 0;
    double mean = 0;

    const vector<double> &prxAngularSpread = m_chsData.at(rx).prxAngularSpctr;
    const vector<double> &u = m_chsData.at(rx).u;

    for (unsigned i = 0; i < prxAngularSpread.size(); i++) {
        prx += prxAngularSpread.at(i);
        variance += pow(u.at(i), 2) * prxAngularSpread.at(i);
        mean += u.at(i) * prxAngularSpread.at(i);
    }

    m_chsData.at(rx).angularSpred = sqrt(variance/prx - pow(mean, 2));
}

void MathematicalTransmitterProduct::compute(ProductObservable *receiver)
{
    if (!m_beamsFrozen && !m_chosenBeams[receiver]) {

        chooseBeam(receiver);
        comput4FixedBeam(receiver);

    } else {

        comput4FixedBeam(receiver);
    }

    double totalPower = computePrx(m_receiversField[receiver],m_receiversGroundField[receiver],receiver);

    double powerDBm = dBm(totalPower);

    // m_algorithm->sendData(this, dynamic_cast<MathematicalProduct *>(receiver));
    receiver->answer(this, m_frequency, m_bandwidth, powerDBm, m_receiversField[receiver]);
}


double
MathematicalTransmitterProduct::computePathLossPower(ProductObservable* copy_receiver)
{
    double powerAtReceiver = 0;
    complex<double> emField = 0;
    complex<double> groundField = 0;

    for (unsigned i = 0; i < m_receiversRays[copy_receiver].size(); i++)
    {
        WholeRay *wholeRay = m_receiversRays[copy_receiver].at(i);

        if (wholeRay->at(0)->getDiffracted())
        {
            map<ProductObservable *, map<double, double>>::iterator it;
            complex<double>EMfield = computeDiffractedEfield(copy_receiver,wholeRay,false);
            emField += EMfield;
        }
        else
        {
            complex<double> EMfield = computeEMfield(wholeRay, copy_receiver,false);
            if (wholeRay->size() == 1) {
                // Adding the ground component
                double angle_transmitter = wholeRay->back()->angle();
                groundField = computeEfieldGround(copy_receiver,angle_transmitter,false); // Compute the electrical field from the ray reflected off the ground
            }
            emField += EMfield;
        }
    }
    double totalPower = computePrx(emField,groundField,copy_receiver);

    powerAtReceiver = dBm(totalPower);

    return powerAtReceiver;
}

complex<double> MathematicalTransmitterProduct::computeInterference(ProductObservable* copy_receiver,QLineF local_region)
{
    complex<double> impulse_r = 0;
    vector<WholeRay *> wholeRays = m_receiversRays[copy_receiver];

    for (unsigned j = 0; j < wholeRays.size(); j++)
    {
        WholeRay *wholeRay  = wholeRays.at(j);

        if (wholeRay->at(0)->getDiffracted())
        {
            impulse_r += computeImpulseDiffraction(wholeRay, local_region);
        }
        else
        {
            int amountSegment = wholeRay->size();
            if (amountSegment == 1)
            {
                double angle_transmitter = wholeRay->back()->angle();
                impulse_r += computeImpulseGroundReflection(copy_receiver, angle_transmitter, local_region);
            }
            impulse_r += computeImpulseReflection(wholeRay,local_region);
        }
    }
    return impulse_r;
}

// ---------------------------------------------------- ModelObserver -------------------------------------------------------------------


void MathematicalTransmitterProduct::attachObservable(ModelObservable *modelObservable)
{
    // ????????????
    m_model = modelObservable;
}


// ---------------------------------------------------- AbstractAntenna ---------------------------


void MathematicalTransmitterProduct::notifyParent(ProductObservable *receiver,
                                                  QLineF const movement,
                                                  const QPointF &point,
                                                  WholeRay *wholeRay)
{

    //
    //      Called by the transmitter images and the diffraction points.
    //

    MathematicalRayProduct *newRay = m_rayFactory->createRay(*this, point);
    wholeRay->push_back(newRay);
    m_receiversRays[receiver].push_back(wholeRay);
    ray_speeds[wholeRay] = movement;
}


void MathematicalTransmitterProduct::notifyCarDetected()
{
    emit detectsCar(this);
}


QPointF MathematicalTransmitterProduct::getPosition() const
{
    return *this;
}


QPolygonF MathematicalTransmitterProduct::getIlluminationZone(const QRectF &rect) const
{
    return QPolygonF(rect);
}


QPolygonF MathematicalTransmitterProduct::getIlluminationZone() const
{
    return m_zone;
}


QPolygonF MathematicalTransmitterProduct::getIlluminatedZone() const
{
    return m_zone;
}


void MathematicalTransmitterProduct::setIlluminatedZone(const QPolygonF &zone)
{
    m_zone = zone;
}


void MathematicalTransmitterProduct::carMoved(MathematicalCarProduct *car,
                                              int /*x*/, int /*y*/,
                                              double /*orientation*/)
{
    int idx = 0;
    if (m_zone.intersects(*car)) {

        m_illuminatedCars.push_back(car);
        emit detectsCar(this);
    }

    else if (inIlluminatedCars(car, &idx)) {

        m_illuminatedCars.erase(m_illuminatedCars.begin() + idx);
        emit detectsCar(this);
    }
}
