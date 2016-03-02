#ifndef ACTIONWIDGET_H
#define ACTIONWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QStringListModel>

class ActionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ActionWidget(QStringListModel* model, QWidget *parent = 0);
signals:
    void deleteMe();
private:
    QComboBox* trigger;
    QComboBox* action;
    QComboBox* target;
};

#endif // ACTIONWIDGET_H
