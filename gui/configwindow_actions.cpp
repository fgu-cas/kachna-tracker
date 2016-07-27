#include "configwindow.h"
#include "action.h"
#include <QHBoxLayout>
#include <QLineEdit>

#include "actions_area_sector_dialog.h"
#include "actions_area_robot_dialog.h"
#include "actions_modifycounter_dialog.h"
#include "actions_enabledisabledialog.h"
#include "actionssounddialog.h"

void configWindow::addAction(){
    Action action;
    action.target = actionTriggers.stringList().at(0);
    addAction(action);
}

void configWindow::addAction(Action action){
    QComboBox* triggerComboBox = new QComboBox(this);
    triggerComboBox->setModel(&actionTriggers);
    triggerComboBox->setCurrentText(action.trigger);
    QComboBox* actionType = new QComboBox(this);
    actionType->addItem("Shock");
    actionType->addItem("Set state");
    actionType->addItem("Modify counter");/*
    actionType->addItem("Light");
    actionType->addItem("Sound");*/
    switch (action.type) {
        case Action::SHOCK:
            actionType->setCurrentIndex(0);
            break;
        case Action::ENABLE:
        case Action::DISABLE:
            actionType->setCurrentIndex(1);
            break;
        case Action::COUNTER_INC:
        case Action::COUNTER_DEC:
        case Action::COUNTER_SET:
            actionType->setCurrentIndex(2);
            break;
        case Action::SOUND:
            //
            break;
        case Action::LIGHT:
            //
            break;
    }
    QPushButton* setButton = new QPushButton("Set...", this);
    connect(setButton, SIGNAL(clicked(bool)), this, SLOT(actionActionSetPressed()));
    QPushButton* deleteButton = new QPushButton("X", this);
    connect(deleteButton, SIGNAL(clicked(bool)), this, SLOT(removeThisActionRow()));

    int row = ui->actionLayout->rowCount();
    ui->actionLayout->addWidget(triggerComboBox, row, 0);
    ui->actionLayout->addWidget(actionType, row, 1);
    ui->actionLayout->addWidget(setButton, row, 2);
    ui->actionLayout->addWidget(deleteButton, row, 3);

    partialActions[row] = action;
}

void configWindow::addArea(){
    Area nullSettings;
    addArea(nullSettings);
}

void configWindow::addArea(Area area){
    QStringList list = actionTriggers.stringList();
    list.append(area.id);
    actionTriggers.setStringList(list);

    QLineEdit* titleEdit = new QLineEdit(area.id, this);
    connect(titleEdit, SIGNAL(textEdited(QString)), this, SLOT(triggersChanged(QString)));
    QComboBox* anchor = new QComboBox(this);
    anchor->addItem("Robot frame");
    anchor->addItem("Arena frame");
    QPushButton* positionButton = new QPushButton("Set...", this);
    connect(positionButton, SIGNAL(clicked(bool)), this, SLOT(actionAreaSetPressed()));
    QCheckBox* enabledCheckBox = new QCheckBox("Enabled", this);
    enabledCheckBox->setChecked(area.enabled);
    QPushButton* deleteButton = new QPushButton("X", this);
    connect(deleteButton, SIGNAL(clicked(bool)), this, SLOT(removeThisAreaRow()));

    int row = ui->areaLayout->rowCount();
    ui->areaLayout->addWidget(titleEdit, row, 0);
    ui->areaLayout->addWidget(anchor, row, 1);
    ui->areaLayout->addWidget(positionButton, row, 2);
    ui->areaLayout->addWidget(enabledCheckBox, row, 3);
    ui->areaLayout->addWidget(deleteButton, row, 4);

    partialAreas[row] = area;
}

void configWindow::addCounter(){
    Counter nullCounter;
    addCounter(nullCounter);
}

void configWindow::addCounter(Counter counter){
    QStringList list = actionTriggers.stringList();
    list.append(counter.id);
    actionTriggers.setStringList(list);

    QLineEdit* titleEdit = new QLineEdit(counter.id, this);
    connect(titleEdit, SIGNAL(textEdited(QString)), this, SLOT(triggersChanged(QString)));
    QDoubleSpinBox* frequencyBox = new QDoubleSpinBox(this);
    frequencyBox->setValue(counter.frequency);
    frequencyBox->setDecimals(1);
    frequencyBox->setSuffix(" s");
    QSpinBox* limitBox = new QSpinBox(this);
    limitBox->setValue(counter.limit);
    QCheckBox* enabledCheckBox = new QCheckBox("Enabled", this);
    enabledCheckBox->setChecked(counter.enabled);
    QPushButton* deleteButton = new QPushButton("X", this);
    connect(deleteButton, SIGNAL(clicked(bool)), this, SLOT(removeThisCounterRow()));

    int row = ui->counterLayout->rowCount();
    ui->counterLayout->addWidget(titleEdit, row, 0);
    ui->counterLayout->addWidget(limitBox, row, 1);
    ui->counterLayout->addWidget(frequencyBox, row, 2);
    ui->counterLayout->addWidget(enabledCheckBox, row, 3);
    ui->counterLayout->addWidget(deleteButton, row, 4);
}

