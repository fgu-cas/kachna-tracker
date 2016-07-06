#ifndef ACTIONS_AREA_ROBOT_WIDGET_H
#define ACTIONS_AREA_ROBOT_WIDGET_H

#include <QDialog>
#include "configwindow.h"
#include "params.h"

namespace Ui {
class AreaRobotWidget;
}

class AreaRobotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AreaRobotDialog(QWidget *parent, configWindow *configWindow, int row, Area area);
    ~AreaRobotDialog();

signals:
    void update(int row, Area area);

public slots:
    void on_distanceBox_valueChanged(int px);
    void on_triggerBox_valueChanged(int px);

private:
    Ui::AreaRobotWidget *ui;
    int row;
    configWindow* configWin;

    void closeEvent(QCloseEvent*);

};

#endif // ACTIONS_AREA_ROBOT_WIDGET_H
