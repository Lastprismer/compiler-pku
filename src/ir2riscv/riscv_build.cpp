#include "riscv_build.h"
namespace riscv {

void ret(ostream& os) {
  os << "  ret" << endl;
}

void li(ostream& os, const Reg& dest, int imm) {
  os << "  li " << regstr(dest) << ", " << imm << endl;
}

void mv(ostream& os, const Reg& rd, const Reg& rs) {
  os << "  mv " << regstr(rd) << ", " << regstr(rs) << endl;
}

void add(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  os << "  add " << regstr(rd) << ", " << regstr(rs1) << ", " << regstr(rs2)
     << endl;
}

void addi(ostream& os, const Reg& rd, const Reg& rs1, int imm) {
  os << "  addi " << regstr(rd) << ", " << regstr(rs1) << ", " << imm << endl;
}

void sub(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  os << "  sub " << regstr(rd) << ", " << regstr(rs1) << ", " << regstr(rs2)
     << endl;
}

void mul(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  os << "  mul " << regstr(rd) << ", " << regstr(rs1) << ", " << regstr(rs2)
     << endl;
}

void div(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  os << "  div " << regstr(rd) << ", " << regstr(rs1) << ", " << regstr(rs2)
     << endl;
}

void rem(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  os << "  rem " << regstr(rd) << ", " << regstr(rs1) << ", " << regstr(rs2)
     << endl;
}

void seqz(ostream& os, const Reg& rd, const Reg& rs) {
  os << "  seqz " << regstr(rd) << ", " << regstr(rs) << endl;
}

void snez(ostream& os, const Reg& rd, const Reg& rs) {
  os << "  snez " << regstr(rd) << ", " << regstr(rs) << endl;
}

void slt(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  os << "  slt " << regstr(rd) << ", " << regstr(rs1) << ", " << regstr(rs2)
     << endl;
}

void sgt(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  os << "  sgt " << regstr(rd) << ", " << regstr(rs1) << ", " << regstr(rs2)
     << endl;
}

void xorr(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  os << "  xor " << regstr(rd) << ", " << regstr(rs1) << ", " << regstr(rs2)
     << endl;
}

void xori(ostream& os, const Reg& rd, const Reg& rs1, int imm) {
  os << "  xori " << regstr(rd) << ", " << regstr(rs1) << ", " << imm << endl;
}

void andr(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  os << "  and " << regstr(rd) << ", " << regstr(rs1) << ", " << regstr(rs2)
     << endl;
}

void orr(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  os << "  or " << regstr(rd) << ", " << regstr(rs1) << ", " << regstr(rs2)
     << endl;
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
    case NONE:
    default:
      assert(false);
  }
}

void sle(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  sgt(os, rd, rs1, rs2);
  xori(os, rd, rd, 1);
}

void sge(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  slt(os, rd, rs1, rs2);
  xori(os, rd, rd, 1);
}

void eq(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  xorr(os, rd, rs1, rs2);
  seqz(os, rd, rd);
}

void neq(ostream& os, const Reg& rd, const Reg& rs1, const Reg& rs2) {
  xorr(os, rd, rs1, rs2);
  snez(os, rd, rd);
}

}  // namespace riscv