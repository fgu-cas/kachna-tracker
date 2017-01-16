#ifndef MASKLABEL_H
#define MASKLABEL_H

#include <QLabel>
#include <QMouseEvent>

class MaskLabel : public QLabel
{
	Q_OBJECT
public:
	explicit MaskLabel(QWidget *parent = 0) : QLabel(parent) {}

	void mousePressEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);

signals:
	void posChangedX(int x);
	void posChangedY(int y);
	void radiusChanged(int r);

	public slots:
	void setEditable(bool editable);

private:
	bool editable = false;
	bool in_editing = false;
	int x;
	int y;
};

#endif // MASKLABEL_H