void configWindow::actionAreaSetPressed(){
    QObject* sender = QObject::sender();
    int index = ui->areaLayout->indexOf(qobject_cast<QWidget*>(sender));
    int row, x;
    ui->areaLayout->getItemPosition(index, &row, &x, &x, &x);
    QComboBox* box = qobject_cast<QComboBox*>(ui->areaLayout->itemAtPosition(row, 1)->widget());
    if (box->currentIndex() == 1){
        AreaSectorDialog dialog(this, row, partialAreas[row]);
        connect(&dialog, SIGNAL(update(int,Area)), this, SLOT(areaUpdate(int,Area)));
        dialog.exec();
    } else {
        AreaRobotDialog dialog(this, this, row, partialAreas[row]);
        connect(&dialog, SIGNAL(update(int,Area)), this, SLOT(areaUpdate(int,Area)));
        dialog.exec();
    }
}

void configWindow::actionActionSetPressed(){
    QObject* sender = QObject::sender();
    int index = ui->actionLayout->indexOf(qobject_cast<QWidget*>(sender));
    int row, x;
    ui->actionLayout->getItemPosition(index, &row, &x, &x, &x);
    QComboBox* box = qobject_cast<QComboBox*>(ui->actionLayout->itemAtPosition(row, 1)->widget());
    if (box->currentIndex() == 1){
        ActionEnableDisableDialog dialog(actionTriggers, row, partialActions[row], this);
        connect(&dialog, SIGNAL(update(int,Action)), this, SLOT(actionUpdate(int,Action)));
        dialog.exec();
    } else if (box->currentIndex() == 2){
        ActionModifyCounterDialog dialog(actionTriggers, row, partialActions[row], this);
        connect(&dialog, SIGNAL(update(int,Action)), this, SLOT(actionUpdate(int,Action)));
        dialog.exec();
    } else if (box->currentIndex() == 4){
        ActionsSoundDialog dialog(this);
        dialog.exec();
    }
}

void configWindow::removeThisActionRow(){
    QObject* sender = QObject::sender();
    int index = ui->actionLayout->indexOf(qobject_cast<QWidget*>(sender));
    int row, x;
    ui->actionLayout->getItemPosition(index, &row, &x, &x, &x);
    for (int i = 0; i < ui->actionLayout->columnCount(); i++){
        QLayoutItem* item = ui->actionLayout->itemAtPosition(row, i);
        delete item->widget();
    }
}

void configWindow::removeThisCounterRow(){
    QObject* sender = QObject::sender();
    int index = ui->counterLayout->indexOf(qobject_cast<QWidget*>(sender));
    int row, x;
    ui->counterLayout->getItemPosition(index, &row, &x, &x, &x);
    QString id = qobject_cast<QLineEdit*>(ui->counterLayout->itemAtPosition(row,0)->widget())->text();
    for (int i = 0; i < ui->counterLayout->columnCount(); i++){
        QLayoutItem* item = ui->counterLayout->itemAtPosition(row, i);
        delete item->widget();
    }
    QStringList list = actionTriggers.stringList();
    list.removeOne(id);
    actionTriggers.setStringList(list);
}

void configWindow::removeThisAreaRow(){
    QObject* sender = QObject::sender();
    int index = ui->areaLayout->indexOf(qobject_cast<QWidget*>(sender));
    int row, x;
    ui->areaLayout->getItemPosition(index, &row, &x, &x, &x);
    QString id = qobject_cast<QLineEdit*>(ui->areaLayout->itemAtPosition(row,0)->widget())->text();
    for (int i = 0; i < ui->areaLayout->columnCount(); i++){
        QLayoutItem* item = ui->areaLayout->itemAtPosition(row, i);
        delete item->widget();
    }
    QStringList list = actionTriggers.stringList();
    list.removeOne(id);
    actionTriggers.setStringList(list);

    partialAreas.remove(row);
}

