#include "shockspinbox.h"

#include <iostream>

QString ShockSpinbox::textFromValue(double value) const
{
    if (value < 0.2)
        return QString("0.0");
    else
        return QString::number(value);
}

double ShockSpinbox::valueFromText(const QString &text) const
{
    return QDoubleSpinBox::valueFromText(text);
}

QValidator::State ShockSpinbox::validate(QString &input, int &pos) const
{
    return QDoubleSpinBox::validate(input, pos);
}
