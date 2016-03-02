#include "actions_counterwidget.h"

#include <QHBoxLayout>
#include <QPushButton>

#include <QDebug>

CounterWidget::CounterWidget(QWidget *parent) : QWidget(parent)
{
    id = new QLineEdit(this);
    connect(id, SIGNAL(textChanged(QString)), this, SIGNAL(idChanged(QString)));

    limit = new QDoubleSpinBox(this);
    limit->setMinimum(0);

    frequency = new QDoubleSpinBox(this);
    frequency->setSuffix("/s");

    enabled = new QCheckBox("Enabled", this);

    QPushButton *deleteButton = new QPushButton("X", this);
    deleteButton->setMaximumWidth(25);
    connect(deleteButton, SIGNAL(pressed()), this, SIGNAL(deleteMe()));


    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(QMargins(0, 0, 0, 0));

    layout->addWidget(id);
    layout->addWidget(limit);
    layout->addWidget(frequency);
    layout->addWidget(enabled);
    layout->addWidget(deleteButton);

    layout->setStretch(0,2);
    layout->setStretch(1,1);
    layout->setStretch(2,1);
    layout->setStretch(3,1);
    layout->setStretch(4,0);

    setLayout(layout);
}
