#include "applicationwindow.h"

ApplicationWindow::ApplicationWindow(QWidget *parent) : QMainWindow(parent)
{
    createToolBox();
    createActions();
    createMenus();
    view = new QGraphicsView();
    m_scene = new GraphicScene(view,this,m_productmenu);
    m_model = new Model(this);
    m_receiverFactory = new ReceiverFactory(m_productmenu);
    m_transmitterFactory = new TransmitterFactory();
    m_buildingFactory = new BuildingFactory();
    m_treeFactory = new TreeFactory();
    m_carFactory = new CarFactory();
    m_rayTracingAlgorithmFactory = new RayTracingAlgorithmFactory();

    //dialogfactory = new DialogFactory(dynamic_cast<SceneObservable*>(graphicsfactory));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_toolbox);
    layout->addWidget(view);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    setCentralWidget(widget);

    m_mode = MoveItem;
}

ApplicationWindow::~ApplicationWindow(){

}

void ApplicationWindow::attachObserver(WindowObserver *windowobserver){
    m_windowobserver.push_back(windowobserver);
}

void ApplicationWindow::detachObserver(WindowObserver *windowobserver){
    unsigned long long i = 0;
    for (m_windowobserveriterator = m_windowobserver.begin();
         m_windowobserveriterator != m_windowobserver.end();
         ++m_windowobserveriterator){
        if (m_windowobserver.at(i) == windowobserver){
            m_windowobserver.erase(m_windowobserveriterator);
        }
        i++;
    }
}

void ApplicationWindow::notify(int mode){
    for (unsigned long long i=0;i<m_windowobserver.size();i++){
        m_windowobserver.at(i)->update(mode);
    }
}

void ApplicationWindow::answer(SceneProduct *sceneproduct){
    cout<<"Answer"<<endl;
    m_sceneProducts.push_back(sceneproduct);

//    notify(int(m_mode));
    QList<QAbstractButton*> antennabuttons = m_antennagroup->buttons();
    QList<QAbstractButton*> obstaclebuttons = m_obstaclegroup->buttons();
    for(QAbstractButton *button: antennabuttons){
        button->setChecked(false);
    }
    for(QAbstractButton *button: obstaclebuttons){
        button->setChecked(false);
    }
    m_model->addMathematicalComponent(sceneproduct->toMathematicalComponent());
    m_mode = MoveItem;
    cout<<"Item added to model"<<endl;
}

void ApplicationWindow::modelAnswer(vector<MathematicalComponent *> sceneproducts){
    for(int i = 0; i< sceneproducts.size();i++){
        m_scene->addItem((QGraphicsItem*)sceneproducts.at(i)->toGraphicsComponent());
    }
}

void ApplicationWindow::modelAnswer(vector<MathematicalRayProduct> *sceneproducts){
    for(int i = 0; i< sceneproducts->size();i++){
        //m_scene->addItem((QGraphicsItem*)sceneproducts->at(i).toGraphicsComponent());
        GraphicsComponent* gComp = sceneproducts->at(i).toGraphicsComponent();
        //QGraphicsItem* gItem = (QGraphicsItem*)sceneproducts->at(i).toGraphicsComponent();
        QGraphicsItem* gItem = dynamic_cast<QGraphicsItem*>(sceneproducts->at(i).toGraphicsComponent());
        //m_scene->addItem(dynamic_cast<QGraphicsItem*>(sceneproducts->at(i).toGraphicsComponent()));
        m_scene->addItem(gItem);
        cout<<"Item added to scene"<<endl;
    }
}

void ApplicationWindow::modelNotify(vector<SceneProduct *> sceneproducts){

}

void ApplicationWindow::modelNotify(vector<MathematicalRayProduct *> sceneproducts){

}

