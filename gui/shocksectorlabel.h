#ifndef SHOCKSECTORLABEL_H
#define SHOCKSECTORLABEL_H

#include <QLabel>

class ShockSectorLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ShockSectorLabel(QWidget *parent = 0);

signals:

public slots:
    void setAngle(double angle);
    void setRange(double range);
private:
    void update();

    int angle = 90*16;
    int range = 0;
};

#endif // SHOCKSECTORLABEL_H
