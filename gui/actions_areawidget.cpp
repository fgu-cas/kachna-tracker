#include "actions_areawidget.h"

#include <QHBoxLayout>
#include <QPushButton>

#include <QDebug>

AreaWidget::AreaWidget(QWidget *parent) : QWidget(parent)
{
    id = new QLineEdit(this);
    connect(id, SIGNAL(textChanged(QString)), this, SIGNAL(idChanged(QString)));

    anchor = new QComboBox(this);
    anchor->addItem("Fixed");
    anchor->addItem("Arena frame");
    anchor->addItem("Robot frame");

    position = new QPushButton("Position...", this);

    QPushButton *deleteButton = new QPushButton("X", this);
    deleteButton->setMaximumWidth(25);
    connect(deleteButton, SIGNAL(pressed()), this, SIGNAL(deleteMe()));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(QMargins(0, 0, 0, 0));

    layout->addWidget(id);
    layout->addWidget(anchor);
    layout->addWidget(position);
    layout->addWidget(deleteButton);

    layout->setStretch(0,1);
    layout->setStretch(1,1);
    layout->setStretch(2,1);
    layout->setStretch(3,0);

    setLayout(layout);
}
