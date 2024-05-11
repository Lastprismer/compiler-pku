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

#pragma region RegisterManager

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

#pragma region StackMemoryModule

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
  StackMemoryModule smem = gen.StackMemManager;
  ostream& os = gen.Setting.getOs();
  switch (info.src.ty) {
    case StackMemoryModule::ValueType::imm:
      if (info.dest.ty == StackMemoryModule::ValueType::reg) {
        // store imm to reg
        // 现在不会用到，之后优化寄存器策略时可能用？
        li(os, info.dest.content.reg, info.src.content.imm);
      } else {
        // store imm to stack
        Reg rd = gen.RegManager.getAvailableReg();
        li(os, rd, info.src.content.imm);
        sw(os, Reg::sp, rd, info.dest.content.addr);
        gen.RegManager.releaseReg(rd);
      }
      break;
    case StackMemoryModule::ValueType::reg:
      if (info.dest.ty == StackMemoryModule::ValueType::reg) {
        // store reg to reg
        // 啥也不用做，最后会改表
      } else {
        // store reg to stack
        sw(os, Reg::sp, info.src.content.reg, info.dest.content.addr);
        gen.RegManager.releaseReg(info.src.content.reg);
      }
      break;
    case StackMemoryModule::ValueType::stack: {
      if (info.dest.ty == StackMemoryModule::ValueType::stack) {
        Reg rs = gen.RegManager.getAvailableReg();
        lw(os, rs, Reg::sp, info.src.content.addr);
        sw(os, Reg::sp, rs, info.dest.content.addr);
        gen.RegManager.releaseReg(rs);
      }
    }
  }
  return;
}

void StackMemoryModule::WriteLI(const Reg& rd, int imm) {
  ostream& os = RiscvGenerator::getInstance().Setting.getOs();
  li(os, rd, imm);
}

void StackMemoryModule::WriteLW(const Reg& rs, const Reg& rd, int addr) {
  ostream& os = RiscvGenerator::getInstance().Setting.getOs();
  lw(os, rs, rd, addr);
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
  StackUsed += 4;
  return StackUsed;
}

StackMemoryModule::StackMemoryModule()
    : BaseModule(), stackMemoryNeeded(0), StackUsed(0) {
  InstResult = map<koopa_raw_value_t, InstResultInfo>();
}

#pragma endregion

RiscvGenerator::RiscvGenerator() {
  Setting.setOs(cout).setIndent(0);
  FunctionName = "";
  RegManager = RegisterModule();
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
  int stackMemoryAlloc = StackMemManager.GetStackMem();
  if (stackMemoryAlloc == 0)
    return;
  if (IsImmInBound(-stackMemoryAlloc)) {
    addi(os, Reg::sp, Reg::sp, -stackMemoryAlloc);
  } else {
    Reg rd = RegManager.getAvailableReg();
    li(os, rd, -stackMemoryAlloc);
    add(os, Reg::sp, Reg::sp, rd);
    RegManager.releaseReg(rd);
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
    lw(os, a0, Reg::sp, retValueInfo.content.addr);
  }

  // 回收栈内存
  int stackMemoryAlloc = StackMemManager.GetStackMem();
  if (stackMemoryAlloc != 0) {
    if (IsImmInBound(stackMemoryAlloc)) {
      addi(os, Reg::sp, Reg::sp, stackMemoryAlloc);
    } else {
      Reg rd = RegManager.getAvailableReg();
      li(os, rd, stackMemoryAlloc);
      add(os, Reg::sp, Reg::sp, rd);
      RegManager.releaseReg(rd);
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
#pragma region neq
      neq(os, left, left, right);
      RegManager.releaseReg(right);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_EQ:
#pragma region eq
      eq(os, left, left, right);
      RegManager.releaseReg(right);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_GT:
#pragma region sgt
      sgt(os, left, left, right);
      RegManager.releaseReg(right);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_LT:
#pragma region slt
      slt(os, left, left, right);
      RegManager.releaseReg(right);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_GE:
#pragma region sge
      sge(os, left, left, right);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_LE:
#pragma region sle
      sle(os, left, left, right);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_ADD:
#pragma region add
      add(os, left, left, right);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_SUB:
#pragma region sub
      sub(os, left, left, right);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_MUL:
#pragma region mul
      mul(os, left, left, right);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_DIV:
#pragma region div
      div(os, left, left, right);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_MOD:
#pragma region mod
      rem(os, left, left, right);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_AND:
#pragma region and
      andr(os, left, left, right);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_OR:
#pragma region or
      orr(os, left, left, right);
      break;
#pragma endregion

    default:
      cerr << op;
      assert(false);
  }
}

}  // namespace riscv
