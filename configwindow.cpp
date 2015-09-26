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

    connect(ui->skipSpin, SIGNAL(valueChanged(int)), ui->skipSlider, SLOT(setValue(int)));
    connect(ui->skipSlider, SIGNAL(valueChanged(int)), ui->skipSpin, SLOT(setValue(int)));

    connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTracking()));

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

    int deviceIndex = settings.value("video/device").toInt();
    if (deviceIndex == -1){
        videoFilename = settings.value("video/filename").toString();
        refreshDevices();
        ui->deviceCombobox->setCurrentIndex(ui->deviceCombobox->count()-1);
    } else {
        refreshDevices();
        ui->deviceCombobox->setCurrentIndex(deviceIndex);
    }

    ui->maskXBox->setValue(settings.value("arena/X").toDouble());
    ui->maskYBox->setValue(settings.value("arena/Y").toDouble());
    ui->maskRadiusBox->setValue(settings.value("arena/radius").toInt());
    ui->arenaSizeBox->setValue(settings.value("arena/size").toDouble());

    ui->threshSpin->setValue(settings.value("tracking/threshold").toInt());

    //ui->multipleCombo->setCurrentIndex(settings.value("faults/multipleReaction").toInt());
    ui->skipCombo->setCurrentIndex(settings.value("faults/skipReaction").toInt());
    ui->skipSpin->setValue(settings.value("faults/skipDistance").toInt());
    ui->skipTimeoutBox->setValue(settings.value("faults/skipTimeout").toInt()/1000.0);

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


    Settings newsettings = compileSettings();
    if (lastSettings != newsettings){
        lastSettings = newsettings;
        emit(configurationUpdated(settings));
    }
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

    int deviceIndex = ui->deviceCombobox->currentIndex();
    if (deviceIndex == ui->deviceCombobox->count()-1){
        settings.insert("video/device", -1);
        settings.insert("video/filename", videoFilename);
    } else {
        settings.insert("video/device", deviceIndex);
    }

    settings.insert("tracking/threshold", ui->threshSpin->value());

    //settings.insert("faults/multipleReaction", ui->multipleCombo->currentIndex());
    settings.insert("faults/skipReaction", ui->skipCombo->currentIndex());
    settings.insert("faults/skipDistance", ui->skipSpin->value());
    settings.insert("faults/skipTimeout", (int) (ui->skipTimeoutBox->value()*1000));

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
    Mat frame;
    capture >> frame;
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
   capture >> frame;

   detector.reset(new Detector(compileSettings(), frame.rows, frame.cols));

   refreshTracking();
}

void configWindow::on_browseButton_clicked()
{
    QString directoryPath = QFileDialog::getExistingDirectory(this);
    if (!directoryPath.isEmpty()){
        ui->directoryEdit->setText(directoryPath);
    }
}

void configWindow::on_refreshCheckbox_stateChanged(int state)
{
    if (state == Qt::Checked){
        ui->refreshTrackingButton->setEnabled(false);
        ui->threshSpin->setEnabled(false);
        ui->ratMaxSize->setEnabled(false);
        ui->ratMinSize->setEnabled(false);
        ui->robotMaxSize->setEnabled(false);
        ui->robotMinSize->setEnabled(false);

        Mat frame;
        capture >> frame;

        detector.reset(new Detector(compileSettings(), frame.rows, frame.cols));
        refreshTimer.start(ui->updateBox->value());
    } else {
        ui->refreshTrackingButton->setEnabled(true);
        ui->threshSpin->setEnabled(true);
        ui->ratMaxSize->setEnabled(true);
        ui->ratMinSize->setEnabled(true);
        ui->robotMaxSize->setEnabled(true);
        ui->robotMinSize->setEnabled(true);

        refreshTimer.stop();
    }
}

