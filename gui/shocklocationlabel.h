#ifndef SHOCKLOCATIONWIDGET_H
#define SHOCKLOCATIONWIDGET_H

#include <QLabel>

class ShockLocationLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ShockLocationLabel(QWidget *parent = 0);

public slots:
    void setDistance(int distance);
    void setAngle(int angle);
    void setRadius(int radius);
    void setCentered(bool state);

private:
    int distance = 0;
    int angle = 0;
    int radius = 0;
    bool centered = false;

    void update();
};

#endif // SHOCKLOCATIONWIDGET_H
