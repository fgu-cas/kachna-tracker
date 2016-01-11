#ifndef COLORSELECTOR_H
#define COLORSELECTOR_H
#include "detector_color.h"

#include <QDataStream>
#include <QWidget>
#include <QLabel>
#include <QPushButton>

class ColorDialog;

class ColorSelector: public QWidget
{
    Q_OBJECT
public:
    explicit ColorSelector(QWidget *parent = 0);

    void setTitle(QString title);
    colorRange getColorRange();

signals:
    void showStateChanged(bool state);
    void colorRangeChanged(colorRange range);

public slots:
    void onShowButtonPressed(bool);
    void onSetButtonPressed();
    void rangeChanged(colorRange range);
    void setRange(colorRange range);
    void setShow(bool state);

private:
    QLabel* title;
    QLabel* colorLabel;
    QLabel* hsvLabel;
    QPushButton* showButton;

    ColorDialog* colorDialog;

    colorRange range;


};

#include "colordialog.h"
#endif // COLORSELECTOR_H
