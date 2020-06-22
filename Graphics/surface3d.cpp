#include "surface3d.h"

#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DTheme>
#include <QtGui/QImage>
#include <QtCore/qmath.h>

using namespace QtDataVisualization;

const int sampleCountY = 50;
const int sampleCountTheta = 180;
const int sampleCountPhy = 90;
const float sampleThetaMin = 0.0;
const float sampleThetaMax = 360.0;
const float samplePhyMin = 0.0;
const float samplePhyMax = 180.0;

Surface3D::Surface3D(Q3DSurface *surface, DialogTransmitterProduct *dialog)
    : m_graph(surface), m_dialog(dialog), m_thetamin(0.0),m_thetamax(0.0),m_ymin(0.0),m_ymax(0.0),
      m_rmin(0.0),m_rmax(0.0)
{
    m_graph->setAxisX(new QValue3DAxis);
    m_graph->setAxisY(new QValue3DAxis);
    m_graph->setAxisZ(new QValue3DAxis);
    m_graph->setPolar(true);
    m_graph->setShadowQuality(QAbstract3DGraph::ShadowQualityHigh);

    m_patternProxy = new QSurfaceDataProxy();
    m_patternSeries = new QSurface3DSeries(m_patternProxy);
    fillPatternProxy();
}

Surface3D::~Surface3D(){

}

void Surface3D::fillPatternProxy()
{
    float stepTheta = (sampleThetaMax - sampleThetaMin) / float(sampleCountTheta - 1);
//    float stepPhy = (samplePhyMax - samplePhyMin) / float(sampleCountPhy - 1);
//    float frequency = m_dialog->getFrequency();
//    int row = m_dialog->getRow();
//    int column = m_dialog->getColumn();
//    double antennaDistance = m_dialog->getAntennaDistance();

//    m_dataArray = new QSurfaceDataArray;
//    m_dataArray->reserve(sampleCountPhy);
//    for (int i = 0 ; i < sampleCountPhy ; i++) {
//        QSurfaceDataRow *newRow = new QSurfaceDataRow(sampleCountTheta);
//        // Keep values within range bounds, since just adding step can cause minor drift due
//        // to the rounding errors.
//        float phy = qMin(samplePhyMax, (i * stepPhy + samplePhyMin));
//        int index = 0;
//        for (int j = 0; j < sampleCountTheta; j++) {
//            float theta = qMin(sampleThetaMax, (j * stepTheta + sampleThetaMin));
//            double gain = MathematicalTransmitterProduct::computeGain(double(theta),double(phy),frequency,row,column,antennaDistance);
//            float xtheta = theta*M_PI/180.0;
//            m_thetamax = qMax(xtheta,m_thetamax);
//            m_thetamin = qMin(xtheta,m_thetamin);
//            float y = gain*qCos(phy*M_PI/180.0);
//            m_ymin = qMin(y,m_ymin);
//            m_ymax = qMax(y,m_ymax);
//            float r = gain;
//            m_rmax = qMax(r,m_rmax);
//            m_rmin = qMin(r,m_rmin);
//            (*newRow)[index++].setPosition(QVector3D(xtheta, y, r));
//        }
//        *m_dataArray << newRow;
//    }
//    m_graph->axisX()->setRange(m_thetamin, m_thetamax);
//    m_graph->axisY()->setRange(m_ymin-1.0, m_ymax+1.0);
//    m_graph->axisZ()->setRange(m_rmin, m_rmax);
//    m_patternProxy->resetArray(m_dataArray);
}

void Surface3D::enablePatternModel(bool enable)
{
    if (enable) {
        m_patternSeries->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
        m_patternSeries->setFlatShadingEnabled(true);

        m_graph->axisX()->setLabelFormat("%.2f");
        m_graph->axisZ()->setLabelFormat("%.2f");
        m_graph->axisX()->setRange(m_thetamin, m_thetamax);
        m_graph->axisY()->setRange(m_ymin, m_ymax);
        m_graph->axisZ()->setRange(m_rmin, m_rmax);
        m_graph->axisX()->setLabelAutoRotation(30);
        m_graph->axisY()->setLabelAutoRotation(90);
        m_graph->axisZ()->setLabelAutoRotation(30);

        m_graph->addSeries(m_patternSeries);

        m_stepTheta = (sampleThetaMax - sampleThetaMin) / float(sampleCountTheta - 1);
        m_stepY = (m_ymax - m_ymin) / float(sampleCountY - 1);
    }
}

void Surface3D::changeTheme(int theme)
{
    m_graph->activeTheme()->setType(Q3DTheme::Theme(theme));
}

void Surface3D::setBlackToYellowGradient()
{
    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::black);
    gr.setColorAt(0.33, Qt::blue);
    gr.setColorAt(0.67, Qt::red);
    gr.setColorAt(1.0, Qt::yellow);

    m_graph->seriesList().at(0)->setBaseGradient(gr);
    m_graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}

void Surface3D::setGreenToRedGradient()
{
    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::darkGreen);
    gr.setColorAt(0.5, Qt::yellow);
    gr.setColorAt(0.8, Qt::red);
    gr.setColorAt(1.0, Qt::darkRed);

    m_graph->seriesList().at(0)->setBaseGradient(gr);
    m_graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}
