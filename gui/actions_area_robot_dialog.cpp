#include "actions_area_robot_dialog.h"
#include "ui_actions_area_robot_dialog.h"

AreaRobotDialog::AreaRobotDialog(QWidget *parent, configWindow* configWindow) :
    QDialog(parent),
    ui(new Ui::AreaRobotWidget)
{
    ui->setupUi(this);

    configWin = configWindow;
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
