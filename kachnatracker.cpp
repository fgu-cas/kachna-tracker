#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "kachnatracker.h"
#include "ui_kachnatracker.h"
#include <iostream>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QTimer>
#include <QPainter>
#include <cmath>

using namespace cv;

kachnatracker::kachnatracker(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::kachnatracker)
{
    ui->setupUi(this);

    QPixmap black(500, 400);
    black.fill(Qt::black);
    ui->displayLabel->setPixmap(black);

    appSettings = new QSettings("FGU AV", "Kachna Tracker", this);
    QString fileName = appSettings->value("lastUsedSettings",
                                          QCoreApplication::applicationDirPath()+"experiment.ini").toString();
    QSettings experimentIni(fileName, QSettings::IniFormat, this);
    QMap<QString, QVariant> experimentSettings;

    QStringList keys = experimentIni.allKeys();
    for (int i = 0; i<keys.size();i++){
        QString key = keys.value(i);
        experimentSettings.insert(key, experimentIni.value(key));
    }

    configWin.setSettings(experimentSettings);

    connect(&experimentTimer, SIGNAL(timeout()), this, SLOT(updateTick()));
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(requestUpdate()));

    experiment = 0;
}

kachnatracker::~kachnatracker()
{
    delete ui;
}


void kachnatracker::on_actionConfigure_triggered()
{
    configWin.show();
}

void kachnatracker::on_actionImportConfig_triggered()
{


    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "",
                                                     tr("Files (*.ini)"));
    if (fileName.isEmpty()){
        return;
    }
    QMap<QString, QVariant> experimentSettings;
    QSettings experimentIni(fileName, QSettings::IniFormat);

    QStringList keys = experimentIni.allKeys();
    for (int i = 0; i<keys.size();i++){
        QString key = keys.value(i);
        experimentSettings.insert(key, experimentIni.value(key));
    }

    appSettings->setValue("lastUsedSettings", fileName);

    configWin.setSettings(experimentSettings);
}


void kachnatracker::on_actionExportConfig_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                     "",
                                                     tr("Files (*.ini)"));
    if (!fileName.isEmpty()){
        if (!fileName.endsWith(".ini")){
            fileName += ".ini";
        }
        QSettings settings(fileName, QSettings::IniFormat, this);
        QMap<QString, QVariant> experimentSettings = configWin.getSettings();
        QList<QString> keys = experimentSettings.keys();
        for (int i = 0; i < keys.size(); i++){
            QString key = keys[i];
            settings.setValue(key, experimentSettings.value(key));
        }
        appSettings->setValue("lastUsedSettings", fileName);
    }
}

void kachnatracker::experimentTimeout(){
    experimentTimer.stop();
    QMessageBox *alert = new QMessageBox(this);
    alert->setAttribute(Qt::WA_DeleteOnClose);
    alert->setStandardButtons(QMessageBox::Ok);
    alert->setText("Experiment ended!");
    if (configWin.getSettings().value("experiment/stopAfterTimeout").toBool()){
        experiment->stop();
        alert->exec();
        experimentEnded();
    } else {
        alert->setModal(false);
        alert->show();
    }
}

void kachnatracker::experimentEnded(){
    experimentTimer.stop();
    updateTimer.stop();

    QMap<QString, QVariant> settings = configWin.getSettings();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Rat track"),
                                                    settings.value("system/defaultDirectory").toString()+'/'+
                                                    settings.value("system/defaultFilename").toString(),
                                                    tr("Files (*.log)"));
    if (!fileName.isEmpty()){
        if (!fileName.endsWith(".log")){
            fileName += ".log";
        }
        QFile file(fileName);
        if (file.open(QFile::WriteOnly)){
            QString log = experiment->getLog(true);
            QTextStream out(&file);
            out << log;
        }
        file.close();
    }

    fileName = QFileDialog::getSaveFileName(this, tr("Robot track"),
                                                settings.value("system/defaultDirectory").toString()+'/'+
                                                settings.value("system/defaultFilename").toString(),
                                                tr("Files (*.log)"));
    if (!fileName.isEmpty()){
        if (!fileName.endsWith(".log")){
            fileName += ".log";
        }
        QFile file(fileName);
        if (file.open(QFile::WriteOnly)){
            QString log = experiment->getLog(false);
            QTextStream out(&file);
            out << log;
        }
        file.close();
    }


}


