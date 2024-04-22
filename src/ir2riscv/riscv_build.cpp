#include "riscv_build.h"
namespace riscv {

void ret(ostream& os) {
  os << "  ret" << endl;
}

void li(ostream& os, const string& dest, int imm) {
  os << "  li " << dest << ", " << imm << endl;
}

void li(ostream& os, const Reg& dest, int imm) {
  li(os, regstr(dest), imm);
}

void mv(ostream& os, const string& rd, const string& rs) {
  os << "  mv " << rd << ", " << rs << endl;
}

void mv(ostream& os, const Reg& rd, const Reg& rs) {
  mv(os, regstr(rd), regstr(rs));
}

void add(ostream& os, const string& rd, const string& rs1, const string& rs2) {
  os << "  add " << rd << ", " << rs1 << ", " << rs2 << endl;
}

void add(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  add(os, regstr(rd), regstr(rs1), regstr(rs2));
}

void addi(ostream& os, const string& rd, const string& rs1, int imm) {
  os << "  addi " << rd << ", " << rs1 << ", " << imm << endl;
}

void addi(ostream& os, const Reg& rd, const Reg& rs1, int imm) {
  addi(os, regstr(rd), regstr(rs1), imm);
}

void sub(ostream& os, const string& rd, const string& rs1, const string& rs2) {
  os << "  sub " << rd << ", " << rs1 << ", " << rs2 << endl;
}

void sub(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  sub(os, regstr(rd), regstr(rs1), regstr(rs2));
}

void mul(ostream& os, const string& rd, const string& rs1, const string& rs2) {
  os << "  mul " << rd << ", " << rs1 << ", " << rs2 << endl;
}

void mul(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  mul(os, regstr(rd), regstr(rs1), regstr(rs2));
}

void div(ostream& os, const string& rd, const string& rs1, const string& rs2) {
  os << "  div " << rd << ", " << rs1 << ", " << rs2 << endl;
}

void div(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  div(os, regstr(rd), regstr(rs1), regstr(rs2));
}

void rem(ostream& os, const string& rd, const string& rs1, const string& rs2) {
  os << "  rem " << rd << ", " << rs1 << ", " << rs2 << endl;
}

void rem(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  rem(os, regstr(rd), regstr(rs1), regstr(rs2));
}

void seqz(ostream& os, const string& rd, const string& rs) {
  os << "  seqz " << rd << ", " << rs << endl;
}

void seqz(ostream& os, const Reg& rd, const Reg& rs) {
  seqz(os, regstr(rd), regstr(rs));
}

string regstr(Reg reg) {
  switch (reg) {
    case t0:
      return string("t0");
    case t1:
      return string("t1");
    case t2:
      return string("t2");
    case t3:
      return string("t3");
    case t4:
      return string("t4");
    case t5:
      return string("t5");
    case t6:
      return string("t6");
    case a0:
      return string("a0");
    case a1:
      return string("a1");
    case a2:
      return string("a2");
    case a3:
      return string("a3");
    case a4:
      return string("a4");
    case a5:
      return string("a5");
    case a6:
      return string("a6");
    case a7:
      return string("a7");
    case x0:
      return string("x0");
  }
}

}  // namespace riscv