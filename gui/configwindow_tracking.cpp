#include "configwindow.h"

void configWindow::on_refreshTrackingButton_clicked()
{
   resetDetector();
   trackFrame();
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

void configWindow::trackFrame(){
   Mat frame;
   capture >> frame;
   if (!frame.empty()){
       trackingFrame = frame;
       updateTrackingView();
   }
}

void configWindow::updateTrackingView(){
    if (!trackingFrame.empty()){
        Mat displayFrame, rgbFrame;
        QPixmap pixmap;

        cv::cvtColor(trackingFrame, rgbFrame, CV_BGR2RGB);

        bool colorTracking = ui->trackingCombobox->currentIndex() == 1;
        if (!colorTracking && ui->thresholdEnableBox->isChecked()){
            Mat processedFrame = detector->process(&trackingFrame);
            rgbFrame.copyTo(displayFrame, detector->analyze(&processedFrame));
        } else if (colorTracking && ui->colorthresholdBox->isChecked()){
            Mat processedFrame, tmpFrame;
            processedFrame = detector->process(&trackingFrame);
            processedFrame.copyTo(tmpFrame, detector->analyze(&processedFrame));
            cv::cvtColor(tmpFrame, displayFrame, CV_HSV2RGB);
        } else {
            displayFrame = rgbFrame;
        }

        pixmap = QPixmap::fromImage(QImage((uchar*) displayFrame.data,
                                                    displayFrame.cols,
                                                    displayFrame.rows,
                                                    displayFrame.step,
                                                    QImage::Format_RGB888));

        QPainter painter(&pixmap);
        std::vector<KeyPoint> keypoints = detector->detect(&trackingFrame);
        ui->keypointList->clear();

        for (unsigned i = 0; i < keypoints.size(); i++){
            KeyPoint keypoint = keypoints[i];
            QString line = "%5 %1: [%2, %3] (%4)";
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
                    line = line.arg("[?????] ");
                }
            } else {
                line = line.arg("");
            }
            ui->keypointList->addItem(line);

            if (colorTracking )
            if ((colorTracking && !ui->colorthresholdBox->isChecked()) ||
                (!colorTracking && !ui->thresholdEnableBox->isChecked())){

                if (colorTracking ||
                        (keypoint.size > ui->ratMinSize->value() && keypoint.size < ui->ratMaxSize->value())){

                     QPoint rat(keypoint.pt.x, keypoint.pt.y);
                     int ratSize = (int)(keypoint.size+0.5);
                     painter.setPen(Qt::red);
                     painter.drawEllipse(rat, ratSize, ratSize);

                } else if (keypoint.size > ui->robotMinSize->value() && keypoint.size < ui->robotMaxSize->value()){

                     QPoint robot(keypoint.pt.x, keypoint.pt.y);
                     int robotSize = (int)(keypoint.size+0.5);
                     painter.setPen(Qt::blue);
                     painter.drawEllipse(robot, robotSize, robotSize);

                }
            }
        }

        painter.end();

        ui->trackingLabel->setPixmap(pixmap);
    }
}

void configWindow::trackingParamsChanged(){
   bool colorTracking = ui->trackingCombobox->currentIndex() == 1;
   if ((!colorTracking && ui->thresholdEnableBox->isChecked()) ||
        (colorTracking && ui->colorthresholdBox->isChecked())){
       resetDetector();
       updateTrackingView();
   }
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

void configWindow::on_trackingCombobox_currentIndexChanged(int index)
{
    ui->trackingWidget->setCurrentIndex(index);
    resetDetector();
    updateTrackingView();

    if (index == 0){
        ui->angleBox->setEnabled(false);
        ui->angleSlider->setEnabled(false);
        ui->distanceSlider->setEnabled(false);
        ui->distanceBox->setEnabled(false);
        ui->shockLocationLabel->setCentered(true);
    } else {
        ui->angleBox->setEnabled(true);
        ui->angleSlider->setEnabled(true);
        ui->distanceSlider->setEnabled(true);
        ui->distanceBox->setEnabled(true);
        ui->shockLocationLabel->setCentered(false);
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