QWidget* ApplicationWindow::createToolButton(const QString &text, int mode){
    QWidget *widget = new QWidget;
    QToolButton *button = new QToolButton;
    button->setIconSize(QSize(50, 50));
    button->setCheckable(true);
    QIcon icon;
    switch(mode){
        case int(InsertTransmitter):
            icon = QIcon(GraphicsTransmitterProduct::getImage());
            button->setIcon(icon);
            m_antennagroup->addButton(button,mode);
            break;
        case int(InsertReceiver):
            icon = QIcon(GraphicsReceiverProduct::getImage());
            button->setIcon(icon);
            m_antennagroup->addButton(button,mode);
            break;
        case int(InsertBuilding):
            icon = QIcon(GraphicsBuildingProduct::getImage());
            button->setIcon(icon);
            m_obstaclegroup->addButton(button,mode);
            break;
        case int(InsertTree):
            icon = QIcon(GraphicsTreeProduct::getImage());
            button->setIcon(icon);
            m_obstaclegroup->addButton(button,mode);
        break;
        case int(InsertCar):
            icon = QIcon(GraphicsCarProduct::getImage());
            button->setIcon(icon);
            m_obstaclegroup->addButton(button,mode);
        break;

    }
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(button, 0, 0, Qt::AlignHCenter);
    layout->addWidget(new QLabel(text), 1, 0, Qt::AlignCenter);

    widget->setLayout(layout);
    return widget;
}

void ApplicationWindow::createActions(){
    deleteaction = new QAction(QIcon(":/Images/Delete.png"), tr("&Delete"), this);
    deleteaction->setShortcut(tr("Delete"));
    deleteaction->setStatusTip(tr("Delete selected object"));

    propertiesaction = new QAction(QIcon(":/Images/Properties.png"), tr("&Properties"), this);
    propertiesaction->setShortcut(tr("Open"));

    connect(deleteaction, SIGNAL(triggered()), this, SLOT(deleteProduct()));
    connect(propertiesaction, SIGNAL(triggered()), this, SLOT(openProduct()));

}

void ApplicationWindow::createMenus(){
    m_productmenu = menuBar()->addMenu(tr("&Object"));
    m_productmenu->addAction(propertiesaction);
    m_productmenu->addSeparator();
    m_productmenu->addAction(deleteaction);
}

void ApplicationWindow::createToolBox(){
    /*
     * Creation of ths sidepannel butons
    */
    m_antennagroup = new QButtonGroup(this);
    m_antennagroup->setExclusive(false);
    m_obstaclegroup = new QButtonGroup(this);
    m_obstaclegroup->setExclusive(false);
    m_raytracinggroup = new QButtonGroup(this);
    m_raytracinggroup->setExclusive(false);
    connect(m_antennagroup,SIGNAL(buttonClicked(int)),this,SLOT(antennaGroupClicked(int)));
    connect(m_obstaclegroup,SIGNAL(buttonClicked(int)),this,SLOT(obstacleGroupClicked(int)));
    connect(m_raytracinggroup,SIGNAL(buttonClicked(int)),this,SLOT(rayTracingGroupClicked(int)));
    QGridLayout *antenna_layout = new QGridLayout;
    QGridLayout *obstacle_layout = new QGridLayout;
    QGridLayout *rayTracing_layout = new QGridLayout;

    // Creating the antennas pannel
    QWidget* widget = createToolButton("Transmitter",int(InsertTransmitter));
    antenna_layout->addWidget(widget, 0, 0);
    QWidget* widget1 = createToolButton("Receiver", int(InsertReceiver));
    antenna_layout->addWidget(widget1, 0, 1);

    antenna_layout->setRowStretch(1,10);
    antenna_layout->setColumnStretch(2, 10);

    // Creating the obstacle pannel
    obstacle_layout->setHorizontalSpacing(10);

    QWidget* obstacle_widget = createToolButton("Building",int(InsertBuilding));
    obstacle_layout->addWidget(obstacle_widget, 0, 0);
    QWidget* tree_widget = createToolButton("Tree", int(InsertTree));
    obstacle_layout->addWidget(tree_widget,0,1);
    QWidget* car_layout = createToolButton("Car", int(InsertCar));
    obstacle_layout->addWidget(car_layout,1,0);

    obstacle_layout->setRowStretch(3, 10);
    obstacle_layout->setColumnStretch(2, 10);
    obstacle_layout->setHorizontalSpacing(10);
    QWidget *itemWidget = new QWidget;
    itemWidget->setLayout(antenna_layout);

    QWidget *obstacleWidget = new QWidget;
    obstacleWidget->setLayout(obstacle_layout);

    // Creating the rayTraycing pannel

    QWidget *launchRayTracingWidget = new QWidget;
    QToolButton *launchRayTracingButton = new QToolButton;
    launchRayTracingButton->setIconSize(QSize(50, 50));
    launchRayTracingButton->setCheckable(false);
    QIcon icon  = QIcon(QPixmap(":/Images/playButton.png"));
    launchRayTracingButton->setIcon(icon);
    m_raytracinggroup->addButton(launchRayTracingButton,int(LaunchRayTracing));

    QGridLayout *playButton_layout = new QGridLayout;
    playButton_layout->addWidget(launchRayTracingButton, 0, 0, Qt::AlignHCenter);
    playButton_layout->addWidget(new QLabel("Launch Ray"), 1, 0, Qt::AlignCenter);

    launchRayTracingWidget->setLayout(playButton_layout);



    rayTracing_layout->addWidget(launchRayTracingWidget, 0, 0);

    rayTracing_layout->setRowStretch(1, 10);
    rayTracing_layout->setColumnStretch(2, 10);

    QWidget *rayTracingWidget = new QWidget;
    rayTracingWidget->setLayout(rayTracing_layout);

    m_toolbox = new QToolBox;
    m_toolbox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Ignored));
    m_toolbox->setMinimumWidth(itemWidget->sizeHint().width());
    m_toolbox->addItem(itemWidget, tr("Insert antenna"));
    m_toolbox->addItem(obstacleWidget, tr("Insert obstacles"));
    m_toolbox->addItem(rayTracingWidget, tr("Ray Tracing"));
}