void configWindow::clearAllActions(){
    partialAreas.clear();

    for (int row = 1; row < ui->areaLayout->rowCount(); row++){
        for (int column = 0; column < ui->areaLayout->columnCount(); column++){
            QLayoutItem* item = ui->areaLayout->itemAtPosition(row, column);
            if (item != 0){
                delete item->widget();
            }
        }
    }

    for (int row = 1; row < ui->counterLayout->rowCount(); row++){
        for (int column = 0; column < ui->counterLayout->columnCount(); column++){
            QLayoutItem* item = ui->counterLayout->itemAtPosition(row, column);
            if (item != 0){
                delete item->widget();
            }
        }
    }

    for (int row = 1; row < ui->actionLayout->rowCount(); row++){
        for (int column = 0; column < ui->actionLayout->columnCount(); column++){
            QLayoutItem* item = ui->actionLayout->itemAtPosition(row, column);
            if (item != 0){
                delete item->widget();
            }
        }
    }
}

void configWindow::triggersChanged(QString id){
    QLineEdit* sender = qobject_cast<QLineEdit*>(QObject::sender());
    int index = ui->areaLayout->indexOf(sender);
    if (index != -1){
        int row, x;
        ui->areaLayout->getItemPosition(index, &row, &x, &x, &x);
        partialAreas[row].id = id;
    }


    QStringList oldList = actionTriggers.stringList();

    // TODO: fix so that all fields get updated
    if (oldList.contains(id)){
        sender->setStyleSheet("QLineEdit { color: red; }");
    } else {
        sender->setStyleSheet("QLineEdit { color: black; }");
    }

    QStringList newList;
    newList.append("[START]");
    newList.append("[END]");

    for (int i = ui->areaLayout->columnCount(); i < ui->areaLayout->count(); i++){
        int col, x;
        ui->areaLayout->getItemPosition(i, &x, &col, &x, &x);
        if (col == 0){
            QLineEdit* edit = qobject_cast<QLineEdit*>(ui->areaLayout->itemAt(i)->widget());
            newList.append(edit->text());
        }
    }

    for (int i = ui->counterLayout->columnCount(); i < ui->counterLayout->count(); i++){
        int col, x;
        ui->counterLayout->getItemPosition(i, &x, &col, &x, &x);
        if (col == 0){
            QLineEdit* edit = qobject_cast<QLineEdit*>(ui->counterLayout->itemAt(i)->widget());
            newList.append(edit->text());
        }
    }

    actionTriggers.setStringList(newList);
}

void configWindow::areaUpdate(int row, Area area){
    partialAreas[row] = area;
}

void configWindow::actionUpdate(int row, Action action){
    partialActions[row] = action;
}

QList<Action> configWindow::getActionsFromUI(){
    QList<Action> result;

    for (int row = 1; row < ui->actionLayout->rowCount(); row++){
        QLayoutItem* item = ui->actionLayout->itemAtPosition(row, 0);
        if (item != 0){
            Action action;
            action.trigger = qobject_cast<QComboBox*>(item->widget())->currentText();
            action.type = partialActions[row].type;
            action.target = partialActions[row].target;
            result.append(action);
        }
    }

    return result;
}

QList<Counter> configWindow::getCountersFromUI(){
    QList<Counter> result;

    for (int row = 1; row < ui->counterLayout->rowCount(); row++){
        QLayoutItem* item = ui->counterLayout->itemAtPosition(row, 0);
        if (item != 0){
            Counter counter;
            counter.id = qobject_cast<QLineEdit*>(item->widget())->text();
            counter.limit = qobject_cast<QSpinBox*>(ui->counterLayout->itemAtPosition(row, 1)->widget())->value();
            counter.frequency = qobject_cast<QDoubleSpinBox*>(ui->counterLayout->itemAtPosition(row, 2)->widget())->value();
            counter.enabled = qobject_cast<QCheckBox*>(ui->counterLayout->itemAtPosition(row, 3)->widget())->isChecked();
            result.append(counter);
        }
    }

    return result;
}

QList<Area> configWindow::getAreasFromUI(){
    QList<Area> result;

    for (int row = 1; row < ui->areaLayout->rowCount(); row++){
        QLayoutItem* item = ui->areaLayout->itemAtPosition(row, 0);
        if (item != 0){
            Area area;
            area.id = qobject_cast<QLineEdit*>(item->widget())->text();
            area.enabled = qobject_cast<QCheckBox*>(ui->areaLayout->itemAtPosition(row, 3)->widget())->isChecked();
            area.type = partialAreas[row].type;
            area.angle = partialAreas[row].angle;
            area.distance = partialAreas[row].distance;
            area.radius = partialAreas[row].radius;
            area.range = partialAreas[row].range;
            result.append(area);
        }
    }

    return result;
}