void kachnatracker::on_startButton_clicked()
{
    if (updateTimer.isActive()){
        experiment->stop();
//        experimentTimeout();
        experimentEnded();
    } else {
        QMap<QString, QVariant> settings = configWin.getSettings();

        VideoCapture capture = VideoCapture(settings.value("video/device").toInt());

        pixmap = QPixmap(capture.get(CV_CAP_PROP_FRAME_WIDTH),
                         capture.get(CV_CAP_PROP_FRAME_HEIGHT));
        capture.release();
        pixmap.fill(Qt::white);

        QPainter painter(&pixmap);
        painter.setPen(Qt::black);
        painter.drawEllipse(QPoint(settings.value("mask/X").toInt(), settings.value("mask/Y").toInt()),
                            settings.value("mask/H").toInt(), settings.value("mask/V").toInt());
        painter.end();
        reset();

        delete experiment;
        experiment = new Experiment(this, &settings);
        experiment->start();

        connect(ui->shockBox, SIGNAL(valueChanged(double)), experiment, SLOT(changeShock(double)));

        // Tick every hundredth of the experiment length -> interval=length/100, but the timer is in ms, so *1000 too
        experimentTimer.start(settings.value("experiment/duration", 15*60).toInt()*10);
        updateTimer.start(settings.value("system/updateInterval").toInt());
    }
}

void kachnatracker::requestUpdate(){
    Experiment::Update update = experiment->getUpdate();

    QPoint rat(update.keypoints.rat.pt.x, update.keypoints.rat.pt.y);
    QPoint lastRat;
    if (lastKeypoints.rat.size != 0){
        lastRat = QPoint(lastKeypoints.rat.pt.x, lastKeypoints.rat.pt.y);
    }
    QPoint robot(update.keypoints.robot.pt.x, update.keypoints.robot.pt.y);
    QPoint lastRobot;
    if (lastKeypoints.robot.size != 0){
        lastRobot = QPoint(lastKeypoints.robot.pt.x, lastKeypoints.robot.pt.y);
    }

    QPainter painter(&pixmap);

    if (rat.x() != 0 || rat.y() != 0){
        painter.setPen(Qt::red);
        painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));

        painter.drawEllipse(rat, 2, 2);
        if (lastRat.x() != 0){
            painter.drawLine(lastRat, rat);
        }

        lastKeypoints.rat = update.keypoints.rat;
    }

    if (robot.x() != 0 || robot.y() != 0){
        painter.setPen(Qt::blue);
        painter.setBrush(QBrush(Qt::blue, Qt::SolidPattern));

        painter.drawEllipse(robot, 2, 2);
        if (lastRobot.x() != 0){
            painter.drawLine(lastRobot, robot);
        }

        lastKeypoints.robot = update.keypoints.robot;
    }

    painter.end();
    ui->displayLabel->setPixmap(pixmap);

    ui->goodFramesLCD->display(update.stats.goodFrames);
    ui->badFramesLCD->display(update.stats.badFrames);
    ui->encounterLabel->setText(QString::number(update.stats.entryCount));
    ui->shockLabel->setText(QString::number(update.stats.shockCount));
    //todo: initial shock time display
}

void kachnatracker::updateTick(){
    ui->progressBar->setValue(ui->progressBar->value()+1);
    if (ui->progressBar->value() == 100){
        experimentTimeout();
    }
}

void kachnatracker::closeEvent(QCloseEvent *event){
    configWin.close();
    event->accept();
}

void kachnatracker::reset(){
    lastKeypoints.rat = KeyPoint(0, 0, 0);
    lastKeypoints.robot = KeyPoint(0, 0, 0);

    ui->progressBar->setValue(0);

    ui->badFramesLCD->display(0);
    ui->badFramesLCD->display(0);

    ui->shockBox->setValue(0.2);
}

void kachnatracker::on_actionDebug_triggered()
{

}