void configWindow::refreshTracking(){
   Mat frame;
   capture >> frame;
   std::vector<KeyPoint> keypoints = detector->detect(&frame);

   QPixmap pixmap = QPixmap::fromImage(QImage((uchar*) frame.data,
                                              frame.cols,
                                              frame.rows,
                                              frame.step,
                                              QImage::Format_RGB888));

   QPainter painter(&pixmap);
   ui->keypointList->clear();

   for (unsigned i = 0; i < keypoints.size(); i++){
       KeyPoint keypoint = keypoints[i];
       ui->keypointList->addItem(QString::number(i) + ": " + QString::number(keypoint.pt.x)
                                 + ", " + QString::number(keypoint.pt.y) + " (" +
                                 QString::number(keypoint.size) + ")");

       if (keypoint.size > ui->ratMinSize->value() &&
               keypoint.size < ui->ratMaxSize->value()){

            QPoint rat(keypoint.pt.x, keypoint.pt.y);
            int ratSize = (int)(keypoint.size+0.5);
            painter.setPen(Qt::red);
            painter.drawEllipse(rat, ratSize, ratSize);

       } else if (keypoint.size > ui->robotMinSize->value() &&
               keypoint.size < ui->robotMaxSize->value()){

            QPoint robot(keypoint.pt.x, keypoint.pt.y);
            int robotSize = (int)(keypoint.size+0.5);
            painter.setPen(Qt::blue);
            painter.drawEllipse(robot, robotSize, robotSize);

       }
   }

   painter.end();

   ui->trackingLabel->setPixmap(pixmap);
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

void configWindow::showEvent(QShowEvent *event){
    if (ui->deviceCombobox->currentIndex() != ui->deviceCombobox->count()-1 ||
            videoFilename.isEmpty()){
        on_deviceCombobox_activated(ui->deviceCombobox->currentIndex());
    } else {
        ui->lengthEdit->setEnabled(false);
        ui->timeoutStopBox->setEnabled(false);
        capture.open(videoFilename.toStdString());
    }
    event->accept();
}

void configWindow::closeEvent(QCloseEvent *event){
    if (ui->refreshCheckbox->isChecked()){
       ui->refreshCheckbox->setChecked(false);
    }
    if (compileSettings() == lastSettings){
        capture.release();
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
            capture.release();
            event->accept();
            break;
        case QMessageBox::Discard:
            on_revertButton_clicked();
            capture.release();
            event->accept();
            break;
        case QMessageBox::Cancel:
            setCurrentIndex(0);
            event->ignore();
            break;
    }
}

void configWindow::on_deviceCombobox_activated(int index){
    int last = ui->deviceCombobox->count()-1;
    if (index == last){
        QString filename = QFileDialog::getOpenFileName(this, "Open Video", QString(), "Videos (*.avi)");
        if (!filename.isEmpty()){
            videoFilename = filename;
            ui->deviceCombobox->removeItem(last);
            ui->deviceCombobox->addItem(QString("File... \"%1\"").arg(videoFilename));
            ui->deviceCombobox->setCurrentIndex(last);

            ui->lengthEdit->setEnabled(false);
            ui->timeoutStopBox->setEnabled(false);
            capture.open(videoFilename.toStdString());
        }
    } else {
        ui->timeoutStopBox->setEnabled(true);
        ui->lengthEdit->setEnabled(true);
        capture.open(index);
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

    if (videoFilename.isEmpty()){
        ui->deviceCombobox->addItem("File...");
    } else {
        ui->deviceCombobox->addItem(QString("File... \"%1\"").arg(videoFilename));
    }
}

void configWindow::on_skipCombo_currentIndexChanged(int index)
{
    switch(index){
        case 0:
            ui->skipSlider->setEnabled(false);
            ui->skipSpin->setEnabled(false);
            ui->skipTimeoutBox->setEnabled(false);
            break;
        case 1:
            ui->skipSlider->setEnabled(true);
            ui->skipSpin->setEnabled(true);
            ui->skipTimeoutBox->setEnabled(true);
            break;
        case 2:
            ui->skipSlider->setEnabled(true);
            ui->skipSpin->setEnabled(true);
            ui->skipTimeoutBox->setEnabled(false);
            break;
    }
}
