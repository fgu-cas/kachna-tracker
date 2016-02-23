#ifndef SHOCKSPINBOX_H
#define SHOCKSPINBOX_H

#include <QDoubleSpinBox>

class ShockSpinbox : public QDoubleSpinBox
{
public:
     ShockSpinbox(QWidget *parent): QDoubleSpinBox(parent){}
protected:
    QString textFromValue (double value) const;
    double valueFromText(const QString &text) const;
    QValidator::State validate(QString &input, int &pos) const;
};

#endif // SHOCKSPINBOX_H
