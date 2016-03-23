#ifndef COLORSELECTOR_H
#define COLORSELECTOR_H
#include "detector_color.h"

#include <QDataStream>
#include <QWidget>
#include <QLabel>
#include <QPushButton>

class PointDialog;

class PointSelector: public QWidget
{
    Q_OBJECT
public:
    explicit PointSelector(QWidget *parent = 0);

    void setTitle(QString title);
    pointRange getColorRange();

signals:
    void showStateChanged(bool state);
    void colorRangeChanged(pointRange range);

public slots:
    void onSetButtonPressed();
    void rangeChanged(pointRange range);
    void setRange(pointRange range);
    void setEnabled(bool state);

private:
    QLabel* title;
    QLabel* colorLabel;
    QLabel* hsvLabel;
    QPushButton* setButton;

    PointDialog* colorDialog;

    pointRange range;


};

#include "pointdialog.h"
#endif // COLORSELECTOR_H
