#include "actions_actionwidget.h"

#include "actions_modifycounter_dialog.h"

#include <QHBoxLayout>

#include <QDebug>

ActionWidget::ActionWidget(QStringListModel *model, QWidget *parent):
    QWidget(parent),
    model(model)
{
    trigger = new QComboBox(this);
    trigger->addItem("Trigger...");
    trigger->setModel(model);

    action = new QComboBox(this);
    action->addItem("Shock");
    action->addItem("Enable");
    action->addItem("Disable");
    action->addItem("Modify counter");
    action->addItem("Play sound");
    action->addItem("Make light");
    connect(action, SIGNAL(currentIndexChanged(int)), this, SLOT(actionChanged(int)));

    setButton = new QPushButton("Set", this);
    setButton->setMaximumWidth(40);
    connect(setButton, SIGNAL(clicked(bool)), this, SLOT(setPressed()));

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

    actionChanged(0);
}

void ActionWidget::actionChanged(int index){
    switch (index) {
        case 0: // shock
            setButton->setEnabled(false);
            target->setCurrentText("Rat");
            target->setEnabled(false);
        case 1: // Enable, fall-through
        case 2: // Disable
            setButton->setEnabled(false);
            target->setModel(model);
            target->setEnabled(true);
            break;
        case 3: // modify counter
            setDialog = new ActionModifyCounterDialog(this);
            setButton->setEnabled(true);
            target->setModel(model); //TODO
            target->setEnabled(true);
            break;
        case 4:
        case 5:
            setButton->setEnabled(true);
            target->setEnabled(false);
            break;
    }
}

void ActionWidget::setPressed(){
    if (setDialog != 0){
        setDialog->show();
        setDialog->raise();
        setDialog->activateWindow();
    }
}
