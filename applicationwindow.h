#ifndef APPLICATIONWINDOW_H
#define APPLICATIONWINDOW_H
#include <vector>

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QGraphicsView>
#include <QButtonGroup>
#include <QGridLayout>
#include <QToolButton>
#include <QLabel>
#include <QToolBox>
#include <QAction>
#include <QMenu>
#include <QMenuBar>

#include "Abstract_Factory/scenefactory.h"
#include "Abstract_Factory/graphicsfactory.h"
#include "Abstract_Factory/dialogfactory.h"
#include "Abstract_Factory/mathematicalfactory.h"
#include "Observer/windowobservable.h"

using namespace std;

class ApplicationWindow :public QMainWindow,public WindowObservable
{
    Q_OBJECT
public:
    ApplicationWindow(QWidget *parent = nullptr);
    ~ApplicationWindow() override;

    void attachObserver(WindowObserver *windowobserver) override;
    void detachObserver(WindowObserver *windowobserver) override;
    void notify(int mode) override;
    void answer() override;

    QWidget* createToolButton(const QString &text,int id);
    void createActions();
    void createMenus();
    void createToolBox();

    void setMode(Mode mode);

private:
    QGraphicsView *view;
    SceneFactory *graphicsfactory,*dialogfactory,*mathematicalfactory;
    vector<WindowObserver*> m_windowobserver;
    vector<WindowObserver*>::iterator m_windowobserveriterator;

    Mode m_mode;

    QButtonGroup *m_antennagroup;
    QButtonGroup *m_obstaclegroup;
    QButtonGroup *m_raytracinggroup;
    QToolBox *m_toolbox;

    QAction *deleteaction;
    QAction *propertiesaction;

    QMenu *m_productmenu;


signals:

public slots:
    void antennaGroupClicked(int);
    void obstacleGroupClicked(int);
    void rayTracingGroupClicked(int);
    void deleteProduct();
    void openProduct();
};

#endif // APPLICATIONWINDOW_H
