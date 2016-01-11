#include "colorselector.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColor>
#include <QPixmap>



ColorSelector::ColorSelector(QWidget *parent) : QWidget(parent)
{
    title = new QLabel();
    title->setTextFormat(Qt::RichText);
    colorLabel = new QLabel;
    hsvLabel = new QLabel();

    showButton = new QPushButton("Show");
    showButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    showButton->setMaximumWidth(40);
    showButton->setCheckable(true);
    connect(showButton, SIGNAL(clicked(bool)), this, SLOT(onShowButtonPressed(bool)));
    QPushButton *setButton = new QPushButton("Set");
    setButton->setMaximumWidth(40);
    connect(setButton, SIGNAL(pressed()), this, SLOT(onSetButtonPressed()));


    QVBoxLayout *glob_layout = new QVBoxLayout;
    QHBoxLayout *top_layout = new QHBoxLayout;
    QHBoxLayout *bot_layout = new QHBoxLayout;

    glob_layout->setContentsMargins(QMargins(0, 0, 0, 0));

    top_layout->addWidget(title);
    top_layout->addStretch();
    top_layout->addWidget(colorLabel);

    bot_layout->addWidget(hsvLabel);
    bot_layout->addStretch();
    bot_layout->addWidget(showButton);
    bot_layout->addWidget(setButton);

    glob_layout->addLayout(top_layout);
    glob_layout->addLayout(bot_layout);

    setLayout(glob_layout);

    colorDialog = new ColorDialog(this);
    connect(colorDialog, SIGNAL(rangeChanged(colorRange)), this, SLOT(rangeChanged(colorRange)));

    rangeChanged(range);
}

void ColorSelector::setTitle(QString title){
   this->title->setText(QString("<b>%1</b>").arg(title));
   colorDialog->setTitle(title);
}

colorRange ColorSelector::getColorRange(){
    return range;
}

void ColorSelector::rangeChanged(colorRange range){
    QPixmap range_p(40, 20);
    QColor range_c;
    QString range_s("H: %1° (+/- %2°), S: %3->, V: %4->");


    range_c.setHsv(range.hue, 255, 255);
    range_p.fill(range_c);

    range_s = range_s.arg(range.hue, 3, 10, QChar('0'));
    range_s = range_s.arg(range.hue_tolerance, 3, 10, QChar('0'));
    range_s = range_s.arg(range.saturation_low, 3, 10, QChar('0'));
    range_s = range_s.arg(range.value_low, 3, 10, QChar('0'));

    hsvLabel->setText(range_s);
    colorLabel->setPixmap(range_p);

    this->range = range;
    emit colorRangeChanged(range);
}

void ColorSelector::onShowButtonPressed(bool state){
    emit showStateChanged(state);
}

void ColorSelector::onSetButtonPressed(){
   colorDialog->show();
   colorDialog->raise();
   colorDialog->activateWindow();
}

void ColorSelector::setRange(colorRange range){
    rangeChanged(range);
    colorDialog->setRange(range);
}

void ColorSelector::setShow(bool state){
    showButton->setChecked(state);
}
