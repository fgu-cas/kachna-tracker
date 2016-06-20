#include "debugwindow.h"
#include "ui_debugwindow.h"
#include <QtSerialPort/QtSerialPort>
#include <QtSerialPort/QSerialPortInfo>

DebugWindow::DebugWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DebugWindow)
{
    ui->setupUi(this);

    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        ui->portsComboBox->addItem(info.portName());
    }
    on_portsComboBox_activated(ui->portsComboBox->currentText());
}

DebugWindow::~DebugWindow()
{
    arenomat.release();
    delete ui;
}

void DebugWindow::on_portsComboBox_activated(const QString &portName)
{
    arenomat.reset(new Arenomat(portName));
    connect(arenomat.get(), SIGNAL(messageReceived(QByteArray)), this, SLOT(showMessage(QByteArray)));
    ui->receivedLine->setText("");
    ui->statusLabel->setText("UNKNOWN");
}

void DebugWindow::showMessage(QByteArray message){
    ui->receivedLine->setText(QTime::currentTime().toString("[hh:mm:ss] ")
                              + QString(message));
}

void DebugWindow::on_checkButton_clicked()
{
    ui->statusLabel->setText("CHECKING...");
    ui->statusLabel->repaint();

    if (arenomat != 0 && arenomat->check()){
        ui->statusLabel->setText("WORKING");
    } else {
        ui->statusLabel->setText("NOT WORKING!");
    }
}

void DebugWindow::on_ledOnButton_clicked()
{
    if (arenomat != 0) arenomat->setLed(true);
}

void DebugWindow::on_ledOffButton_clicked()
{
    if (arenomat != 0) arenomat->setLed(false);
}

void DebugWindow::on_shockButton_clicked()
{
    if (arenomat != 0) arenomat->setShock(ui->shockSpinbox->value());
}

void DebugWindow::on_lengthButton_clicked()
{
    if (arenomat != 0) arenomat->setShockLength(ui->lengthSpinbox->value());
}

void DebugWindow::on_delayButton_clicked()
{
    if (arenomat != 0) arenomat->setShockDelay(ui->delaySpinbox->value());
}
