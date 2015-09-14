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
    configWin.setWindowModality(Qt::ApplicationModal);
    connect(&configWin, SIGNAL(configurationUpdated(Settings)), this, SLOT(onConfigurationUpdated(Settings)));
    connect(ui->actionSave_tracks, SIGNAL(triggered()), this, SLOT(saveTracks()));

    appSettings.reset(new QSettings("FGU AV", "Kachna Tracker", this));

    QString fileName = appSettings->value("lastUsedSettings").toString();
    if (fileName.isEmpty()){
        loadSettings(QCoreApplication::applicationDirPath()+"experiment.ini");

        QMessageBox alert;
        alert.setText(tr("<b>Is this the first time you're running Kachna Tracker?</b>"));
        alert.setInformativeText(tr("No valid settings were found, please make sure to correctly set up"
                                    " the experiment parameters and export them before proceeding."));
        alert.setStandardButtons(QMessageBox::Ok);
        alert.exec();
        configWin.show();
    } else {
        loadSettings(fileName);
        dirty = false;
    }

    reset();

    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(requestUpdate()));
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateTick()));

    experiment = 0;
}

kachnatracker::~kachnatracker(){
    delete ui;
}

void kachnatracker::loadSettings(QString fileName){

    QSettings experimentIni(fileName, QSettings::IniFormat, this);
    Settings settings;

    QStringList keys = experimentIni.allKeys();
    for (int i = 0; i<keys.size();i++){
        QString key = keys.value(i);
        settings.insert(key, experimentIni.value(key));
    }

    configWin.load(settings);
    appSettings->setValue("lastUsedSettings", fileName);

    QString version = experimentIni.value("general/version").toString();
    if ( ! version.isEmpty() ){
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
}


void kachnatracker::on_actionConfigure_triggered(){
    configWin.show();
}

void kachnatracker::onConfigurationUpdated(Settings settings){
    currentSettings = settings;
    dirty = true;
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
        QList<QString> keys = currentSettings.keys();
        for (int i = 0; i < keys.size(); i++){
            QString key = keys[i];
            settings.setValue(key, currentSettings.value(key));
        }
        settings.setValue("general/version", QString("%1.%2")
                          .arg(MAJOR_VERSION)
                          .arg(MINOR_VERSION));
        appSettings->setValue("lastUsedSettings", fileName);
    }

    dirty = false;
}

void kachnatracker::experimentTimeout(){

    if (!experimentEnded){
        QMessageBox *alert = new QMessageBox(this);
        alert->setAttribute(Qt::WA_DeleteOnClose);
        alert->setStandardButtons(QMessageBox::Ok);
        alert->setText(tr("Experiment ended!"));
        if (currentSettings.value("experiment/stopAfterTimeout").toBool() || !isLive){
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
        QString fileName = QFileDialog::getSaveFileName(this, tr("Rat track"),
                                                        currentSettings.value("system/defaultDirectory").toString()+'/'+
                                                        currentSettings.value("system/defaultFilename").toString()+"_rat",
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
                                                    currentSettings.value("system/defaultDirectory").toString()+'/'+
                                                    currentSettings.value("system/defaultFilename").toString()+"_rob",
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
        VideoCapture capture;
        int deviceIndex = currentSettings.value("video/device").toInt();
        if (deviceIndex == -1){
            capture.open(currentSettings.value("video/filename").toString().toStdString());
            isLive = false;
            ui->progressBar->setValue(100);
            ui->timeLabel->setText("---");
        } else {
            capture.open(deviceIndex);
            isLive = true;
        }
        appSettings->setValue("lastSize", QSize(capture.get(CV_CAP_PROP_FRAME_WIDTH),
                                                capture.get(CV_CAP_PROP_FRAME_HEIGHT)));
        pixmap = QPixmap(capture.get(CV_CAP_PROP_FRAME_WIDTH),
                         capture.get(CV_CAP_PROP_FRAME_HEIGHT));
        pixmap.fill(Qt::white);
        capture.release();

        QPainter painter(&pixmap);
        painter.setPen(Qt::black);
        painter.drawEllipse(QPoint(currentSettings.value("arena/X").toInt(), currentSettings.value("arena/Y").toInt()),
                            currentSettings.value("arena/radius").toInt(), currentSettings.value("arena/radius").toInt());
        painter.end();

        reset();

        experiment.reset(new Experiment(this, &currentSettings));
        experiment->start();

        connect(ui->shockBox, SIGNAL(valueChanged(double)), experiment.get(), SLOT(changeShock(double)));

        updateTimer.start(currentSettings.value("system/updateInterval").toInt());
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

        int radius = currentSettings.value("shock/triggerDistance").toInt();
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
    if (isLive){
        qint64 duration = currentSettings.value("experiment/duration").toInt()*1000;
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
}

void kachnatracker::closeEvent(QCloseEvent *closeEvent){
    if (experiment != 0 && experiment->isRunning()){
        QMessageBox reallyDialog;
        reallyDialog.setIcon(QMessageBox::Warning);
        reallyDialog.setText("Wait! An experiment is currently in progress!");
        reallyDialog.setInformativeText("Are you sure you want to quit the application?");
        reallyDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel );
        switch (reallyDialog.exec()){
            case QMessageBox::Yes:
                updateTimer.stop();
                experiment->stop();
                break;
            case QMessageBox::Cancel:
                closeEvent->ignore();
                return;
        }
    } else if (dirty){
        QMessageBox reallyDialog;
        reallyDialog.setIcon(QMessageBox::Warning);
        reallyDialog.setModal(true);
        reallyDialog.setText("Wait! You've altered the experiment settings but haven't exported them.");
        reallyDialog.setInformativeText("Are you sure you want to quit the application without saving?");
        reallyDialog.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
        switch (reallyDialog.exec()){
            case QMessageBox::Save:
                on_actionExportConfig_triggered();
                break;
            case QMessageBox::Cancel:
                closeEvent->ignore();
                return;
        }
    }

    configWin.close();
    closeEvent->accept();
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

void kachnatracker::on_actionAbout_triggered()
{
    QMessageBox aboutBox;
    aboutBox.setText("<b>Kachna Tracker</b>");
    aboutBox.setInformativeText("Version 1.2-rc2 (2015/09/07)<br><br>https://www.github.com/tmladek/kachna-tracker");
    aboutBox.exec();
}
