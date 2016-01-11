#include "colordialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QSpinBox>

ColorDialog::ColorDialog(QWidget *parent) : QDialog(parent)
{
    QHBoxLayout *hueLayout = new QHBoxLayout();
    hueSlider = new QSlider(Qt::Horizontal);
    QSpinBox *hueSpinbox = new QSpinBox();
    hueSlider->setRange(0,360);
    hueSpinbox->setRange(0,360);
    hueSpinbox->setSuffix("°");
    connect(hueSlider, SIGNAL(valueChanged(int)), hueSpinbox, SLOT(setValue(int)));
    connect(hueSpinbox, SIGNAL(valueChanged(int)), hueSlider, SLOT(setValue(int)));
    connect(hueSlider, SIGNAL(valueChanged(int)), this, SLOT(onControlsChanged()));
    hueLayout->addWidget(hueSlider);
    hueLayout->addWidget(hueSpinbox);

    QHBoxLayout *hueTolLayout = new QHBoxLayout();
    hueTolSlider = new QSlider(Qt::Horizontal);
    QSpinBox *hueTolSpinbox = new QSpinBox();
    hueTolSlider->setRange(0,180);
    hueTolSpinbox->setRange(0,180);
    hueTolSpinbox->setPrefix("+/- ");
    hueTolSpinbox->setSuffix("°");
    connect(hueTolSlider, SIGNAL(valueChanged(int)), hueTolSpinbox, SLOT(setValue(int)));
    connect(hueTolSpinbox, SIGNAL(valueChanged(int)), hueTolSlider, SLOT(setValue(int)));
    connect(hueTolSlider, SIGNAL(valueChanged(int)), this, SLOT(onControlsChanged()));
    hueTolLayout->addWidget(hueTolSlider);
    hueTolLayout->addWidget(hueTolSpinbox);

    QHBoxLayout *satLayout = new QHBoxLayout();
    satSlider = new QSlider(Qt::Horizontal);
    QSpinBox *satSpinbox = new QSpinBox();
    satSlider->setRange(0,255);
    satSpinbox->setRange(0,255);
    satSpinbox->setSuffix("-255");
    connect(satSlider, SIGNAL(valueChanged(int)), satSpinbox, SLOT(setValue(int)));
    connect(satSpinbox, SIGNAL(valueChanged(int)), satSlider, SLOT(setValue(int)));
    connect(satSlider, SIGNAL(valueChanged(int)), this, SLOT(onControlsChanged()));
    satLayout->addWidget(satSlider);
    satLayout->addWidget(satSpinbox);

    QHBoxLayout *valLayout = new QHBoxLayout();
    valSlider = new QSlider(Qt::Horizontal);
    QSpinBox *valSpinbox = new QSpinBox();
    valSlider->setRange(0,255);
    valSpinbox->setRange(0,255);
    valSpinbox->setSuffix("-255");
    connect(valSlider, SIGNAL(valueChanged(int)), valSpinbox, SLOT(setValue(int)));
    connect(valSpinbox, SIGNAL(valueChanged(int)), valSlider, SLOT(setValue(int)));
    connect(valSlider, SIGNAL(valueChanged(int)), this, SLOT(onControlsChanged()));
    valLayout->addWidget(valSlider);
    valLayout->addWidget(valSpinbox);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Hue value", hueLayout);
    formLayout->addRow("Hue tolerance", hueTolLayout);
    formLayout->addRow("Saturation range", satLayout);
    formLayout->addRow("Value range", valLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *revertButton = new QPushButton("Revert");
    connect(revertButton, SIGNAL(clicked(bool)), this, SLOT(onRevertButtonPressed()));
    //buttonLayout->addWidget(revertButton);
    QPushButton *okayButton = new QPushButton("Okay");
    connect(okayButton, SIGNAL(clicked(bool)), this, SLOT(hide()));
    buttonLayout->addWidget(okayButton);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);

    setLayout(layout);
}

colorRange ColorDialog::getRange(){
    colorRange result;
    result.hue = hueSlider->value();
    result.hue_tolerance = hueTolSlider->value();
    result.saturation_low = satSlider->value();
    result.value_low = valSlider->value();
    return result;
}

void ColorDialog::setRange(colorRange range){
    hueSlider->setValue(range.hue);
    hueTolSlider->setValue(range.hue_tolerance );
    satSlider->setValue(range.saturation_low);
    valSlider->setValue(range.value_low);
}

void ColorDialog::setTitle(QString title){
    setWindowTitle(QString("%1 - Color Selection").arg(title));
}

void ColorDialog::onControlsChanged(){
    emit rangeChanged(getRange());
}

void ColorDialog::onRevertButtonPressed(){

}
