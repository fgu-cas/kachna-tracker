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

#include "detector_color.h"


configWindow::configWindow(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::configWindow)
{
    ui->setupUi(this);

    connect(ui->maskXBox, SIGNAL(valueChanged(int)), this, SLOT(maskValueChanged()));
    connect(ui->maskYBox, SIGNAL(valueChanged(int)), this, SLOT(maskValueChanged()));
    connect(ui->maskRadiusBox, SIGNAL(valueChanged(int)), this, SLOT(maskValueChanged()));

    connect(ui->triggerBox, SIGNAL(valueChanged(int)), ui->triggerSlider, SLOT(setValue(int)));
    connect(ui->triggerSlider, SIGNAL(valueChanged(int)), ui->triggerBox, SLOT(setValue(int)));

    connect(ui->skipSpin, SIGNAL(valueChanged(int)), ui->skipSlider, SLOT(setValue(int)));
    connect(ui->skipSlider, SIGNAL(valueChanged(int)), ui->skipSpin, SLOT(setValue(int)));

    connect(ui->thresholdSpin, SIGNAL(valueChanged(int)), ui->thresholdSlider, SLOT(setValue(int)));
    connect(ui->thresholdSlider, SIGNAL(valueChanged(int)), ui->thresholdSpin, SLOT(setValue(int)));

    connect(ui->videoLabel, SIGNAL(posChangedX(int)), ui->maskXBox, SLOT(setValue(int)));
    connect(ui->videoLabel, SIGNAL(posChangedY(int)), ui->maskYBox, SLOT(setValue(int)));
    connect(ui->videoLabel, SIGNAL(radiusChanged(int)), ui->maskRadiusBox, SLOT(setValue(int)));

    connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTracking()));

    connect(ui->refreshDevicesButton, SIGNAL(clicked(bool)), this, SLOT(refreshDevices()));

    ui->ratFrontSelector->setTitle("Rat Front");
    ui->ratBackSelector->setTitle("Rat Back");
    ui->robotFrontSelector->setTitle("Robot Front");
    ui->robotBackSelector->setTitle("Robot Back");

    connect(ui->ratFrontSelector, SIGNAL(showStateChanged(bool)), this, SLOT(filteringStateChanged(bool)));
    connect(ui->ratBackSelector, SIGNAL(showStateChanged(bool)), this, SLOT(filteringStateChanged(bool)));
    connect(ui->robotFrontSelector, SIGNAL(showStateChanged(bool)), this, SLOT(filteringStateChanged(bool)));
    connect(ui->robotBackSelector, SIGNAL(showStateChanged(bool)), this, SLOT(filteringStateChanged(bool)));

    connect(ui->ratFrontSelector, SIGNAL(colorRangeChanged(colorRange)), this, SLOT(filterRangeChanged(colorRange)));
    connect(ui->ratBackSelector, SIGNAL(colorRangeChanged(colorRange)), this, SLOT(filterRangeChanged(colorRange)));
    connect(ui->robotFrontSelector, SIGNAL(colorRangeChanged(colorRange)), this, SLOT(filterRangeChanged(colorRange)));
    connect(ui->robotBackSelector, SIGNAL(colorRangeChanged(colorRange)), this, SLOT(filterRangeChanged(colorRange)));

    connect(ui->resolutionHeightSpin, SIGNAL(valueChanged(int)), this, SLOT(captureResolutionChanged()));
    connect(ui->resolutionWidthSpin, SIGNAL(valueChanged(int)), this, SLOT(captureResolutionChanged()));

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

    ui->resolutionBox->setChecked(settings.value("video/resolution/enabled").toBool());
    ui->resolutionWidthSpin->setValue(settings.value("video/resolution/width").toInt());
    ui->resolutionHeightSpin->setValue(settings.value("video/resolution/height").toInt());

    ui->maskXBox->setValue(settings.value("arena/X").toDouble());
    ui->maskYBox->setValue(settings.value("arena/Y").toDouble());
    ui->maskRadiusBox->setValue(settings.value("arena/radius").toInt());
    ui->arenaSizeBox->setValue(settings.value("arena/size").toDouble());

    //ui->multipleCombo->setCurrentIndex(settings.value("faults/multipleReaction").toInt());
    ui->skipCombo->setCurrentIndex(settings.value("faults/skipReaction").toInt());
    ui->skipSpin->setValue(settings.value("faults/skipDistance").toInt());
    ui->skipTimeoutBox->setValue(settings.value("faults/skipTimeout").toInt()/1000.0);

    ui->trackingCombobox->setCurrentIndex(settings.value("tracking/type").toInt());

    ui->thresholdSpin->setValue(settings.value("tracking/threshold/threshold").toInt());
    ui->ratMinSize->setValue(settings.value("tracking/threshold/minRat").toDouble());
    ui->ratMaxSize->setValue(settings.value("tracking/threshold/maxRat").toDouble());
    ui->robotMinSize->setValue(settings.value("tracking/threshold/minRobot").toDouble());
    ui->robotMaxSize->setValue(settings.value("tracking/threshold/maxRobot").toDouble());

    //Forgive me.
    colorRange ratFrontRange;
    ratFrontRange.hue = settings.value("tracking/color/ratFront/hue").toInt();
    ratFrontRange.hue_tolerance = settings.value("tracking/color/ratFront/hue_tolerance").toInt();
    ratFrontRange.saturation_low = settings.value("tracking/color/ratFront/saturation_low").toInt();
    ratFrontRange.value_low = settings.value("tracking/color/ratFront/value_low").toInt();
    ui->ratFrontSelector->setRange(ratFrontRange);

    colorRange ratBackRange;
    ratBackRange.hue = settings.value("tracking/color/ratBack/hue").toInt();
    ratBackRange.hue_tolerance = settings.value("tracking/color/ratBack/hue_tolerance").toInt();
    ratBackRange.saturation_low = settings.value("tracking/color/ratBack/saturation_low").toInt();
    ratBackRange.value_low = settings.value("tracking/color/ratBack/value_low").toInt();
    ui->ratBackSelector->setRange(ratBackRange);

    colorRange robotFrontRange;
    robotFrontRange.hue = settings.value("tracking/color/robotFront/hue").toInt();
    robotFrontRange.hue_tolerance = settings.value("tracking/color/robotFront/hue_tolerance").toInt();
    robotFrontRange.saturation_low = settings.value("tracking/color/robotFront/saturation_low").toInt();
    robotFrontRange.value_low = settings.value("tracking/color/robotFront/value_low").toInt();
    ui->robotFrontSelector->setRange(robotFrontRange);

    colorRange robotBackRange;
    robotBackRange.hue = settings.value("tracking/color/robotBack/hue").toInt();
    robotBackRange.hue_tolerance = settings.value("tracking/color/robotBack/hue_tolerance").toInt();
    robotBackRange.saturation_low = settings.value("tracking/color/robotBack/saturation_low").toInt();
    robotBackRange.value_low = settings.value("tracking/color/robotBack/value_low").toInt();
    ui->robotBackSelector->setRange(robotBackRange);

    ui->synchroBox->setChecked(settings.value("output/sync_enabled").toBool());
    ui->invertBox->setChecked(settings.value("output/sync_inverted").toBool());
    ui->shockBox->setChecked(settings.value("output/shock").toBool());

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

    settings.insert("video/resolution/enabled", ui->resolutionBox->isChecked());
    settings.insert("video/resolution/width", ui->resolutionWidthSpin->value());
    settings.insert("video/resolution/height", ui->resolutionHeightSpin->value());

    //settings.insert("faults/multipleReaction", ui->multipleCombo->currentIndex());
    settings.insert("faults/skipReaction", ui->skipCombo->currentIndex());
    settings.insert("faults/skipDistance", ui->skipSpin->value());
    settings.insert("faults/skipTimeout", (int) (ui->skipTimeoutBox->value()*1000));

    settings.insert("arena/X", ui->maskXBox->value());
    settings.insert("arena/Y", ui->maskYBox->value());
    settings.insert("arena/radius", ui->maskRadiusBox->value());
    settings.insert("arena/size", ui->arenaSizeBox->value());

    settings.insert("tracking/type", ui->trackingCombobox->currentIndex());

    settings.insert("tracking/threshold/threshold", ui->thresholdSpin->value());
    settings.insert("tracking/threshold/minRat", ui->ratMinSize->value());
    settings.insert("tracking/threshold/maxRat", ui->ratMaxSize->value());
    settings.insert("tracking/threshold/minRobot", ui->robotMinSize->value());
    settings.insert("tracking/threshold/maxRobot", ui->robotMaxSize->value());

    colorRange ratFrontRange = ui->ratFrontSelector->getColorRange();
    settings.insert("tracking/color/ratFront/hue", ratFrontRange.hue);
    settings.insert("tracking/color/ratFront/hue_tolerance", ratFrontRange.hue_tolerance);
    settings.insert("tracking/color/ratFront/value_low", ratFrontRange.value_low);
    settings.insert("tracking/color/ratFront/saturation_low", ratFrontRange.saturation_low);

    colorRange ratBackRange = ui->ratBackSelector->getColorRange();
    settings.insert("tracking/color/ratBack/hue", ratBackRange.hue);
    settings.insert("tracking/color/ratBack/hue_tolerance", ratBackRange.hue_tolerance);
    settings.insert("tracking/color/ratBack/value_low", ratBackRange.value_low);
    settings.insert("tracking/color/ratBack/saturation_low", ratBackRange.saturation_low);

    colorRange robotFrontRange = ui->robotFrontSelector->getColorRange();
    settings.insert("tracking/color/robotFront/hue", robotFrontRange.hue);
    settings.insert("tracking/color/robotFront/hue_tolerance", robotFrontRange.hue_tolerance);
    settings.insert("tracking/color/robotFront/value_low", robotFrontRange.value_low);
    settings.insert("tracking/color/robotFront/saturation_low", robotFrontRange.saturation_low);

    colorRange robotBackRange = ui->robotBackSelector->getColorRange();
    settings.insert("tracking/color/robotBack/hue", robotBackRange.hue);
    settings.insert("tracking/color/robotBack/hue_tolerance", robotBackRange.hue_tolerance);
    settings.insert("tracking/color/robotBack/value_low", robotBackRange.value_low);
    settings.insert("tracking/color/robotBack/saturation_low", robotBackRange.saturation_low);

    settings.insert("output/sync_enabled", ui->synchroBox->isChecked());
    settings.insert("output/sync_inverted", ui->invertBox->isChecked());
    settings.insert("output/shock", ui->shockBox->isChecked());

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
        cvtColor(frame, frame, CV_BGR2RGB);
        testFrame = QPixmap::fromImage(QImage((uchar*) frame.data,
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
    if (testFrame.height() > 0){
        QPoint center = QPoint(ui->maskXBox->value(), ui->maskYBox->value());

        QPixmap pixmap = QPixmap(testFrame);
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
   resetDetector();
   refreshTracking();
}

void configWindow::on_browseButton_clicked()
{
    QString directoryPath = QFileDialog::getExistingDirectory(this);
    if (!directoryPath.isEmpty()){
        ui->directoryEdit->setText(directoryPath);
    }
}

void configWindow::on_triggerBox_valueChanged(int dist_px)
{
   double diameter_real = ui->arenaSizeBox->value();
   double radius_px = ui->maskRadiusBox->value();

   double dist_real = diameter_real * (dist_px/(radius_px*2));

   QString result("%1 m");

   ui->triggerRealLabel->setText(result.arg(dist_real, 3, 'f', 3, '0'));
}

void configWindow::on_refreshCheckbox_stateChanged(int state)
{
    if (state == Qt::Checked){
        ui->trackingCombobox->setEnabled(false);
        ui->refreshTrackingButton->setEnabled(false);
        ui->thresholdSpin->setEnabled(false);
        ui->thresholdSlider->setEnabled(false);
        ui->ratMaxSize->setEnabled(false);
        ui->ratMinSize->setEnabled(false);
        ui->robotMaxSize->setEnabled(false);
        ui->robotMinSize->setEnabled(false);

        ui->ratFrontSelector->setEnabled(false);
        ui->ratBackSelector->setEnabled(false);
        ui->robotFrontSelector->setEnabled(false);
        ui->robotBackSelector->setEnabled(false);

        resetDetector();
        refreshTimer.start(ui->updateBox->value());
    } else {
        ui->trackingCombobox->setEnabled(true);
        ui->refreshTrackingButton->setEnabled(true);
        ui->thresholdSpin->setEnabled(true);
        ui->thresholdSlider->setEnabled(true);
        ui->ratMaxSize->setEnabled(true);
        ui->ratMinSize->setEnabled(true);
        ui->robotMaxSize->setEnabled(true);
        ui->robotMinSize->setEnabled(true);

        ui->ratFrontSelector->setEnabled(true);
        ui->ratBackSelector->setEnabled(true);
        ui->robotFrontSelector->setEnabled(true);
        ui->robotBackSelector->setEnabled(true);

        refreshTimer.stop();
    }
}

void configWindow::refreshTracking(){
   Mat frame;
   capture >> frame;
   if (!frame.empty()){
       trackingFrame = frame;
       updateTrackingView();
   }
}

void configWindow::updateTrackingView(){
    if (!trackingFrame.empty()){
        Mat frame;
        cv::cvtColor(trackingFrame, frame, CV_BGR2RGB);
        QPixmap pixmap;
        bool colorTracking = ui->trackingCombobox->currentIndex() == 1;

        if (!colorTracking && ui->thresholdEnableBox->isChecked()){
                frame = detector->process(&trackingFrame);
                pixmap = QPixmap::fromImage(QImage((uchar*) frame.data,
                                                            frame.cols,
                                                            frame.rows,
                                                            frame.step,
                                                            QImage::Format_Grayscale8));

        } else {
            if (colorTracking && colorFiltering){
                DetectorColor* detectorColor = dynamic_cast<DetectorColor*>(detector.get());
                frame = detectorColor->filter(&frame, filterRange);
            }
            pixmap = QPixmap::fromImage(QImage((uchar*) frame.data,
                                                        frame.cols,
                                                        frame.rows,
                                                        frame.step,
                                                        QImage::Format_RGB888));

        }

        QPainter painter(&pixmap);
        std::vector<KeyPoint> keypoints = detector->detect(&trackingFrame);
        ui->keypointList->clear();

        for (unsigned i = 0; i < keypoints.size(); i++){
            KeyPoint keypoint = keypoints[i];
            QString line = "%5%1: [%2, %3] (%4)";
            line = line.arg(i);
            line = line.arg(keypoint.pt.x);
            line = line.arg(keypoint.pt.y);
            line = line.arg(keypoint.size);
            if (colorTracking){
                switch (keypoint.class_id){
                case DetectorColor::RAT_FRONT:
                    line = line.arg("[RAT F] ");
                    break;
                case DetectorColor::RAT_BACK:
                    line = line.arg("[RAT B] ");
                    break;
                case DetectorColor::ROBOT_FRONT:
                    line = line.arg("[ROB F] ");
                    break;
                case DetectorColor::ROBOT_BACK:
                    line = line.arg("[ROB B] ");
                    break;
                default:
                    line = line.arg("[WHAT?] ");
                }
            } else {
                line = line.arg("");
            }
            ui->keypointList->addItem(line);

            if (colorTracking || (keypoint.size > ui->ratMinSize->value() &&
                    keypoint.size < ui->ratMaxSize->value())){

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

        if (colorTracking) detector->find(&trackingFrame);
        ui->trackingLabel->setPixmap(pixmap);
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

void configWindow::showEvent(QShowEvent *event){
    if (ui->deviceCombobox->currentIndex() != ui->deviceCombobox->count()-1){
        ui->lengthEdit->setEnabled(false);
        ui->timeoutStopBox->setEnabled(false);
        ui->resolutionBox->setEnabled(true);
        capture.open(ui->deviceCombobox->currentIndex());
    } else if (!videoFilename.isEmpty()) {
        ui->lengthEdit->setEnabled(false);
        ui->timeoutStopBox->setEnabled(false);
        ui->resolutionBox->setChecked(false);
        ui->resolutionBox->setEnabled(false);
        capture.open(videoFilename.toStdString());
    }

    if (ui->resolutionBox->isChecked()){
        capture.set(CV_CAP_PROP_FRAME_HEIGHT, ui->resolutionHeightSpin->value());
        capture.set(CV_CAP_PROP_FRAME_WIDTH, ui->resolutionWidthSpin->value());
    } else {
        ui->resolutionHeightSpin->setValue(capture.get(CV_CAP_PROP_FRAME_HEIGHT));
        ui->resolutionWidthSpin->setValue(capture.get(CV_CAP_PROP_FRAME_WIDTH));
    }

    if (event) event->accept();
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
        if (filename.isEmpty()){
            return;
        }
        videoFilename = filename;
        ui->deviceCombobox->removeItem(last);
        ui->deviceCombobox->addItem(QString("File... \"%1\"").arg(videoFilename));
        ui->deviceCombobox->setCurrentIndex(last);
    }
    showEvent(NULL);
}

void configWindow::filteringStateChanged(bool state){
    ColorSelector* sender = dynamic_cast<ColorSelector*>(QObject::sender());
    if (state){
        ui->ratFrontSelector->setShow(false);
        ui->ratBackSelector->setShow(false);
        ui->robotFrontSelector->setShow(false);
        ui->robotBackSelector->setShow(false);

        sender->setShow(true);
        filterRange = sender->getColorRange();
    }

    colorFiltering = state;
    updateTrackingView();
}

void configWindow::filterRangeChanged(colorRange range){
    filterRange = range;
    updateTrackingView();
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

void configWindow::on_maskButton_toggled(bool checked)
{
    ui->videoLabel->setEditable(checked);

    ui->maskXBox->setDisabled(checked);
    ui->maskYBox->setDisabled(checked);
    ui->maskRadiusBox->setDisabled(checked);
}

void configWindow::resetDetector(){
    if (trackingFrame.empty()){
        capture >> trackingFrame;
    }
    if (ui->trackingCombobox->currentIndex() == 0){
        detector.reset(new DetectorThreshold(compileSettings(), trackingFrame.rows, trackingFrame.cols));
    } else {
        detector.reset(new DetectorColor(compileSettings(), trackingFrame.rows, trackingFrame.cols));
    }
}

void configWindow::on_thresholdEnableBox_toggled(bool checked)
{
    (void)checked; // silence warning about unused parameter
    resetDetector();
    updateTrackingView();
}

void configWindow::on_thresholdSlider_valueChanged(int value)
{
    (void)value;
    if (ui->thresholdEnableBox->isChecked()){
        resetDetector();
        updateTrackingView();
    }
}

void configWindow::on_trackingCombobox_currentIndexChanged(int index)
{
    ui->trackingWidget->setCurrentIndex(index);
    resetDetector();
}

void configWindow::on_resolutionBox_toggled(bool checked)
{
    ui->resolutionHeightSpin->setEnabled(checked);
    ui->resolutionWidthSpin->setEnabled(checked);

    if (!checked){
        ui->resolutionHeightSpin->setValue(capture.get(CV_CAP_PROP_FRAME_HEIGHT));
        ui->resolutionWidthSpin->setValue(capture.get(CV_CAP_PROP_FRAME_WIDTH));
    }
}

void configWindow::captureResolutionChanged(){
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, ui->resolutionHeightSpin->value());
    capture.set(CV_CAP_PROP_FRAME_WIDTH, ui->resolutionWidthSpin->value());
}
