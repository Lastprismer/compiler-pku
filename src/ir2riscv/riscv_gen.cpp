#include "riscv_gen.h"

namespace riscv {

#pragma region BaseModule

BaseModule::BaseModule() : active(false) {}

void BaseModule::Activate() {
  active = true;
}

void BaseModule::Deactivate() {
  active = false;
}

bool BaseModule::IsActive() {
  return active;
}

#pragma endregion

#pragma region Register

RegisterModule::RegisterModule() : BaseModule() {
  availableRegs = {};
  for (Reg reg = Reg::t0; reg != Reg::ra; reg = (Reg)((int)reg + 1)) {
    availableRegs.insert(reg);
  }
}

Reg RegisterModule::getAvailableReg() {
  assert(availableRegs.size() > 0);
  Reg reg = *availableRegs.begin();
  availableRegs.erase(reg);
  return reg;
}

void RegisterModule::releaseReg(Reg reg) {
  if (reg != Reg::NONE)
    availableRegs.insert(reg);
}

#pragma endregion

#pragma region Stack

void StackMemoryModule::InstResultInfo::Output() {
  cout << "type: ";
  switch (ty) {
    case ValueType::imm:
      cout << "imm, value: " << content.imm << endl;
      break;
    case ValueType::reg:
      cout << "reg, Reg: " << regstr(content.reg) << endl;
      break;
    case ValueType::stack:
      cout << "stack, addr: " << content.addr << endl;
      break;
    default:
      break;
  }
  return;
}

const int& StackMemoryModule::GetStackMem() {
  assert(IsActive());
  return stackMemoryNeeded;
}

void StackMemoryModule::SetStackMem(const int& mem) {
  assert(IsActive());
  stackMemoryNeeded = mem;
}

StackMemoryModule::StoreInfo::StoreInfo(const InstResultInfo& dstInfo,
                                        const InstResultInfo& srcInfo)
    : dest(dstInfo), src(srcInfo) {
  assert(dstInfo.ty != StackMemoryModule::ValueType::imm);
}

void StackMemoryModule::WriteStoreInst(const StoreInfo& info) {
  RiscvGenerator& gen = RiscvGenerator::getInstance();
  StackMemoryModule smem = gen.smem;
  ostream& os = gen.Setting.getOs();
  switch (info.src.ty) {
    case StackMemoryModule::ValueType::imm:
      if (info.dest.ty == StackMemoryModule::ValueType::reg) {
        // store imm to reg
        // 现在不会用到，之后优化寄存器策略时可能用？
        li(os, info.dest.content.reg, info.src.content.imm);
      } else {
        // store imm to stack
        Reg rd = gen.regmng.getAvailableReg();
        li(os, rd, info.src.content.imm);
        WriteSW(rd, info.dest.content.addr);
        gen.regmng.releaseReg(rd);
      }
      break;
    case StackMemoryModule::ValueType::reg:
      if (info.dest.ty == StackMemoryModule::ValueType::reg) {
        // store reg to reg
        // 啥也不用做，最后会改表
      } else {
        // store reg to stack
        WriteSW(info.src.content.reg, info.dest.content.addr);
        gen.regmng.releaseReg(info.src.content.reg);
      }
      break;
    case StackMemoryModule::ValueType::stack: {
      if (info.dest.ty == StackMemoryModule::ValueType::stack) {
        Reg rs = gen.regmng.getAvailableReg();
        WriteLW(rs, info.src.content.addr);
        WriteSW(rs, info.dest.content.addr);
        gen.regmng.releaseReg(rs);
      }
    }
  }
  return;
}

void StackMemoryModule::WriteLI(const Reg& rd, int imm) {
  ostream& os = RiscvGenerator::getInstance().Setting.getOs();
  li(os, rd, imm);
}

void StackMemoryModule::WriteLW(const Reg& rd, int addr) {
  auto& gen = RiscvGenerator::getInstance();
  ostream& os = gen.Setting.getOs();
  Reg adr = gen.regmng.getAvailableReg();
  li(os, adr, addr);
  add(os, adr, adr, Reg::sp);
  lw(os, rd, adr, 0);
  gen.regmng.releaseReg(adr);
}

void StackMemoryModule::WriteSW(const Reg& rs, int addr) {
  auto& gen = RiscvGenerator::getInstance();
  ostream& os = gen.Setting.getOs();
  Reg adr = gen.regmng.getAvailableReg();
  li(os, adr, addr);
  add(os, adr, adr, Reg::sp);
  sw(os, adr, rs, 0);
  gen.regmng.releaseReg(adr);
}

void StackMemoryModule::Debug_OutputInstResult() {
  cout << endl;
  cout << "count: " << InstResult.size() << endl;
  for (auto i = InstResult.begin(); i != InstResult.end(); i++) {
    cout << GetTypeString(i->first) << "  ";
    i->second.Output();
  }
  cout << endl;
}

int StackMemoryModule::IncreaseStackUsed() {
  int addr = StackUsed;
  StackUsed += 4;
  return addr;
}

StackMemoryModule::StackMemoryModule()
    : BaseModule(), stackMemoryNeeded(0), StackUsed(0) {
  InstResult = map<koopa_raw_value_t, InstResultInfo>();
}

#pragma endregion

#pragma region BBModule

BBModule::BBModule() {}

void BBModule::WriteBBName(const string& label) {
  ostream& os = RiscvGenerator::getInstance().Setting.getOs();
  wlabel(os, ParseSymbol(label));
}

void BBModule::WriteJumpInst(const string& label) {
  ostream& os = RiscvGenerator::getInstance().Setting.getOs();
  j(os, ParseSymbol(label));
}

void BBModule::WriteBranch(const Reg& cond,
                           const string& trueLabel,
                           const string& falseLabel) {
  ostream& os = RiscvGenerator::getInstance().Setting.getOs();
  const string trueMid = ParseSymbol(trueLabel) + "_mid";
  bnez(os, cond, trueMid);
  // 否则跳到false
  j(os, ParseSymbol(falseLabel));
  wlabel(os, trueMid);
  j(os, ParseSymbol(trueLabel));
}

#pragma endregion

#pragma region RiscvGen

RiscvGenerator::RiscvGenerator() : smem(), BBMan() {
  Setting.setOs(cout).setIndent(0);
  FunctionName = "";
  regmng = RegisterModule();
}

RiscvGenerator& RiscvGenerator::getInstance() {
  static RiscvGenerator riscgen;
  return riscgen;
}

void RiscvGenerator::WritePrologue() {
  ostream& os = Setting.getOs();
  os << "  .text" << endl;
  os << "  .globl " << FunctionName << endl;
  os << FunctionName << ":" << endl;

  // 分配栈内存
  int stackMemoryAlloc = smem.GetStackMem();
  if (stackMemoryAlloc == 0)
    return;
  if (IsImmInBound(-stackMemoryAlloc)) {
    addi(os, Reg::sp, Reg::sp, -stackMemoryAlloc);
  } else {
    Reg rd = regmng.getAvailableReg();
    li(os, rd, -stackMemoryAlloc);
    add(os, Reg::sp, Reg::sp, rd);
    regmng.releaseReg(rd);
  }
}

void RiscvGenerator::WriteEpilogue(
    const StackMemoryModule::InstResultInfo& retValueInfo) {
  ostream& os = Setting.getOs();
  // 此时栈内应有唯一值
  if (retValueInfo.ty == StackMemoryModule::ValueType::imm) {
    li(os, Reg::a0, retValueInfo.content.imm);
  } else if (retValueInfo.ty == StackMemoryModule::ValueType::reg) {
    mv(os, Reg::a0, retValueInfo.content.reg);
  } else {
    smem.WriteLW(a0, retValueInfo.content.addr);
  }

  // 回收栈内存
  int stackMemoryAlloc = smem.GetStackMem();
  if (stackMemoryAlloc != 0) {
    if (IsImmInBound(stackMemoryAlloc)) {
      addi(os, Reg::sp, Reg::sp, stackMemoryAlloc);
    } else {
      Reg rd = regmng.getAvailableReg();
      li(os, rd, stackMemoryAlloc);
      add(os, Reg::sp, Reg::sp, rd);
      regmng.releaseReg(rd);
    }
  }
  ret(os);
}

void RiscvGenerator::WriteBinaInst(OpType op,
                                   const Reg& left,
                                   const Reg& right) {
  ostream& os = Setting.getOs();

  switch (op) {
    case koopa_raw_binary_op::KOOPA_RBO_NOT_EQ:
      neq(os, left, left, right);
      break;

    case koopa_raw_binary_op::KOOPA_RBO_EQ:
      eq(os, left, left, right);
      break;

    case koopa_raw_binary_op::KOOPA_RBO_GT:
      sgt(os, left, left, right);
      break;

    case koopa_raw_binary_op::KOOPA_RBO_LT:
      slt(os, left, left, right);
      break;

    case koopa_raw_binary_op::KOOPA_RBO_GE:
      sge(os, left, left, right);
      break;

    case koopa_raw_binary_op::KOOPA_RBO_LE:
      sle(os, left, left, right);
      break;

    case koopa_raw_binary_op::KOOPA_RBO_ADD:
      add(os, left, left, right);
      break;

    case koopa_raw_binary_op::KOOPA_RBO_SUB:
      sub(os, left, left, right);
      break;

    case koopa_raw_binary_op::KOOPA_RBO_MUL:
      mul(os, left, left, right);
      break;

    case koopa_raw_binary_op::KOOPA_RBO_DIV:
      div(os, left, left, right);
      break;

    case koopa_raw_binary_op::KOOPA_RBO_MOD:
      rem(os, left, left, right);
      break;

    case koopa_raw_binary_op::KOOPA_RBO_AND:
      andr(os, left, left, right);
      break;

    case koopa_raw_binary_op::KOOPA_RBO_OR:
      orr(os, left, left, right);
      break;

    default:
      cerr << op;
      break;
      assert(false);
  }
}

#pragma endregion

}  // namespace riscv
