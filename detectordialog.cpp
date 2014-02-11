#include "detectordialog.h"
#include "ui_detectordialog.h"

DetectorDialog::DetectorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DetectorDialog)
{
    ui->setupUi(this);
    connect(ui->pushButton, SIGNAL(clicked()), parent, SLOT(updateSettings()));
}

DetectorDialog::~DetectorDialog()
{
    delete ui;
}

Settings DetectorDialog::getSettings()
{
    Settings result;
    result.alpha = ui->alphaBox->value();
    result.beta = ui->betaBox->value();
    result.blur = ui->blurBox->value();
    result.minSize = ui->minBox->value();
    result.maxSize = ui->maxBox->value();
    return result;
}
