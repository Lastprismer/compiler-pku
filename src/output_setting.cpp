#include "output_setting.h"

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

ostream& GenSettings::getOs() {
  return *os;
}

string GenSettings::getIndentStr() {
  return string(indent, ' ');
}
