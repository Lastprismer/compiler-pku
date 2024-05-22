#include "output_setting.h"

GenSettings::GenSettings() : shouldWriting(true) {}

GenSettings& GenSettings::setOs(ostream& o) {
  os = &o;
  return *this;
}

GenSettings& GenSettings::setIndent(int val) {
  indent = val;
  return *this;
}

int& GenSettings::getIndent() {
  return indent;
}

ostream& GenSettings::getOs() const {
  return *os;
}

const string GenSettings::getIndentStr() const {
  return string(indent, ' ');
}
