#ifndef plots_H
#define plots_H

#include <QDialog>
#include "room.h"

namespace Ui {
class plots;
}

class room;

class plots : public QDialog
{
    Q_OBJECT

public:
    explicit plots(QWidget *parent = 0);
    ~plots();
    int linreg(int n, QVector<double> x, QVector<double> y, double* m, double* b, double* r);
    double findStandardDeviation(QVector<double> array);
    void plotPathLoss(room *scene);
    void plotModel(double m, double b, double fadingVariability, double minPrx);
    void plotCellRange(double m, double b, double fadingVariability, double minPrx);
    void physicalImpulseResponse(room* scene);
    void TDLImpulseResponse(room* scene);
    void TDL_US(room* scene);
    void dopplerSpectrum(room* scene);

private:
    Ui::plots *ui;
    double minDBM = -102; // Minimal power[dBm] to establish a reliable communication
};

#endif // plots_H
