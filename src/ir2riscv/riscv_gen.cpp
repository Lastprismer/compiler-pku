#include "riscv_gen.h"

namespace riscv {

Node::Node() {}

Node::Node(int i) : tag(NodeTag::imm) {
  content.imm = i;
}

Node::Node::Node(Reg reg) : tag(NodeTag::reg) {
  content.reg = reg;
}

Node::Node(const Node& n) : tag(n.tag), content(n.content) {}

Node::Node(Node&& n) : tag(n.tag), content(n.content) {}

RiscvGenerator::RiscvGenerator() {
  setting.setOs(cout).setIndent(0);
  available_regs = set<Reg>({
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
  function_name = "";
  node_stack = deque<Node>();
}

RiscvGenerator& RiscvGenerator::getInstance() {
  static RiscvGenerator riscgen;
  return riscgen;
}

void RiscvGenerator::writePrologue() {
  ostream& os = setting.getOs();
  os << "  .text" << endl;
  os << "  .globl " << function_name << endl;
  os << function_name << ":" << endl;
}

void RiscvGenerator::writeEpilogue() {
  ostream& os = setting.getOs();
  // 此时栈内应有唯一值
  assert(node_stack.size() >= 1);
  Node retn = node_stack.front();
  if (retn.tag == NodeTag::imm) {
    li(os, Reg::a0, retn.content.imm);
    ret(os);
  } else {
    mv(os, Reg::a0, retn.content.reg);
    ret(os);
  }
}

void RiscvGenerator::pushReg(Reg reg) {
  if (!setting.start_writing)
    return;
  Node bi;
  bi.tag = NodeTag::reg;
  bi.content.reg = reg;
  node_stack.push_front(bi);
}

void RiscvGenerator::pushImm(int imm) {
  if (!setting.start_writing)
    return;
  if (imm == 0) {
    // 保证所有立即数0都被替换为寄存器x0
    pushReg(Reg::x0);
  } else {
    Node bi;
    bi.tag = NodeTag::imm;
    bi.content.imm = imm;
    node_stack.push_front(bi);
  }
}

void RiscvGenerator::writeInst(OpType op) {
  if (!setting.start_writing)
    return;
  assert(node_stack.size() >= 2);
  // 取出两个值，先进的为左
  Node right = node_stack.front();
  node_stack.pop_front();
  Node left = node_stack.front();
  node_stack.pop_front();

  Reg rd = genInst(left, right, op);

  // 推入结果
  pushReg(rd);
}

Reg RiscvGenerator::getAvailableReg() {
  assert(available_regs.size() > 0);
  Reg reg = *available_regs.begin();
  available_regs.erase(reg);
  return reg;
}

void RiscvGenerator::releaseReg(Reg reg) {
  available_regs.insert(reg);
}

Reg RiscvGenerator::genInst(Node& left, Node& right, OpType op) {
  ostream& os = setting.getOs();
  Reg rd = getAvailableReg();
  switch (op) {
    case koopa_raw_binary_op::KOOPA_RBO_EQ:
      // TODO: 当前只有左端会是0
      assert(left.tag == NodeTag::reg && left.content.reg == Reg::x0);

      if (left.tag == NodeTag::reg && left.content.reg == Reg::x0) {
        if (right.tag == NodeTag::reg) {
          // 使用seqz
          seqz(os, rd, right.content.reg);
        } else {
          // 分配rs，用完就释放
          // TODO: 之后优化为常数
          Reg rs = getAvailableReg();
          li(os, rs, right.content.imm);
          seqz(os, rd, rs);
          releaseReg(rs);
        }
      }

      break;
    case koopa_raw_binary_op::KOOPA_RBO_SUB:
      if (left.tag == NodeTag::imm) {
        // left分配，使用完就释放
        Reg lreg = getAvailableReg();
        if (right.tag == NodeTag::imm) {
          // 使用addi rd, left, -right
          addi(os, rd, lreg, -right.content.imm);
        } else {
          sub(os, rd, lreg, right.content.reg);
        }
        releaseReg(lreg);

      } else if (left.tag == NodeTag::reg) {
        if (right.tag == NodeTag::imm) {
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

void RiscvGenerator::immZero2Regx0(Node& node) {
  if (node.tag == NodeTag::imm && node.content.imm == 0) {
    node.tag = NodeTag::reg;
    node.content.reg = Reg::x0;
  }
}

}