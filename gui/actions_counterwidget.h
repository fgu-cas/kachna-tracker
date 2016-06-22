#ifndef COUNTERWIDGET_H
#define COUNTERWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>

class CounterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CounterWidget(QWidget *parent = 0);
signals:
    void idChanged(QString id);
    void deleteMe();
private:
    QLineEdit* id;
    QDoubleSpinBox* limit;
    QDoubleSpinBox* frequency;
    QCheckBox* enabled;
};

#endif // COUNTERWIDGET_H
