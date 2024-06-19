#include "riscv_build.h"
namespace riscv {

void ret(ostream& os) {
  os << "  ret" << endl;
}

void li(ostream& os, const Reg& dest, int imm) {
  os << "  li " << regstr(dest) << ", " << imm << endl;
}

void lw(ostream& os, const Reg& rd, const Reg& rs, int addr) {
  os << "  lw " << regstr(rd) << ", " << addr << "(" << regstr(rs) << ")"
     << endl;
}

void sw(ostream& os, const Reg& rd, const Reg& rs, int addr) {
  os << "  sw " << regstr(rs) << ", " << addr << "(" << regstr(rd) << ")"
     << endl;
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

void j(ostream& os, const string& label) {
  os << "  j " << label << endl;
}

void bnez(ostream& os, const Reg& reg, const string& label) {
  os << "  bnez " << regstr(reg) << ", " << label << endl;
}

void beqz(ostream& os, const Reg& reg, const string& label) {
  os << "  beqz " << regstr(reg) << ", " << label << endl;
}

void call(ostream& os, const string& name) {
  os << "  call " << name << endl;
}

void la(ostream& os, const Reg& reg, const string& name) {
  os << "  la " << regstr(reg) << ", " << name << endl;
}

const char* regstr(Reg reg) {
  switch (reg) {
    case t0:
      return "t0";
    case t1:
      return "t1";
    case t2:
      return "t2";
    case t3:
      return "t3";
    case t4:
      return "t4";
    case t5:
      return "t5";
    case t6:
      return "t6";
    case a0:
      return "a0";
    case a1:
      return "a1";
    case a2:
      return "a2";
    case a3:
      return "a3";
    case a4:
      return "a4";
    case a5:
      return "a5";
    case a6:
      return "a6";
    case a7:
      return "a7";
    case s0:
      return "s0";
    case s1:
      return "s1";
    case s2:
      return "s2";
    case s3:
      return "s3";
    case s4:
      return "s4";
    case s5:
      return "s5";
    case s6:
      return "s6";
    case s7:
      return "s7";
    case s8:
      return "s8";
    case s9:
      return "s9";
    case s10:
      return "s10";
    case s11:
      return "s11";
    case ra:
      return "ra";
    case sp:
      return "sp";
    case x0:
      return "x0";
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

void wlabel(ostream& os, const string& label) {
  os << label << ":" << endl;
}

}  // namespace riscv