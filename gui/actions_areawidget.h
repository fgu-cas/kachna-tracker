#ifndef AREAWIDGET_H
#define AREAWIDGET_H

#include <memory>

#include <QWidget>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

#include <configwindow.h>

class AreaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AreaWidget(QWidget *parent = 0);

public slots:
    void onPositionTriggered();
    void onAnchorChanged(int index);

signals:
    void idChanged(QString id);
    void deleteMe();
    
private:
    std::unique_ptr<QDialog> selectionWidget;
    configWindow* configWin;

    QLineEdit* id;
    QComboBox* anchor;
    QPushButton* position;
};

#endif // AREAWIDGET_H
