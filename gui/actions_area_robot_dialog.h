#ifndef ACTIONS_AREA_ROBOT_WIDGET_H
#define ACTIONS_AREA_ROBOT_WIDGET_H

#include <QDialog>
#include "configwindow.h"

namespace Ui {
class AreaRobotWidget;
}

class AreaRobotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AreaRobotDialog(QWidget *parent = 0, configWindow *configWindow = 0);
    ~AreaRobotDialog();

public slots:
    void on_distanceBox_valueChanged(int px);
    void on_triggerBox_valueChanged(int px);

private:
    Ui::AreaRobotWidget *ui;

    configWindow* configWin;
};

#endif // ACTIONS_AREA_ROBOT_WIDGET_H
