void configWindow::on_browseButton_clicked()
{
    QString directoryPath = QFileDialog::getExistingDirectory(this);
    if (!directoryPath.isEmpty()){
        ui->directoryEdit->setText(directoryPath);
    }
}

void configWindow::on_distanceBox_valueChanged(int dist_px)
{
   double diameter_real = ui->arenaSizeBox->value();
   double radius_px = ui->maskRadiusBox->value();

   double dist_real = diameter_real * (dist_px/(radius_px*2));

   QString result("%1 m");

   ui->distanceRealLabel->setText(result.arg(dist_real, 3, 'f', 3, '0'));
}


void configWindow::on_triggerBox_valueChanged(int dist_px)
{
   double diameter_real = ui->arenaSizeBox->value();
   double radius_px = ui->maskRadiusBox->value();

   double dist_real = diameter_real * (dist_px/(radius_px*2));

   QString result("%1 m");

   ui->triggerRealLabel->setText(result.arg(dist_real, 3, 'f', 3, '0'));
}
