#include "ir2riscv.h"

void li(ostream& os, const string& dest, int imm) {
  os << "  li " << dest << ", " << imm << endl;
}
