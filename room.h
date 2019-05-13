#ifndef ROOM_H
#define ROOM_H


// Dependencies
#include "MainWindow.h"
#include "antena.h"
#include "math.h"
#include "wall.h"
#include "ray.h"
#include "lineo.h"

// Libraries
#include <complex>
#include <vector>
#include <array>
#include "math.h"

using namespace std;

class antena;
class wall;
class MainWindow;
class ray;

class room : public QGraphicsScene//, private QImage
{

    Q_OBJECT
public:
    explicit room(MainWindow *parent = 0);
    ~room(void);

    void launch_algo(bool drawR);

    float distance();
    void drawWall(qreal &x1, qreal &y1, qreal &x2, qreal &y2);

    // Getters && Setters
    double getReceivedPower();
    antena* getTransmitter();
    antena* getReceiver();
    wall* getWalls(int i);
    int getAmountWalls();
    double getBinaryDebit();
    int getAmountDiscret();
    double getpowerEmettor();
    double getInitBinaryDeb();
    complex <double> getTotalEfield();
    int getRows();
    int getColumns();
    int getTotalArea();
    double* getData();
    int getMinimalDistance();
    int getSquare_size();
    double getPxToMeter();
    int getRayNumber();
    double* getChannelData();
    map<const char*,int>* getStreetsPenDep();
    double getCarrierFrequency();
    double getBandwidth();
    double getMinPrx();
    
    double getPrx(int posX, int posY);
    double getSNR(int posX, int posY);
    double getDelay(int posX, int posY);
    double getCoherenceBandwidth(int i, int j);
    double getRiceFactor(int i, int j);
    double getDistance(int i, int j);

    double getDelay_local();
    double getCoherenceBandwidth_local();
    double getRiceFactor_local();
    double getSNR_local();

    void setTransmitter(antena *new_transmitter);
    void setReceiver(antena *new_receiver);
    void setAntenaType(int type);

    //Misc tools
    void readSettingsFile();
    void clearAll();
    void clearLocalParameters();
    void clearPenDepth();

    void drawCoverege();
    bool DataComputed();
    void getDataIndices(int posX, int posY, int &index_i, int &index_j);
    void getTxIndices(int &index_i, int &index_j);
    bool workingZone(int x, int y);

    // Telecom calculation tools
    double dB(double power);
    double dBm(double power);
    double dBmRev(double dbm);
    double binaryDebit(double power);

signals:

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *);

    int antenaType;

private:

    // Qt visuals
    MainWindow *myParent;
    QGraphicsView *graphicsView;

    // Graphical parameters
    double pxToMeter = 0.1;
    int square_size = 10;
    int rows = 500/square_size; // 950 = window width
    int columns = 950/square_size; // 500 = window height
    int totalArea = rows * columns; // total number of local area
    int minimalDistance = 10; // 10m

    // General objects
    antena *Transmitter;
    antena *Receiver;
    wall *walls[28];    // For easier use walls are put in arrays
    vector <std::array <double,2>> diffractionPoints;
    //lineo *uselessWalls[10];


    // --> Global variables (electrical constants)

    double  epsilonAir = 8.864e-12; // A²S⁴kg⁻1m⁻3
    double  Zvoid = 120*M_PI;
    double  muAir = 4*M_PI*1e-7;    // Tm/A
    double  c = 2.998e+8;           // m/s
    double  kb = 1.379e-23;         // Boltzmann's constant
    double  T0 = 290;               // K; reference temperature T_0
    double  freq = 26e+9;           // Hz
    double  BW = 100e+6;            // Hz
    double  antennaHeight = 1.8;    //m

    double lambda = c/freq;
    // double alpha;
    // double beta;
    double Beta = 2*M_PI*freq*sqrt(muAir*epsilonAir); // Used for the diffraction.

    // complex <double> gamma;
    //double power = 0;
    double diffractedPower = 0;


    double  Ra = 71.0;   // Ohms, its a typical resistance data for \lambda/2 emettors
    struct Drawer;

    // Algo parameters
    int ray_counter;
    vector< vector <ray*>> allRays;  // Declaring the multi-ray vector
    vector < vector <ray*>> allDiffractedRays;

    vector <ray*> completeRay;
    vector <ray*> current; // algo power

    //complex <double> Efield = 0;
    complex <double> totalEfield = 0.0;


    //lineo *current_wall = NULL;
    lineo *current_ray;
    QLineF lineToVirtualSource;

    int recursionState = 0;
    wall* wallRecursiveNumber[9];
    double transmitterSerie[9][2];


    double angle_wall;
    qreal x2wall;
    qreal y2wall;

    double imCoordinates[2];

    // Wall parameters
    double wallThickness;
    double wallSigma;
    double wallEps;
    double epsilonWallRel;

    // Problem parameters
    int reflectionsNumber;
    unsigned int amount_walls = 18;
    int amount_useless_walls = 10;
    unsigned int amount_all_walls = 28;
    int amount_discret = 20;
    map<const char*,int> streetsPenDep;
    double stDepth[5] = {0,0,0,0,0}; // 0 - commerce up, 1 - deux eg, 2 - spa, 3, commrece down, 4 - ind

    struct streets{
        int laLoi[4]= {1,200,950,300};

        int commerceUp[4] = {200,1,250,200};
        int commerceDown[4] = {200,300,250,500};
        int deuxEg[4] = {450,1,500,200};
        int spa[4] = {700,1,750,200};
        int indu[4] = {600,300,650,500};
    };

    streets* st = new streets();

    double minLength, maxLength = 0;
    double *Data = nullptr;
    bool coverageDone = false;

    // System Parameters
    double  maxEIRP = 1; // Watt
    double  n = 1; // n = efficiency of the antenna
    double  maxGain = n*16/(3*M_PI);
    double  L_Tx = 1; //L_Tx
    double  powerEmettor = maxEIRP*L_Tx/maxGain;   // In watts the power of the emettor
    double  Zwall;
    double  eps;
    bool    diffractOn = false;

    // SNR parameters
    double targetSNR = 8; //[dB] 
    double noiseFigure = 10; //[dB]
    double inputNoise = 10*log10(kb*T0*BW);
    double interferenceMargin = 6; //[dB]

    double minPrx = targetSNR + noiseFigure + inputNoise + interferenceMargin - 30; // +30 to convert dB -> dBm

