#ifndef DETECTORDIALOG_H
#define DETECTORDIALOG_H

#include <QDialog>
#include "util.cpp"


namespace Ui {
class DetectorDialog;
}

class DetectorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DetectorDialog(QWidget *parent = 0);
    Settings getSettings();
    ~DetectorDialog();

private:
    Ui::DetectorDialog *ui;
};

#endif // DETECTORDIALOG_H
