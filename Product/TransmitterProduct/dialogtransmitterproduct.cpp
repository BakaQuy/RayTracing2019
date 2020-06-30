#include "dialogtransmitterproduct.h"

DialogTransmitterProduct::DialogTransmitterProduct(TransmitterProduct *mathematicalproduct):m_mathematicalproduct(mathematicalproduct)
{
    createDialog();
    setPosX(m_mathematicalproduct->getPosX());
    setPosY(m_mathematicalproduct->getPosY());
    setPower(m_mathematicalproduct->getPower());
    setFrequency(m_mathematicalproduct->getFrequency());
    setOrientation(m_mathematicalproduct->getOrientation());
    setKind(m_mathematicalproduct->getKind());
    setAttribute(Qt::WA_DeleteOnClose,true);
    exec();
}

DialogTransmitterProduct::~DialogTransmitterProduct(){

}

void DialogTransmitterProduct::createDialog(){
    setWindowTitle("Transmitter properties: ");
    setWindowIcon(QIcon(GraphicsTransmitterProduct::getImage()));
    QPushButton *save = new QPushButton("Save",this);
    QPushButton *cancel = new QPushButton("Cancel",this);
    QPushButton *plot = new QPushButton("Plot gain",this);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(plot);
    buttonLayout->addWidget(save);
    buttonLayout->addWidget(cancel);
    buttonLayout->setAlignment(Qt::AlignRight);

    m_modelBox = new QComboBox(this);
    m_modelBox->addItem("Half-wave dipole antenna");
    m_modelBox->addItem("Array dipole antenna");

    m_rowBox = new QSpinBox(this);
    m_columnBox = new QSpinBox(this);
    m_rowBox->setRange(1,8);
    m_columnBox->setRange(1,8);
    m_rowBox->setAccelerated(true);
    m_columnBox->setAccelerated(true);

    m_posx = new QSpinBox(this);
    m_posy = new QSpinBox(this);
    m_orientation = new QDoubleSpinBox(this);
    m_orientation->setRange(-360,360);
    m_posx->setRange(0,5000);
    m_posx->setAccelerated(true);
    m_posy->setRange(0,5000);
    m_posy->setAccelerated(true);

    m_frequencyValue = new QDoubleSpinBox(this);
    m_frequencyValue->setRange(0.00,999.00);
    m_frequencyValue->setAccelerated(true);

    m_frequencyorder = new QComboBox(this);
    m_frequencyorder->addItem("kHz");
    m_frequencyorder->addItem("MHz");
    m_frequencyorder->addItem("GHz");

    QHBoxLayout *frequency = new QHBoxLayout();
    frequency->addWidget(m_frequencyValue);

    frequency->addWidget(m_frequencyorder);

    m_power = new QDoubleSpinBox(this);
    m_power->setRange(0.0,100.0);
    m_power->setAccelerated(true);

    QFormLayout *modelProperties = new QFormLayout(this);
    modelProperties->addRow("Antenna model: ",m_modelBox);
    modelProperties->addRow("number of row antenna: ",m_rowBox);
    modelProperties->addRow("number of column antenna: ",m_columnBox);
    QString distance = "Distance relative to ";
    distance.append(QChar(0x03BB));

    QGroupBox *model = new QGroupBox("Model antenna");
    model->setLayout(modelProperties);

    QFormLayout *geoProperties = new QFormLayout(this);
    geoProperties->addRow("Orientation: ",m_orientation);
    geoProperties->addRow("X center: ",m_posx);
    geoProperties->addRow("Y center: ",m_posy);

    QGroupBox *geo = new QGroupBox("Geometry properties");
    geo->setLayout(geoProperties);

    QFormLayout *phyProperties = new QFormLayout(this);
    phyProperties->addRow("Frequency: ",frequency);
    phyProperties->addRow("Power: ",m_power);

    QGroupBox *phy = new QGroupBox("Physical properties");
    phy->setLayout(phyProperties);

    QGridLayout *firstLayout = new QGridLayout;
    firstLayout->addWidget(model,0,0);
    firstLayout->addWidget(geo,1,0);
    firstLayout->addWidget(phy,2,0);
    firstLayout->addLayout(buttonLayout,3,0);

    setLayout(firstLayout);

    connect(cancel,SIGNAL(clicked()),this,SLOT(close()));
    connect(save,SIGNAL(clicked()),this,SLOT(saveProperties()));
    connect(plot,SIGNAL(clicked()),this,SLOT(openPlot()));
    connect(m_modelBox,SIGNAL(activated(QString)),this,SLOT(changeModel(QString)));

}

unsigned long DialogTransmitterProduct::getFrequency(){
    QString text = m_frequencyorder->currentText();
    if (text == "kHz"){
        m_frequency = long(m_frequencyValue->value()*1e3);
    }
    else if (text == "MHz"){
        m_frequency = long(m_frequencyValue->value()*1e6);
    }
    else {
        m_frequency = long(m_frequencyValue->value()*1e9);
    }
    return m_frequency;
}

void DialogTransmitterProduct::setFrequency(unsigned long frequency){
    if (frequency/1e3 <= 999){
        m_frequencyValue->setValue(int(frequency/1e3));
        m_frequencyorder->setCurrentText("kHz");
    }
    else if (frequency/1e6 <= 999){
        m_frequencyValue->setValue(int(frequency/1e6));
        m_frequencyorder->setCurrentText("MHz");
    }
    else{
        m_frequencyValue->setValue(int(frequency/1e9));
        m_frequencyorder->setCurrentText("GHz");
    }
    m_frequency = frequency;
}

void  DialogTransmitterProduct::setKind(Kind kind) {
    m_kind = kind;
    if (kind == dipole){
        m_modelBox->setCurrentText("Half-wave dipole antenna");
        setRow(1);
        setColumn(1);
        m_rowBox->setEnabled(false);
        m_columnBox->setEnabled(false);
    }
    else if (kind == array) {
        m_modelBox->setCurrentText("Array dipole antenna");
        m_rowBox->setEnabled(true);
        m_columnBox->setEnabled(true);
    }
}

void DialogTransmitterProduct::newProperties(){
    m_mathematicalproduct->setRow(getRow());
    m_mathematicalproduct->setColumn(getColumn());
    m_mathematicalproduct->setKind(getKind());
    m_mathematicalproduct->setPosX(getPosX());
    m_mathematicalproduct->setPosY(getPosY());
    m_mathematicalproduct->setFrequency(getFrequency());
    m_mathematicalproduct->setPower(getPower());
    m_mathematicalproduct->setOrientation(getOrientation());
    m_mathematicalproduct->newProperties();
}

void DialogTransmitterProduct::saveProperties(){
    newProperties();
    close();
}

void DialogTransmitterProduct::openPlot(){
    getFrequency();
    m_row = getRow();
    m_column = getColumn();
    new PatternWindow(this);
}

void DialogTransmitterProduct::changeModel(QString model)
{
    if (model == "Half-wave dipole antenna")setKind(dipole);
    else if (model == "Array dipole antenna")setKind(array);
}
