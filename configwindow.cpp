#include "configwindow.h"
#include "ui_configwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <initguid.h>
#include <windows.h>
#include <dshow.h>


configWindow::configWindow(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::configWindow)
{
    ui->setupUi(this);

    connect(ui->maskXBox, SIGNAL(valueChanged(double)), this, SLOT(maskValueChanged()));
    connect(ui->maskYBox, SIGNAL(valueChanged(double)), this, SLOT(maskValueChanged()));
    connect(ui->maskRadiusBox, SIGNAL(valueChanged(int)), this, SLOT(maskValueChanged()));

    connect(ui->triggerBox, SIGNAL(valueChanged(int)), ui->triggerSlider, SLOT(setValue(int)));
    connect(ui->triggerSlider, SIGNAL(valueChanged(int)), ui->triggerBox, SLOT(setValue(int)));

    connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(on_refreshTrackingButton_clicked()));

    connect(ui->refreshDevicesButton, SIGNAL(clicked(bool)), this, SLOT(refreshDevices()));
    refreshDevices();
}

configWindow::~configWindow()
{
    delete ui;
}

void configWindow::load(Settings settings)
{
    int length = settings.value("experiment/duration").toInt();
    int s = length%60;
    length /= 60;
    int m = length % 60;
    length /= 60;
    int h = length % 24;

    ui->lengthEdit->setTime(QTime(h, m, s));
    ui->timeoutStopBox->setChecked(settings.value("experiment/stopAfterTimeout").toBool());

    ui->deviceCombobox->setCurrentIndex(settings.value("video/device").toInt());

    ui->maskXBox->setValue(settings.value("arena/X").toDouble());
    ui->maskYBox->setValue(settings.value("arena/Y").toDouble());
    ui->maskRadiusBox->setValue(settings.value("arena/radius").toInt());
    ui->arenaSizeBox->setValue(settings.value("arena/size").toDouble());

    ui->threshSpin->setValue(settings.value("tracking/threshold").toInt());
    ui->maxAreaBox->setValue(settings.value("tracking/maxArea").toDouble());
    ui->minAreaBox->setValue(settings.value("tracking/minArea").toDouble());

    ui->ratMinSize->setValue(settings.value("tracking/minRat").toDouble());
    ui->ratMaxSize->setValue(settings.value("tracking/maxRat").toDouble());
    ui->robotMinSize->setValue(settings.value("tracking/minRobot").toDouble());
    ui->robotMaxSize->setValue(settings.value("tracking/maxRobot").toDouble());

    ui->entryBox->setValue(settings.value("shock/EntranceLatency").toInt());
    ui->interBox->setValue(settings.value("shock/InterShockLatency").toInt());
    ui->durationBox->setValue(settings.value("shock/ShockDuration").toInt());
    ui->refractoryBox->setValue(settings.value("shock/OutsideRefractory").toInt());
    ui->triggerBox->setValue(settings.value("shock/triggerDistance").toInt());

    ui->directoryEdit->setText(settings.value("system/defaultDirectory").toString());
    ui->filenameEdit->setText(settings.value("system/defaultFilename").toString());

    ui->updateBox->setValue(settings.value("system/updateInterval").toInt());

    lastSettings = compileSettings();
}

Settings configWindow::compileSettings()
{
    Settings settings;

    QTime time = ui->lengthEdit->time();
    settings.insert("experiment/duration", time.hour()*60*60+time.minute()*60+time.second());
    settings.insert("experiment/stopAfterTimeout", ui->timeoutStopBox->isChecked());

    settings.insert("system/defaultDirectory", ui->directoryEdit->text());
    settings.insert("system/defaultFilename", ui->filenameEdit->text());
    settings.insert("system/updateInterval", ui->updateBox->value());


    settings.insert("video/device", ui->deviceCombobox->currentIndex());

    settings.insert("tracking/threshold", ui->threshSpin->value());
    settings.insert("tracking/maxArea", ui->maxAreaBox->value());
    settings.insert("tracking/minArea", ui->minAreaBox->value());

    settings.insert("arena/X", ui->maskXBox->value());
    settings.insert("arena/Y", ui->maskYBox->value());
    settings.insert("arena/radius", ui->maskRadiusBox->value());
    settings.insert("arena/size", ui->arenaSizeBox->value());

    settings.insert("tracking/minRat", ui->ratMinSize->value());
    settings.insert("tracking/maxRat", ui->ratMaxSize->value());
    settings.insert("tracking/minRobot", ui->robotMinSize->value());
    settings.insert("tracking/maxRobot", ui->robotMaxSize->value());

    settings.insert("shock/EntranceLatency", ui->entryBox->value());
    settings.insert("shock/InterShockLatency", ui->interBox->value());
    settings.insert("shock/ShockDuration", ui->durationBox->value());
    settings.insert("shock/OutsideRefractory", ui->refractoryBox->value());
    settings.insert("shock/triggerDistance", ui->triggerBox->value());

    return settings;
}

void configWindow::on_testButton_clicked()
{
    capture.open(ui->deviceCombobox->currentIndex());

    Mat frame;
    int i = 0;
    do {
        capture >> frame;
        i++;
    } while (frame.empty() && i < 10);

    if (!frame.empty()){
        capturedFrame = QPixmap::fromImage(QImage((uchar*) frame.data,
                                                  frame.cols,
                                                  frame.rows,
                                                  frame.step,
                                                  QImage::Format_RGB888));
        maskValueChanged();
    } else {
        QMessageBox result;
        result.setText("Error! Couldn't retrieve frame.");
        result.exec();
    }

    capture.release();
}

