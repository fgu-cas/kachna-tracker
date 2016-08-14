#include "configwindow.h"
#include "arenomat.h"
#include <QtSerialPort/QtSerialPort>
#include <QtSerialPort/QSerialPortInfo>

void configWindow::on_browseButton_clicked()
{
    QString directoryPath = QFileDialog::getExistingDirectory(this);
    if (!directoryPath.isEmpty()){
        ui->directoryEdit->setText(directoryPath);
    }
}

void configWindow::on_refreshPortButton_clicked()
{
    ui->portComboBox->clear();
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        ui->portComboBox->addItem(info.portName());
    }
}

void configWindow::on_portComboBox_activated(const QString &arg1)
{
    Arenomat arenomat(arg1);
    ui->portStatusLabel->setText("STATUS: CHECKING...");
    ui->portStatusLabel->repaint();
    if (arenomat.check()){
        ui->portStatusLabel->setText("STATUS: WORKING!");
    } else {
        ui->portStatusLabel->setText("STATUS: NOT WORKING");
    }
}

void configWindow::on_modeComboBox_activated(int index)
{
    bool visible = index == 0 ? false : true;
    ui->robotThreshOuter->setVisible(visible);
    ui->robotFrontSelector->setVisible(visible);
    if (ui->colorModeBox->currentIndex() == 1){
        ui->robotBackSelector->setVisible(visible);
    }
}
