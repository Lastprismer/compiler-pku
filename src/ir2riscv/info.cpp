#include "info.h"

IRFuncInfo::IRFuncInfo() : start_writing(false) {
  aval_regs = set<reg_t>({
      t0,
      t1,
      t2,
      t3,
      t4,
      t5,
      t6,
      a0,
      a1,
      a2,
      a3,
      a4,
      a5,
      a6,
      a7,
  });
}

void IRFuncInfo::write_prologue(ostream& os) {
  os << "  .text" << endl;
  os << "  .globl " << func_name << endl;
  os << func_name << ":" << endl;
}

void IRFuncInfo::write_epilogue(ostream& os) {
  // 此时栈内应有唯一值
  assert(node_stack.size() >= 1);
  node_t ret = node_stack.front();
  if (ret.tag == nodetype_t::imm) {
    li(os, reg_t::a0, ret.content.imm);
    build::ret(os);
  } else {
    mv(os, reg_t::a0, ret.content.reg);
    build::ret(os);
  }
}

void IRFuncInfo::push_reg(reg_t reg) {
  if (!start_writing)
    return;
  node_t bi;
  bi.tag = nodetype_t::reg;
  bi.content.reg = reg;
  node_stack.push_front(bi);
}

void IRFuncInfo::push_imm(int imm) {
  if (!start_writing)
    return;
  if (imm == 0) {
    // 保证所有立即数0都被替换为寄存器x0
    push_reg(reg_t::x0);
  } else {
    node_t bi;
    bi.tag = nodetype_t::imm;
    bi.content.imm = imm;
    node_stack.push_front(bi);
  }
}

void IRFuncInfo::write_inst(ostream& os, biop_t op) {
  if (!start_writing)
    return;
  assert(node_stack.size() >= 2);
  // 取出两个值，先进的为左
  node_t right = node_stack.front();
  node_stack.pop_front();
  node_t left = node_stack.front();
  node_stack.pop_front();

  reg_t rd = gen_inst(os, left, right, op);

  // 推入结果
  push_reg(rd);
}

reg_t IRFuncInfo::get_aval_reg() {
  assert(aval_regs.size() > 0);
  reg_t reg = *aval_regs.begin();
  aval_regs.erase(reg);
  return reg;
}

void IRFuncInfo::release_reg(reg_t reg) {
  aval_regs.insert(reg);
}

reg_t IRFuncInfo::gen_inst(ostream& os,
                           node_t& left,
                           node_t& right,
                           biop_t op) {
  reg_t rd = get_aval_reg();
  switch (op) {
    case koopa_raw_binary_op::KOOPA_RBO_EQ:
      // TODO: 当前只有左端会是0
      assert(left.tag == nodetype_t::reg && left.content.reg == reg_t::x0);

      if (left.tag == nodetype_t::reg && left.content.reg == reg_t::x0) {
        if (right.tag == nodetype_t::reg) {
          // 使用seqz
          seqz(os, rd, right.content.reg);
        } else {
          // 分配rs，用完就释放
          // TODO: 之后优化为常数
          reg_t rs = get_aval_reg();
          li(os, rs, right.content.imm);
          seqz(os, rd, rs);
          release_reg(rs);
        }
      }

      break;
    case koopa_raw_binary_op::KOOPA_RBO_SUB:
      if (left.tag == nodetype_t::imm) {
        // left分配，使用完就释放
        reg_t lreg = get_aval_reg();
        if (right.tag == nodetype_t::imm) {
          // 使用addi rd, left, -right
          addi(os, rd, lreg, -right.content.imm);
        } else {
          sub(os, rd, lreg, right.content.reg);
        }
        release_reg(lreg);

      } else if (left.tag == nodetype_t::reg) {
        if (right.tag == nodetype_t::imm) {
          // 使用addi rd, left, -right
          addi(os, rd, left.content.reg, -right.content.imm);
        } else {
          sub(os, rd, left.content.reg, right.content.reg);
        }
      }
      break;

    default:
      assert(false);
  }
  return rd;
}

void IRFuncInfo::imm_zero2rig_x0(node_t& node) {
  if (node.tag == nodetype_t::imm && node.content.imm == 0) {
    node.tag = nodetype_t::reg;
    node.content.reg = reg_t::x0;
  }
}