void configWindow::maskValueChanged(){
    if (capturedFrame.height() > 0){
        QPoint center = QPoint(ui->maskXBox->value(), ui->maskYBox->value());

        QPixmap pixmap = QPixmap(capturedFrame);
        QPainter painter(&pixmap);
        painter.setPen(Qt::magenta);
        painter.drawLine(QPoint(center.x()-5, center.y()-5), QPoint(center.x()+5, center.y()+5));
        painter.drawLine(QPoint(center.x()-5, center.y()+5), QPoint(center.x()+5, center.y()-5));
        painter.drawEllipse(center, (int) ui->maskRadiusBox->value(), (int) ui->maskRadiusBox->value());
        painter.end();

        ui->videoLabel->setPixmap(pixmap);
        resize(minimumSizeHint());
    }
}

void configWindow::on_refreshTrackingButton_clicked()
{
    Mat frame;
    if (capture.isOpened()){
        capture >> frame;
    } else {
        capture.open(ui->deviceCombobox->currentIndex());
        int i = 0;
        do {
            capture >> frame;
            i++;
        } while (frame.empty() && i < 10);
        capture.release();
    }

   Detector detector(compileSettings(), frame.rows, frame.cols);

   Detector::keyPoints keypoints = detector.detect(&frame);

    QPoint rat(keypoints.rat.pt.x, keypoints.rat.pt.y);
    QPoint robot(keypoints.robot.pt.x, keypoints.robot.pt.y);

    std::vector<KeyPoint> detectAll = detector.detectAll(&frame);
    ui->keypointList->clear();
    for (unsigned i = 0; i < detectAll.size(); i++){
        KeyPoint keypoint = detectAll[i];
        ui->keypointList->addItem(QString::number(i) + ": " + QString::number(keypoint.pt.x)
                                  + ", " + QString::number(keypoint.pt.y) + " (" +
                                  QString::number(keypoint.size) + ")");
    }


    QPixmap pixmap = QPixmap::fromImage(QImage((uchar*) frame.data,
                                               frame.cols,
                                               frame.rows,
                                               frame.step,
                                               QImage::Format_RGB888));

    QPainter painter(&pixmap);

    int ratSize = (int)(keypoints.rat.size+0.5);
    painter.setPen(Qt::red);
    painter.drawEllipse(rat, ratSize, ratSize);

    int robotSize = (int)(keypoints.robot.size+0.5);
    painter.setPen(Qt::blue);
    painter.drawEllipse(robot, robotSize, robotSize);

    painter.end();

    ui->trackingLabel->setPixmap(pixmap);
}

void configWindow::on_browseButton_clicked()
{
    QString directoryPath = QFileDialog::getExistingDirectory(this);
    if (!directoryPath.isEmpty()){
        ui->directoryEdit->setText(directoryPath);
    }
}

void configWindow::on_checkBox_stateChanged(int state)
{
    if (state == Qt::Checked){
        ui->refreshTrackingButton->setDisabled(true);
        capture.open(ui->deviceCombobox->currentIndex());
        refreshTimer.start(100);
    } else {
        ui->refreshTrackingButton->setDisabled(false);
        refreshTimer.stop();
        capture.release();
    }
}

void configWindow::on_revertButton_clicked()
{
    load(lastSettings);
}

void configWindow::on_applyButton_clicked()
{
    Settings settings = compileSettings();
    lastSettings = settings;
    emit(configurationUpdated(settings));
}

void configWindow::on_okayButton_clicked()
{
    on_applyButton_clicked();
    close();
}

void configWindow::closeEvent(QCloseEvent *event){
    if (compileSettings() == lastSettings){
        event->accept();
        return;
    }
    QMessageBox reallyDialog;
    reallyDialog.setIcon(QMessageBox::Warning);
    reallyDialog.setModal(true);
    reallyDialog.setText("Wait! You've got unsaved changes in experiment configuration.");
    reallyDialog.setInformativeText("Do you want to save the changes?");
    reallyDialog.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
    switch (reallyDialog.exec()){
        case QMessageBox::Save:
            on_applyButton_clicked();
            event->accept();
            break;
        case QMessageBox::Discard:
            on_revertButton_clicked();
            event->accept();
            break;
        case QMessageBox::Cancel:
            setCurrentIndex(0);
            event->ignore();
            break;
    }
}

void configWindow::refreshDevices(){
    ui->deviceCombobox->clear();

    // Here be dragons. If it weren't for Naveen @ stackoverflow I'd be completely lost.
    // https://stackoverflow.com/questions/4286223/how-to-get-a-list-of-video-capture-devices-web-cameras-on-windows-c

//    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    IEnumMoniker *pEnum;
    ICreateDevEnum *pDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
            CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
    if (SUCCEEDED(hr)){
        hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
        if (hr == S_FALSE)
        {
            hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
        }
        pDevEnum->Release();
    }

    if (SUCCEEDED(hr)){
        IMoniker *pMoniker = NULL;
        while (pEnum->Next(1, &pMoniker, NULL) == S_OK){
            IPropertyBag *pPropBag;
            HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
            if (FAILED(hr))
            {
                pMoniker->Release();
                continue;
            }

            VARIANT var;
            VariantInit(&var);

            hr = pPropBag->Read(L"Description", &var, 0);
            if (FAILED(hr))
            {
                hr = pPropBag->Read(L"FriendlyName", &var, 0);
            }
            if (SUCCEEDED(hr))
            {
                wchar_t *name = var.bstrVal;
                ui->deviceCombobox->addItem(QString::fromWCharArray(name));
                VariantClear(&var);
            }

            pPropBag->Release();
            pMoniker->Release();
        }
        pEnum->Release();
        //CoUninitialize();
    }
}
