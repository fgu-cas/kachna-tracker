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
}

void DebugWindow::showEvent(QShowEvent *ev){
    on_portsComboBox_activated(ui->portsComboBox->currentText());
}

void DebugWindow::closeEvent(QCloseEvent *ev){
    arenomat.reset();
}

DebugWindow::~DebugWindow()
{
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
    if (arenomat != 0) arenomat->setLight(true);
}

void DebugWindow::on_ledOffButton_clicked()
{
    if (arenomat != 0) arenomat->setLight(false);
}

void DebugWindow::on_shockButton_clicked()
{
    if (arenomat != 0) arenomat->setShock(ui->shockSpinbox->value());
}
/*
void DebugWindow::on_lengthButton_clicked()
{
    if (arenomat != 0) arenomat->setShockLength(ui->lengthSpinbox->value());
}

void DebugWindow::on_delayButton_clicked()
{
    if (arenomat != 0) arenomat->setShockDelay(ui->delaySpinbox->value());
}
*/

void DebugWindow::on_directionCombobox_currentIndexChanged(int index)
{
    if (arenomat != 0) arenomat->setTurntableDirection(index);
}

void DebugWindow::on_rateButton_clicked()
{
    if (arenomat != 0) arenomat->setTurntableSpeed(ui->rateSpinbox->value());
}

void DebugWindow::on_setPButton_clicked()
{
    if (arenomat != 0) arenomat->setPIDValue(0, ui->pSpinbox->value());
}

void DebugWindow::on_setIButton_clicked()
{
    if (arenomat != 0) arenomat->setPIDValue(1, ui->iSpinbox->value());
}

void DebugWindow::on_setDButton_clicked()
{
    if (arenomat != 0) arenomat->setPIDValue(2, ui->dSpinbox->value());
}

void DebugWindow::on_pwmButton_clicked()
{
    if (arenomat != 0) arenomat->setTurntablePWM(ui->pwmSpinbox->value());
}

void DebugWindow::on_feederButton_clicked()
{
    if (arenomat != 0) arenomat->feed();
}
