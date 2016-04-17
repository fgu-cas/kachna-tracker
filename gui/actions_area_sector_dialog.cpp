#include "actions_area_sector_dialog.h"
#include "ui_actions_area_sector_dialog.h"

#include <math.h>

AreaSectorDialog::AreaSectorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AreaSectorDialog)
{
    ui->setupUi(this);
}

AreaSectorDialog::~AreaSectorDialog()
{
    delete ui;

    setWindowTitle("Area selection"); //TODO
}

void AreaSectorDialog::on_angleSlider_sliderMoved(int position)
{
    ui->angleSpinbox->setValue(position);
    ui->shockLabel->setAngle(position);
}

void AreaSectorDialog::on_angleSpinbox_valueChanged(double arg1)
{
    ui->angleSlider->setValue(round(arg1));
    ui->shockLabel->setAngle(arg1);
}

void AreaSectorDialog::on_rangeSlider_sliderMoved(int position)
{
    ui->rangeSpinbox->setValue(position);
    ui->shockLabel->setRange(position);
}


void AreaSectorDialog::on_rangeSpinbox_valueChanged(double arg1)
{
    ui->rangeSlider->setValue(round(arg1));
    ui->shockLabel->setRange(arg1);
}
