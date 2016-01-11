#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QSlider>

#include "colorselector.h"


class ColorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ColorDialog(QWidget *parent = 0);

    void setTitle(QString title);
    void setRange(colorRange range);

signals:
    void rangeChanged(colorRange range);

public slots:
    void onRevertButtonPressed();
    void onControlsChanged();

private:
    colorRange getRange();

    QSlider* hueSlider;
    QSlider* hueTolSlider;
    QSlider* satSlider;
    QSlider* valSlider;
};

#endif // COLORDIALOG_H
