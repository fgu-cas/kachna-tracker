#include "actions_areawidget.h"

#include <QHBoxLayout>
#include <QPushButton>

#include "actions_area_robot_dialog.h"
#include "actions_area_sector_dialog.h"

#include <QDebug>

AreaWidget::AreaWidget(QWidget *parent) : QWidget(parent)
{    
    id = new QLineEdit(this);
    connect(id, SIGNAL(textChanged(QString)), this, SIGNAL(idChanged(QString)));

    anchor = new QComboBox(this);
    anchor->addItem("Fixed");
    // anchor->addItem("Arena frame");
    anchor->addItem("Robot frame");
    connect(anchor, SIGNAL(currentIndexChanged(int)), this, SLOT(onAnchorChanged(int)));

    position = new QPushButton("Position...", this);
    connect(position, SIGNAL(clicked(bool)), this, SLOT(onPositionTriggered()));

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

    onAnchorChanged(0);

    configWin = qobject_cast<configWindow*>(parentWidget());
}

void AreaWidget::onAnchorChanged(int index){
    selectionWidget.release();
    QDialog* newWidget = 0;

    switch(index){
        case 0:
            newWidget = new AreaSectorDialog(this);
            break;
        case 1:
            newWidget = new AreaRobotDialog(this, configWin);
            break;
    }

    selectionWidget.reset(newWidget);
}

void AreaWidget::onPositionTriggered(){
    if (selectionWidget.get() != 0){
        selectionWidget->show();
        selectionWidget->raise();
        selectionWidget->activateWindow();
    }
}
