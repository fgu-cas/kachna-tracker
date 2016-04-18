#ifndef ACTIONWIDGET_H
#define ACTIONWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QStringListModel>

class ActionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ActionWidget(QStringListModel* model, QWidget *parent = 0);
public slots:
    void actionChanged(int);
    void setPressed();
signals:
    void deleteMe();
private:
    QComboBox* trigger;
    QComboBox* action;
    QPushButton* setButton;
    QComboBox* target;

    QStringListModel* model;

    QDialog* setDialog;
};

#endif // ACTIONWIDGET_H
