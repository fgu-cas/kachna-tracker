#include "pointselector.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColor>
#include <QPixmap>

#include <QDebug>



PointSelector::PointSelector(QWidget *parent) : QWidget(parent)
{
    title = new QLabel();
    title->setTextFormat(Qt::RichText);
    colorLabel = new QLabel;
    hsvLabel = new QLabel();

    setButton = new QPushButton("Set");
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
    bot_layout->addWidget(setButton);

    glob_layout->addLayout(top_layout);
    glob_layout->addLayout(bot_layout);

    setLayout(glob_layout);

    colorDialog = new PointDialog(this);
    connect(colorDialog, SIGNAL(rangeChanged(pointRange)), this, SLOT(rangeChanged(pointRange)));

    rangeChanged(range);
}

void PointSelector::setTitle(QString title){
   this->title->setText(QString("<b>%1</b>").arg(title));
   colorDialog->setTitle(title);
}

pointRange PointSelector::getColorRange(){
    return range;
}

void PointSelector::rangeChanged(pointRange range){
    QPixmap range_p(40, 20);
    QColor range_c;
    QString range_s("Hue: %1°, Size: %3-%4");


    range_c.setHsv(range.hue, 255, 255);
    range_p.fill(range_c);

    range_s = range_s.arg(range.hue, 3, 10, QChar('0'));
    if (range.minimum_size == -1){
        range_s = range_s.arg("-");
        range_s = range_s.arg("-");
    } else {
        range_s = range_s.arg(range.minimum_size);
        range_s = range_s.arg(range.maximum_size);
    }

    hsvLabel->setText(range_s);
    colorLabel->setPixmap(range_p);

    this->range = range;
    emit colorRangeChanged(range);
}

void PointSelector::onSetButtonPressed(){
   colorDialog->show();
   colorDialog->raise();
   colorDialog->activateWindow();
}

void PointSelector::setRange(pointRange range){
    rangeChanged(range);
    colorDialog->setRange(range);
}

void PointSelector::setEnabled(bool state){
    setButton->setEnabled(state);
}

void PointSelector::closeDialog(){
    colorDialog->hide();
}
