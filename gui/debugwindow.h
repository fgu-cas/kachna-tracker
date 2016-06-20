#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include <memory>
#include <QDialog>
#include <QByteArray>
#include "experiment/arenomat.h"

namespace Ui {
class DebugWindow;
}

class DebugWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DebugWindow(QWidget *parent = 0);
    ~DebugWindow();

private slots:
    void showMessage(QByteArray message);

    void on_portsComboBox_activated(const QString &portName);

    void on_checkButton_clicked();

    void on_ledOnButton_clicked();

    void on_ledOffButton_clicked();

    void on_shockButton_clicked();

    void on_lengthButton_clicked();

    void on_delayButton_clicked();

private:
    Ui::DebugWindow *ui;
    std::unique_ptr<Arenomat> arenomat;
};

#endif // DEBUGWINDOW_H
