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
    // TODO: Error checking
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

void kachnatracker::experimentEnded(){
    experimentTimer.stop();
    updateTimer.stop();

    QMessageBox alert;
    alert.setText("Experiment ended!");
    alert.exec();
    //TODO: Experiment possibly continuing after time-out

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                     "",
                                                     tr("Files (*.log)"));
    if (!fileName.isEmpty()){
        if (!fileName.endsWith(".log")){
            fileName += ".log";
        }
        QFile file(fileName);
        if (file.open(QFile::WriteOnly)){
            QString log = experiment->getLog();
            QTextStream out(&file);
            out << log;
        }
        file.close();
    }

    delete experiment;
    experiment = 0;
}


void kachnatracker::on_startButton_clicked()
{
    if (experiment != 0){
        experimentTimer.stop();
        experiment->stop();
    } else {
        QMap<QString, QVariant> experimentSettings = configWin.getSettings();

        VideoCapture *capture;
        if (experimentSettings.value("deviceId").toInt() != 7){
            capture = new VideoCapture(experimentSettings.value("deviceId", 0).toInt());
        } else {
            capture = new VideoCapture("/tmp/video.avi");
        }

        pixmap = QPixmap(capture->get(CV_CAP_PROP_FRAME_WIDTH),
                         capture->get(CV_CAP_PROP_FRAME_HEIGHT));
        pixmap.fill(Qt::white);

        QPainter painter(&pixmap);
        painter.setPen(Qt::black);
        painter.drawEllipse(QPoint(experimentSettings.value("maskX").toInt(), experimentSettings.value("maskY").toInt()),
                            experimentSettings.value("maskH").toInt(), experimentSettings.value("maskV").toInt());
        painter.end();
        reset();

        experiment = new Experiment(this, &experimentSettings);        
        experiment->start();

        connect(ui->shockBox, SIGNAL(valueChanged(double)), experiment, SLOT(changeShock(double)));

        // Tick every hundredth of the experiment length -> interval=length/100, but the timer is in ms, so *1000 too
        experimentTimer.start(experimentSettings.value("experimentLength", 15*60).toInt()*10);
        updateTimer.start(100);
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

    ui->goodFramesLCD->display(update.goodFrames);
    ui->badFramesLCD->display(update.badFrames);
}

void kachnatracker::updateTick(){
    ui->progressBar->setValue(ui->progressBar->value()+1);
    if (ui->progressBar->value() == 100){
        experimentTimer.stop();
        experiment->stop();
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

    ui->shockBox->setValue(0.4);
}

void kachnatracker::on_actionDebug_triggered()
{

}
