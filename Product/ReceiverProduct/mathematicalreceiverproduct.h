#ifndef MATHEMATICALRECEIVERPRODUCT_H
#define MATHEMATICALRECEIVERPRODUCT_H

//#include <stdio.h>  /* defines FILENAME_MAX */
//#ifdef WINDOWS
//#include <direct.h>
//#define GetCurrentDir _getcwd
//#else
//#include <unistd.h>
//#define GetCurrentDir getcwd
//#endif


#include <QPointF>
#include <iostream>
#include <vector>
#include <fstream>


#include "Product/mathematicalproduct.h"
#include "Product/graphicsproduct.h"
#include "Observer/productobservable.h"
#include "receiverproduct.h"
#include "dialogreceiverproduct.h"
#include "Widget/infowidget.h"


const double kb = 1.379e-23;
const double To = 290.0;
const double c = 2.998e+8;

using namespace std;

class MathematicalReceiverProduct: public QPointF, public MathematicalProduct,
                                   public ProductObservable, public ReceiverProduct,
                                    public QObject
{
public:
    MathematicalReceiverProduct(int posX, int posY);
    MathematicalReceiverProduct(MathematicalReceiverProduct *receiver);
    ~MathematicalReceiverProduct() override;

    void clearData();
    double inputNoise();
    void computeMinPrx();
    void setInfoWidget(InfoWidget* info) {m_info_widget = info;}
    void setTransmitterDistance(double distance) {m_transmitter_distance = distance;}
    void computeSnr();
    void computeDelaySpread();
    void riceFactor(double rice);
    void coherenceBandwidth();
    void attachTransmitter(ProductObserver *transmitter);


    // 1. Path Loss Computation:
    std::map<double,double> pathLoss(){return m_pathloss;}
    void setPathLoss(std::map<double,double> pathloss);
    void linearRegressionPathLoss();
    void computePathLossFading();
    double standardDeviation();
    void modelPathLoss();

    // 2. Impulse Response and TDL Computation:
    std::map<std::vector<double>,double> getimpulseTau() {return m_tau;}
    std::map<std::vector<double>,std::complex<double>> impulseAttenuation() {return m_attenuation;}
    void setImpulseTau(std::map<std::vector<double>,double> raylength){m_tau = raylength;}
    void setImpulseAttenuation(std::map<std::vector<double>,std::complex<double>> attenuation);
    void computeImpulseTDL();

    // 3. Cell Range Computation:
    void cellRange();

    // 4. Doppler
    void setDopplerShift(map<vector<double>,double> dopplershift);


    // From ReceiverProduct:
    float getSpeed() override;
    float getOrientation() override;
    QLineF movement() const;
    int getPosX() override;
    int getPosY() override;
    double getPower() override;
    complex<double> getEField() override;
    bool getEnable() override;
    int targetSNR() override {return m_target_snr;}
    int noiseFigure() override {return m_noise_figure;}
    int interFerenceMargin() override {return m_interferencemargin;}

    void setSpeed(float speed) override;
    void setOrientation(float orientation) override;
    void setMovement(QLineF const movement);
    void setPosX(int posX) override;
    void setPosY(int posY) override;
    void setPower(double p) override;
    void setEField(complex<double> e) override;
    void setEnable(bool enable) override;
    void setTargetSNR(int target) override {m_target_snr = target;}
    void setNoiseFigure(int figure) override {m_noise_figure = figure;}
    void setInterferecenceMargin(int interference) override {m_interferencemargin = interference;}

    void newProperties() override;

    // From MathematicalProduct
    void update(QGraphicsItem *graphic) override;
    void attachObservable(GraphicsProduct* graphic) override;
    void openDialog() override;
    void updateInformation() override;


    // From ProductObservable
    void attachObserver(ProductObserver *productObserver) override;
    void detachObservers() override;
    void notifyObservers();
    void notify() override;
    void notify(double &power, std::vector<double> *powers, std::complex<double> &EMfiled) override;
    void answer(ProductObserver *observer, double &power, std::vector<double> *powers, std::complex<double> &EMfield) override;
    QPointF* getPos()override;

private:
    // 0. Intrisic parameters
    bool enable;
    //float m_speed;
    //float m_orientation;
    QLineF m_movement;
    int m_target_snr;
    int m_noise_figure;
    int m_interferencemargin;
    DialogReceiverProduct *m_dialog = nullptr;
    InfoWidget *m_info_widget;

    // 1. For ProductOBserves
    vector<ProductObserver *> m_observers;
    vector<ProductObserver *> m_transmitters;
    ProductObserver *m_transmitter = nullptr;

    // 2. For E Field And Power Computation
    complex <double> m_e_field;
    double m_power = 0;
    double m_transmitter_distance;
    double snr_received;
    double delay_spread;
    double rice_factor;
    double coherence_bandwidth;

    // 3. For Path Loss Computation
    std::map<double, double> m_pathloss;
    QVector<double> logD, fading, logD_model;

    // 4. For Impulse Response and TDL
    std::map<std::vector<double>,double> m_tau;
    std::map<std::vector<double>,std::complex<double>> m_attenuation;

    // 5. Doppler
    std::map<std::vector<double>,double> m_doppler_shift;

public slots:
    void save(string path);
};

#endif // MATHEMATICALRECEIVERPRODUCT_H
