#include "masklabel.h"
#include <qmath.h>

void MaskLabel::setEditable(bool editable){
    this->editable = editable;
}

void MaskLabel::mousePressEvent(QMouseEvent *ev){
    if (editable){
        in_editing = true;
        x = ev->x();
        y = ev->y();
        emit(posChangedX(x));
        emit(posChangedY(y));
        emit(radiusChanged(0));
    }
    ev->accept();
}

void MaskLabel::mouseReleaseEvent(QMouseEvent *ev){
   in_editing = false;
   ev->accept();
}

void MaskLabel::mouseMoveEvent(QMouseEvent *ev){
    if (in_editing){
        int dx = ev->x() - x;
        int dy = ev->y() - y;
        int res = qSqrt(dx*dx + dy*dy);
        emit(radiusChanged(res));
    }

    ev->accept();
}
