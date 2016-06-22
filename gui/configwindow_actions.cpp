#include "configwindow.h"
#include "action.h"
#include <QHBoxLayout>
#include <QLineEdit>

#include "actions_area_sector_dialog.h"
#include "actions_area_robot_dialog.h"
#include "actions_modifycounter_dialog.h"
#include "actions_enabledisabledialog.h"
#include "actionssounddialog.h"


#include <QDebug>

void configWindow::addAction(){
    QStringList list = actionTriggers.stringList();
    QString first = list.at(0);
    addAction(first, 0);
}

void configWindow::addAction(QString trigger, Action *action){
    QComboBox* triggerComboBox = new QComboBox(this);
    triggerComboBox->setModel(&actionTriggers);
    triggerComboBox->setCurrentText(trigger);
    QComboBox* actionType = new QComboBox(this);
    actionType->addItem("Shock");
    actionType->addItem("Set state");
    actionType->addItem("Modify counter");
    actionType->addItem("Light");
    actionType->addItem("Sound");
    QPushButton* setButton = new QPushButton("Set...", this);
    connect(setButton, SIGNAL(clicked(bool)), this, SLOT(actionActionSetPressed()));
    QPushButton* deleteButton = new QPushButton("X", this);
    connect(deleteButton, SIGNAL(clicked(bool)), this, SLOT(removeThisActionRow()));

    int row = ui->actionLayout->rowCount();
    ui->actionLayout->addWidget(triggerComboBox, row, 0);
    ui->actionLayout->addWidget(actionType, row, 1);
    ui->actionLayout->addWidget(setButton, row, 2);
    ui->actionLayout->addWidget(deleteButton, row, 3);
}

void configWindow::addArea(){
    AreaSectorSettings nullSettings;
    addArea("", nullSettings, false);
}

void configWindow::addArea(QString title, AreaSectorSettings settings, bool enabled){
    QLineEdit* titleEdit = new QLineEdit(title, this);
    connect(titleEdit, SIGNAL(textEdited(QString)), this, SLOT(triggersChanged(QString)));
    QComboBox* anchor = new QComboBox(this);
    anchor->addItem("Area frame");
    anchor->addItem("Robot frame");
    QPushButton* positionButton = new QPushButton("Set...", this);
    connect(positionButton, SIGNAL(clicked(bool)), this, SLOT(actionAreaSetPressed()));
    QCheckBox* enabledCheckBox = new QCheckBox("Enabled", this);
    enabledCheckBox->setChecked(enabled);
    QPushButton* deleteButton = new QPushButton("X", this);
    connect(deleteButton, SIGNAL(clicked(bool)), this, SLOT(removeThisAreaRow()));

    int row = ui->areaLayout->rowCount();
    ui->areaLayout->addWidget(titleEdit, row, 0);
    ui->areaLayout->addWidget(anchor, row, 1);
    ui->areaLayout->addWidget(positionButton, row, 2);
    ui->areaLayout->addWidget(enabledCheckBox, row, 3);
    ui->areaLayout->addWidget(deleteButton, row, 4);
}

void configWindow::addArea(QString title, AreaRobotSettings settings, bool enabled){
    QLineEdit* titleEdit = new QLineEdit(title, this);
    connect(titleEdit, SIGNAL(textEdited(QString)), this, SLOT(triggersChanged(QString)));
    QComboBox* anchor = new QComboBox(this);
    anchor->addItem("Area frame");
    anchor->addItem("Robot frame");
    anchor->setCurrentIndex(1);
    QPushButton* positionButton = new QPushButton("Set...", this);
    connect(positionButton, SIGNAL(clicked(bool)), this, SLOT(actionAreaSetPressed()));
    QCheckBox* enabledCheckBox = new QCheckBox("Enabled", this);
    enabledCheckBox->setChecked(enabled);
    QPushButton* deleteButton = new QPushButton("X", this);
    connect(deleteButton, SIGNAL(clicked(bool)), this, SLOT(removeThisAreaRow()));

    int row = ui->areaLayout->rowCount();
    ui->areaLayout->addWidget(titleEdit, row, 0);
    ui->areaLayout->addWidget(anchor, row, 1);
    ui->areaLayout->addWidget(positionButton, row, 2);
    ui->areaLayout->addWidget(enabledCheckBox, row, 3);
    ui->areaLayout->addWidget(deleteButton, row, 4);
}


void configWindow::addCounter(){
    addCounter("", 0, 0, false);
}

void configWindow::addCounter(QString title, double limit, double frequency, bool enabled){
    QLineEdit* titleEdit = new QLineEdit(title, this);
    connect(titleEdit, SIGNAL(textEdited(QString)), this, SLOT(triggersChanged(QString)));
    QDoubleSpinBox* frequencyBox = new QDoubleSpinBox(this);
    frequencyBox->setValue(frequency);
    frequencyBox->setDecimals(1);
    frequencyBox->setSuffix("/s");
    QDoubleSpinBox* limitBox = new QDoubleSpinBox(this);
    limitBox->setValue(limit);
    limitBox->setDecimals(1);
    QCheckBox* enabledCheckBox = new QCheckBox("Enabled", this);
    enabledCheckBox->setChecked(enabled);
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
    if (box->currentIndex() == 0){
        AreaSectorDialog dialog(this);
        dialog.exec();
    } else {
        AreaRobotDialog dialog(this, this);
        dialog.exec();
    }
}

void configWindow::actionActionSetPressed(){
    QObject* sender = QObject::sender();
    int index = ui->actionLayout->indexOf(qobject_cast<QWidget*>(sender));
    int row, x;
    ui->actionLayout->getItemPosition(index, &row, &x, &x, &x);
    QComboBox* box = qobject_cast<QComboBox*>(ui->actionLayout->itemAtPosition(row, 1)->widget());
    if (box->currentIndex() == Action::STATE){
        ActionEnableDisableDialog dialog(actionTriggers, this);
        dialog.exec();
    } else if (box->currentIndex() == Action::COUNTER){
        ActionModifyCounterDialog dialog(actionTriggers, this);
        dialog.exec();
    } else if (box->currentIndex() == Action::SOUND){
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
}

void configWindow::clearAllActions(){
}

void configWindow::triggersChanged(QString id){
    QStringList oldList = actionTriggers.stringList();

    // TODO: fix so that all fields get updated
    QLineEdit* sender = qobject_cast<QLineEdit*>(QObject::sender());
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
