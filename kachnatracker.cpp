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
    ui(new Ui::kachnatracker){

    ui->setupUi(this);
    connect(ui->actionSave_tracks, SIGNAL(triggered()), this, SLOT(saveTracks()));

    appSettings = new QSettings("FGU AV", "Kachna Tracker", this);

    QString fileName = appSettings->value("lastUsedSettings",
                                          QCoreApplication::applicationDirPath()+"experiment.ini").toString();
    loadSettings(fileName);

    reset();

    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(requestUpdate()));
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateTick()));

    experiment = 0;
}

kachnatracker::~kachnatracker(){

    delete appSettings;
    delete ui;
}

void kachnatracker::loadSettings(QString fileName){

    QSettings experimentIni(fileName, QSettings::IniFormat, this);
    QMap<QString, QVariant> experimentSettings;

    QStringList keys = experimentIni.allKeys();
    for (int i = 0; i<keys.size();i++){
        QString key = keys.value(i);
        experimentSettings.insert(key, experimentIni.value(key));
    }

    configWin.setSettings(experimentSettings);
    appSettings->setValue("lastUsedSettings", fileName);

    QString version = experimentIni.value("general/version").toString();
    int majorVersion = version.split(".")[0].toInt();
    if (majorVersion < MAJOR_VERSION){
        QMessageBox alert;
        alert.setText(tr("<b>Hold on! You're using an old settings file!</b>"));
        alert.setInformativeText(tr("Please make sure that all settings are correct before proceeding with the experiment, "
                                    "then save the settings afterwards to get rid of this warning."));
        alert.setStandardButtons(QMessageBox::Ok);
        alert.exec();
        configWin.show();
    }
}


void kachnatracker::on_actionConfigure_triggered(){

    configWin.show();
}

void kachnatracker::on_actionImportConfig_triggered(){

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "",
                                                     tr("Files (*.ini)"));
    if (fileName.isEmpty()){
        return;
    }

    loadSettings(fileName);
}


void kachnatracker::on_actionExportConfig_triggered(){

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
        settings.setValue("general/version", QString("%1.%2.%3")
                          .arg(MAJOR_VERSION)
                          .arg(MINOR_VERSION));
        appSettings->setValue("lastUsedSettings", fileName);
    }
}

void kachnatracker::experimentTimeout(){

    if (!experimentEnded){
        QMessageBox *alert = new QMessageBox(this);
        alert->setAttribute(Qt::WA_DeleteOnClose);
        alert->setStandardButtons(QMessageBox::Ok);
        alert->setText(tr("Experiment ended!"));
        if (configWin.getSettings().value("experiment/stopAfterTimeout").toBool()){
            updateTimer.stop();
            experiment->stop();
            alert->exec();
            saveTracks();
        } else {
            alert->setModal(false);
            alert->show();
        }
    }
}

void kachnatracker::saveTracks(){

    if (experiment != 0){
        QMap<QString, QVariant> settings = configWin.getSettings();

        QString fileName = QFileDialog::getSaveFileName(this, tr("Rat track"),
                                                        settings.value("system/defaultDirectory").toString()+'/'+
                                                        settings.value("system/defaultFilename").toString()+"_rat",
                                                        tr("Files (*.dat)"));
        if (!fileName.isEmpty()){
            if (!fileName.endsWith(".dat")){
                fileName += ".dat";
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
                                                    settings.value("system/defaultFilename").toString()+"_rob",
                                                    tr("Files (*.dat)"));
        if (!fileName.isEmpty()){
            if (!fileName.endsWith(".dat")){
                fileName += ".dat";
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
}


void kachnatracker::on_startButton_clicked(){

    if (updateTimer.isActive()){
        updateTimer.stop();
        experiment->stop();
        saveTracks();
    } else {
        experimentSettings = QMap<QString, QVariant>(configWin.getSettings());

        VideoCapture capture = VideoCapture(experimentSettings.value("video/device").toInt());
        appSettings->setValue("lastSize", QSize(capture.get(CV_CAP_PROP_FRAME_WIDTH),
                                                capture.get(CV_CAP_PROP_FRAME_HEIGHT)));
        capture.release();

        QPainter painter(&pixmap);
        painter.setPen(Qt::black);
        painter.drawEllipse(QPoint(experimentSettings.value("arena/X").toInt(), experimentSettings.value("arena/Y").toInt()),
                            experimentSettings.value("arena/radius").toInt(), experimentSettings.value("arena/radius").toInt());
        painter.end();

        reset();

        delete experiment;
        experiment = new Experiment(this, &experimentSettings);
        experiment->start();

        connect(ui->shockBox, SIGNAL(valueChanged(double)), experiment, SLOT(changeShock(double)));

        updateTimer.start(experimentSettings.value("system/updateInterval").toInt());
        elapsedTimer.start();
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

    if (robot.x() != 0 || robot.y() != 0){
        QPixmap tempPixmap(pixmap);
        QPainter painter(&tempPixmap);
        painter.setPen(Qt::yellow);
        painter.setBrush(QBrush(Qt::yellow, Qt::FDiagPattern));

        int radius = experimentSettings.value("shock/triggerDistance").toInt();
        painter.drawEllipse(robot, radius, radius);
        painter.end();
        ui->displayLabel->setPixmap(tempPixmap);
    } else {
        ui->displayLabel->setPixmap(pixmap);
    }

    ui->goodFramesLCD->display(update.stats.goodFrames);
    ui->badFramesLCD->display(update.stats.badFrames);
    ui->encounterLabel->setText(QString::number(update.stats.entryCount));
    ui->shockLabel->setText(QString::number(update.stats.shockCount));
    ui->firstShockLabel->setText(QString::number(update.stats.initialShock/1000)+" s");
}

void kachnatracker::updateTick(){

    qint64 duration = experimentSettings.value("experiment/duration").toInt()*1000;
    qint64 elapsed = elapsedTimer.elapsed();
    ui->progressBar->setValue((elapsed*1.0/duration)*100);

    QString sign = "-";
    int time = (duration-elapsed)/1000;
    if (time <= 0){
        experimentTimeout();
        experimentEnded = true;
        time *= -1;
        sign = "+";
    }
    int s = time%60;
    time /= 60;
    int m = time % 60;
    time /= 60;
    int h = time % 24;
    ui->timeLabel->setText(QString("%1%2:%3:%4").arg(sign)
                           .arg(h, 2, 10, QChar('0'))
                           .arg(m, 2, 10, QChar('0'))
                           .arg(s, 2, 10, QChar('0')));
}

void kachnatracker::closeEvent(QCloseEvent *event){

    configWin.close();
    event->accept();
}

void kachnatracker::reset(){

    experimentEnded = false;

    lastKeypoints.rat = KeyPoint(0, 0, 0);
    lastKeypoints.robot = KeyPoint(0, 0, 0);

    QPixmap blank(appSettings->value("lastSize").toSize());
    blank.fill(Qt::white);
    ui->displayLabel->setPixmap(blank);

    ui->progressBar->setValue(0);

    ui->badFramesLCD->display(0);
    ui->badFramesLCD->display(0);

    ui->shockBox->setValue(0.1);
}

void kachnatracker::on_actionSave_screenshot_triggered(){

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                     "",
                                                     tr("Images (*.png)"));
    if (!fileName.isEmpty()){
        if (!fileName.endsWith(".png")){
            fileName += ".png";
        }
        pixmap.save(fileName);
    }
}
