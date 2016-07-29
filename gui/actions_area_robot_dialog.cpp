#include "actions_area_robot_dialog.h"
#include "ui_actions_area_robot_dialog.h"
#include <QCloseEvent>

AreaRobotDialog::AreaRobotDialog(QWidget *parent, configWindow* configWindow, int row, Area area) :
    QDialog(parent),
    ui(new Ui::AreaRobotWidget)
{
    ui->setupUi(this);

    configWin = configWindow;
    this->row = row;

    ui->angleBox->setValue(area.angle);
    ui->distanceBox->setValue(area.distance);
    ui->triggerBox->setValue(area.radius);

    setWindowTitle("Area selection"); //TODO
}

AreaRobotDialog::~AreaRobotDialog()
{
    delete ui;
}

void AreaRobotDialog::on_distanceBox_valueChanged(int px)
{
    double dist_real = configWin->pixelsToMeters(px);
    QString result("%1 m");
    ui->distanceRealLabel->setText(result.arg(dist_real, 3, 'f', 3, '0'));
}


void AreaRobotDialog::on_triggerBox_valueChanged(int px)
{
    double dist_real = configWin->pixelsToMeters(px);
    QString result("%1 m");
    ui->triggerRealLabel->setText(result.arg(dist_real, 3, 'f', 3, '0'));
}


void AreaRobotDialog::closeEvent(QCloseEvent *event){
    Area area;
    area.angle = ui->angleBox->value();
    area.distance = ui->distanceBox->value();
    area.radius = ui->triggerBox->value();
    area.type = Area::CIRCULAR_AREA;
    emit(update(row, area));
    event->accept();
}
