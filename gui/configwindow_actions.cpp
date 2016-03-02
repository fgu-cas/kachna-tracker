#include "configwindow.h"
#include "actions_actionwidget.h"
#include "actions_areawidget.h"
#include "actions_counterwidget.h"
#include <QHBoxLayout>

#include <QDebug>

void configWindow::addAction(){
    ActionWidget* widget = new ActionWidget(&actionTriggers);
    connect(widget, SIGNAL(deleteMe()), this, SLOT(removeActionWidget()));
    ui->actionsLayout->addWidget(widget);
}

void configWindow::addAction(QString trigger, Action action, QString target){

}

void configWindow::addArea(){
    AreaWidget* widget = new AreaWidget();
    connect(widget, SIGNAL(deleteMe()), this, SLOT(removeActionWidget()));
    connect(widget, SIGNAL(idChanged(QString)), this, SLOT(triggersChanged(QString)));
    ui->areasLayout->addWidget(widget);
}

void configWindow::addCounter(){
    CounterWidget* widget = new CounterWidget();
    connect(widget, SIGNAL(deleteMe()), this, SLOT(removeActionWidget()));
    connect(widget, SIGNAL(idChanged(QString)), this, SLOT(triggersChanged(QString)));
    ui->counterLayout->addWidget(widget);

}

void configWindow::removeActionWidget(){
    QObject* sender = QObject::sender();
    ui->actionsLayout->removeWidget(qobject_cast<QWidget*>(sender));
    delete sender;
}

void configWindow::clearAllActions(){
}

void configWindow::triggersChanged(QString id){
    QObject* sender = QObject::sender();

    int index = 0;

    for (int i = 2; i < ui->areasLayout->count(); i++){
        if (ui->areasLayout->itemAt(i)->widget() == sender){
            goto found;
        }
        index++;
    }

    for (int i = 2; i < ui->counterLayout->count(); i++){
        if (ui->counterLayout->itemAt(i)->widget() == sender){
            goto found;
        }
        index++;
    }


    found:
    QStringList list = actionTriggers.stringList();

    if (index == list.count()){
        list.append(id);
    } else {
        list.replace(index, id);
    }

    actionTriggers.setStringList(list);
}
