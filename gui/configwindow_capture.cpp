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

void configWindow::on_deviceCombobox_activated(int index){
    int last = ui->deviceCombobox->count()-1;
    if (index == last){
        QString dir;
        if (!videoFilename.isEmpty()){
            QStringList list = videoFilename.split("/");
            list.removeLast();
            dir = list.join('/');
        }
        QString filename = QFileDialog::getOpenFileName(this, "Open Video", dir, "Videos (*.avi)");
        if (!filename.isEmpty()){
            videoFilename = filename;
            ui->deviceCombobox->removeItem(last);
            ui->deviceCombobox->addItem(QString("File... \"%1\"").arg(videoFilename));
            ui->deviceCombobox->setCurrentIndex(last);

            ui->lengthEdit->setEnabled(false);
            ui->timeoutStopBox->setEnabled(false);
            capture.open(videoFilename.toStdString());
        }
    }
    showEvent(NULL);
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

void configWindow::on_maskButton_toggled(bool checked)
{
    ui->videoLabel->setEditable(checked);

    ui->maskXBox->setDisabled(checked);
    ui->maskYBox->setDisabled(checked);
    ui->maskRadiusBox->setDisabled(checked);
}
