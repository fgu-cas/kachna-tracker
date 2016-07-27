#include <QDataStream>
#include <QMetaType>
#include "params.h"


QDataStream &operator<<(QDataStream &out, const Area &obj) {
    out << obj.id << obj.radius << obj.distance << obj.angle << obj.range << (int) obj.type << obj.enabled;
    return out;
}
QDataStream &operator>>(QDataStream &in, Area &obj) {
    in >> obj.id >> obj.radius >> obj.distance >> obj.angle >> obj.range >> (int&) obj.type >> obj.enabled;
    return in;
}

bool Area::operator==(const Area &other) const {
  if (this->type != other.type) return false;
  if (this->id != other.id) return false;
  if (this->enabled != other.enabled) return false;
  if (this->type == CIRCULAR_AREA){
      if (this->angle != other.angle) return false;
      if (this->radius != other.radius) return false;
      if (this->distance != other.distance) return false;
  } else {
      if (this->angle != other.angle) return false;
      if (this->range != other.range) return false;
  }

  return true;
}

QDataStream &operator<<(QDataStream &out, const Counter &obj) {
    out << obj.id << obj.enabled << obj.value << obj.frequency << obj.limit << (int) obj.type;
    return out;
}
QDataStream &operator>>(QDataStream &in, Counter &obj) {
    in >> obj.id >> obj.enabled >> obj.value >> obj.frequency >> obj.limit >> (int&) obj.type;
    return in;
}

bool Counter::operator==(const Counter &other) const {
  if (this->type != other.type) return false;
  if (this->id != other.id) return false;
  if (this->enabled != other.enabled) return false;
  if (this->limit != other.limit) return false;
  if (this->frequency != other.frequency) return false;
  if (this->value != other.value) return false; // ?

  return true;
}

QDataStream &operator<<(QDataStream &out, const Action &obj) {
    out << obj.trigger << (int) obj.type << obj.target << obj.arg;
    return out;
}
QDataStream &operator>>(QDataStream &in, Action &obj) {
    in >> obj.trigger >> (int&) obj.type >> obj.target >> obj.arg;
    return in;
}

bool Action::operator==(const Action &other) const {
    if (this->trigger != other.trigger) return false;
    if (this->type != other.type) return false;
    if (this->target != other.target) return false;

    return true;
}
