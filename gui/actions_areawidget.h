#ifndef AREAWIDGET_H
#define AREAWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

class AreaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AreaWidget(QWidget *parent = 0);
signals:
    void idChanged(QString id);
    void deleteMe();
private:
    QLineEdit* id;
    QComboBox* anchor;
    QPushButton* position;
};

#endif // AREAWIDGET_H