void ApplicationWindow::setMode(Mode mode){
    m_mode = mode;
}

void ApplicationWindow::notifyScene(){
    SceneFactory * factory;
    switch (m_mode) {
        case int(InsertReceiver):
            factory = m_receiverFactory;
            break;
        case int(InsertTransmitter):
        factory = m_transmitterFactory;
            break;
        case int(InsertBuilding):
        factory = m_buildingFactory;
            break;
        case int(InsertTree):
        factory = m_treeFactory;
            break;
        case int(InsertCar):
        factory = m_carFactory;
            break;
    }
    m_scene->setSceneFactory(factory);
}

void ApplicationWindow::notifyModel(){
    cout<<"Model notified"<<endl;
    m_model->launchAlgorithm(m_rayTracingAlgorithmFactory);
}

// SLOTS

void ApplicationWindow::antennaGroupClicked(int mode){
    QList <QAbstractButton*> buttons = m_antennagroup->buttons();
    for (int i = 0; i < buttons.size(); i++){
        if (buttons.at(i) != m_antennagroup->checkedButton()){
            buttons.at(i)->setChecked(false);
        }
    }
    if (m_obstaclegroup->checkedButton() != 0)
        m_obstaclegroup->checkedButton()->setChecked(false);
    setMode(Mode(mode));
    notify(mode);
    notifyScene();
    //m_scene->setSceneFactory(m_receiverFactory);
}

void ApplicationWindow::obstacleGroupClicked(int mode){
    QList <QAbstractButton*> buttons = m_obstaclegroup->buttons();
    for (int i = 0; i < buttons.size(); i++){
        if (buttons.at(i) != m_obstaclegroup->checkedButton()){
            buttons.at(i)->setChecked(false);
        }
    }
    if (m_antennagroup->checkedButton() != 0)
        m_antennagroup->checkedButton()->setChecked(false);
    setMode(Mode(mode));
    notify(mode);
    notifyScene();
}

void ApplicationWindow::rayTracingGroupClicked(int mode){
    cout<<"Button clicked"<<endl;
    QList <QAbstractButton*> buttons = m_raytracinggroup->buttons();
    for (int i = 0; i < buttons.size(); i++){
        if (buttons.at(i) != m_raytracinggroup->checkedButton()){
            buttons.at(i)->setChecked(false);
        }
    }
    //setMode(Mode(mode));
    //notify(mode);
    notifyModel();
    //notifyScene();

}

void ApplicationWindow::deleteProduct(){
    notify(int(DeleteItem));
}

void ApplicationWindow::openProduct(){
    notify((int(PropertiesItem)));
}
