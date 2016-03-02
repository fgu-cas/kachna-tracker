#include "actions_actionwidget.h"

#include <QHBoxLayout>
#include <QPushButton>

#include <QDebug>

ActionWidget::ActionWidget(QStringListModel *model, QWidget *parent) : QWidget(parent)
{
    trigger = new QComboBox(this);
    trigger->addItem("Trigger...");
    trigger->setModel(model);

    action = new QComboBox(this);
    action->addItem("Action...");

    QPushButton *setButton = new QPushButton("Set", this);
    setButton->setMaximumWidth(40);

    target = new QComboBox(this);
    target->addItem("Target...");

    QPushButton *deleteButton = new QPushButton("X", this);
    deleteButton->setMaximumWidth(25);
    connect(deleteButton, SIGNAL(pressed()), this, SIGNAL(deleteMe()));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(QMargins(0, 0, 0, 0));

    layout->addWidget(trigger);
    layout->addWidget(action);
    layout->addWidget(setButton);
    layout->addWidget(target);
    layout->addWidget(deleteButton);

    layout->setStretch(0,1);
    layout->setStretch(1,1);
    layout->setStretch(2,0);
    layout->setStretch(3,1);
    layout->setStretch(4,0);

    setLayout(layout);
}
