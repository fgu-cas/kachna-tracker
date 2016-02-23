#include "pointdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>

PointDialog::PointDialog(QWidget *parent) : QDialog(parent)
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

    /*
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
    */

    sizeCheckbox = new QCheckBox("Specific size limits", this);
    connect(sizeCheckbox, SIGNAL(toggled(bool)), this, SLOT(limitsToggled(bool)));

    minSpinbox = new QSpinBox();
    minSpinbox->setRange(0,255);
    minSpinbox->setEnabled(false);
    connect(minSpinbox, SIGNAL(valueChanged(int)), this, SLOT(onControlsChanged()));

    maxSpinbox = new QSpinBox();
    maxSpinbox->setRange(0,255);
    maxSpinbox->setEnabled(false);
    connect(maxSpinbox, SIGNAL(valueChanged(int)), this, SLOT(onControlsChanged()));

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Hue value", hueLayout);
    formLayout->addWidget(sizeCheckbox);
    formLayout->addRow("Minimum size", minSpinbox);
    formLayout->addRow("Maximum size", maxSpinbox);

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

pointRange PointDialog::getRange(){
    pointRange result;
    result.hue = hueSlider->value();
    if (sizeCheckbox->isChecked()){
        result.minimum_size = minSpinbox->value();
        result.maximum_size = maxSpinbox->value();
    } else {
        result.minimum_size = -1;
        result.maximum_size = -1;
    }
    return result;
}

void PointDialog::setRange(pointRange range){
    hueSlider->setValue(range.hue);
    minSpinbox->setValue(range.minimum_size);
    maxSpinbox->setValue(range.maximum_size);
}

void PointDialog::setTitle(QString title){
    setWindowTitle(QString("%1 - Selection").arg(title));
}

void PointDialog::onControlsChanged(){
    emit rangeChanged(getRange());
}

void PointDialog::onRevertButtonPressed(){

}

void PointDialog::limitsToggled(bool state){
    minSpinbox->setEnabled(state);
    maxSpinbox->setEnabled(state);
}