// ---------- Results ------------------------
    double resultsBinaryDebit;
    double powerReceived;
    double powerRef;
    double SNR;
    double LOS;
    double NLOS;

    int rayNumber = 0; // Help to keep track of the number of rays
    bool computePhysicalResponse; // If we compute the channel response or not
    double channelData[2*20] = {}; // 2 * 10 rays

// ---------- Methods ------------------------

    // Computation methods
    void imagesMethod(double transmitterPosX,double transmitterPosY ,double reveiverPosX,double receiverPosY, int numberOfReflections);
    void recursion(double transmitterPosX, double transmitterPosY, double receiverPosX, double receiverPosY, int numberOfReflections, void (*draw)(double, double, double, double, room *));
    static void drawRay(double TransmitterImagePosX, double TransmitterImagePosY, double OriginX, double OriginY, room *scene);
    static void buildRay(double TransmitterPosX, double TransmitterPosY, double OriginX, double OriginY, room *scene);
    bool intersectionCheck(lineo* line1, lineo* line2);
    bool intersectionCheckNonInclusive(lineo* line1, lineo* line2);
    bool checkTransmission(lineo* line1, lineo* line2, int x1, int y1,int x2,int y2);
    static void drawDiffraction(room* scene);
    static void buildDiffraction(room* scene);
    void setUpStreets();
    void penetrationDepth();
    bool onStreet(int street[]);

    // Numerical analysis

    complex <double> FtIntegral(double x);

    // Geometric methods
    //bool pointOnLine(lineo* line1, double x, double y);
    bool pointOnLine(lineo* line1, const double xp, const double yp);
    bool pointOnLineNonInclusive(lineo* line1, const double xp, const double yp);
    vector<double> intersection(lineo* line1, lineo* line2 );

//    double distInWall(double tetai);
    void distCorrection(vector<ray*> oneCompleteRay, wall walls[]);

    bool commonToAnyWall(double posX, double posY, int indWall);

    void drawWalls();
    void findDiffractionPoints();

    double diffractedRayPower(ray* rayReceiver, ray*rayTransmitter);

    //Misc
    void setDefaultSettings();

    double computePrx(complex <double> totalEfield);
    complex <double> computeEfield(vector<ray*> rayLine);
    double computeReflexionPar(double thetaI, double epsilonR);
    double computeReflexionPer(double thetaI, double epsilonR);
    complex <double> computeEfieldGround();
    double computeSNR(double Prx);

public slots:


};

#endif // ROOM_H
