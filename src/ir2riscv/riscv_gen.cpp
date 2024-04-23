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
  // if (imm == 0) {
  // 保证所有立即数0都被替换为寄存器x0
  // 不保证
  // pushReg(Reg::x0);
  Node bi;
  bi.tag = NodeTag::imm;
  bi.content.imm = imm;
  node_stack.push_front(bi);
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

  if (left.tag == NodeTag::imm && right.tag == NodeTag::imm) {
    pushImm(magicInst(left, right, op));
    return;
  }

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
  if (reg != Reg::NONE)
    available_regs.insert(reg);
}

Reg RiscvGenerator::genInst(Node& left, Node& right, OpType op) {
  ostream& os = setting.getOs();
  Reg rs = Reg::NONE;
  Reg rd = Reg::NONE;

  if (op == koopa_raw_binary_op::KOOPA_RBO_ADD) {
#pragma region add
    if (left.tag == NodeTag::imm) {
      if (right.tag == NodeTag::imm) {
        // imm + imm
        rd = getAvailableReg();
        if (immInBound(right.content.imm)) {
          // addi right to left
          li(os, rd, left.content.imm);
          addi(os, rd, rd, right.content.imm);
          return rd;
        } else if (immInBound(left.content.imm)) {
          // addi left to right
          li(os, rd, right.content.imm);
          addi(os, rd, rd, left.content.imm);
          return rd;
        } else {
          // add down
          rs = getAvailableReg();
          li(os, rd, left.content.imm);
          li(os, rs, right.content.imm);
        }
      } else if (right.tag == NodeTag::reg) {
        // imm + reg
        rd = right.content.reg;
        if (immInBound(left.content.imm)) {
          addi(os, rd, rd, left.content.imm);
          return rd;
        } else {
          // add down
          rs = getAvailableReg();
          li(os, rs, left.content.imm);
        }
      }
    } else if (left.tag == NodeTag::reg) {
      if (right.tag == NodeTag::imm) {
        // reg + imm
        rd = left.content.reg;
        if (immInBound(right.content.imm)) {
          addi(os, rd, rd, right.content.imm);
          return rd;
        } else {
          // add down
          rs = getAvailableReg();
          li(os, rs, right.content.imm);
        }
        return rd;
      } else if (right.tag == NodeTag::reg) {
        // reg + reg
        rd = left.content.reg;
        rs = right.content.reg;
      }
      add(os, rd, rd, rs);
      releaseReg(rs);
      return rd;
    }
#pragma endregion

  } else if (op == koopa_raw_binary_op::KOOPA_RBO_SUB) {
#pragma region sub
    if (left.tag == NodeTag::imm) {
      // imm - x
      rd = getAvailableReg();
      li(os, rd, left.content.imm);
    } else
      rd = left.content.reg;
    // reg - x
    if (right.tag == NodeTag::imm) {
      // reg - imm
      if (immInBound(-right.content.imm)) {
        addi(os, rd, rd, -right.content.imm);
        return rd;
      } else {
        rs = getAvailableReg();
        li(os, rs, right.content.imm);
        sub(os, rd, rd, rs);
        releaseReg(rs);
        return rd;
      }
    } else {
      // reg - reg
      rs = right.content.reg;
      sub(os, rd, rd, rs);
      releaseReg(rs);
      return rd;
    }
#pragma endregion
  }

  if (left.tag == NodeTag::imm) {
    rd = getAvailableReg();
    li(os, rd, left.content.imm);
  } else {
    rd = left.content.reg;
  }
  if (right.tag == NodeTag::imm) {
    rs = getAvailableReg();
    li(os, rs, right.content.imm);
  } else {
    rs = right.content.reg;
  }

  switch (op) {
    case koopa_raw_binary_op::KOOPA_RBO_NOT_EQ:
#pragma region neq
      neq(os, rd, rd, rs);
      releaseReg(rs);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_EQ:
#pragma region eq
      eq(os, rd, rd, rs);
      releaseReg(rs);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_GT:
#pragma region sgt
      sgt(os, rd, rd, rs);
      releaseReg(rs);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_LT:
#pragma region slt
      slt(os, rd, rd, rs);
      releaseReg(rs);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_GE:
#pragma region sge
      sge(os, rd, rd, rs);
      releaseReg(rs);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_LE:
#pragma region sle
      sle(os, rd, rd, rs);
      releaseReg(rs);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_ADD:
    case koopa_raw_binary_op::KOOPA_RBO_SUB:
      assert(false);
    case koopa_raw_binary_op::KOOPA_RBO_MUL:
#pragma region mul
      mul(os, rd, rd, rs);
      releaseReg(rs);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_DIV:
#pragma region div
      div(os, rd, rd, rs);
      releaseReg(rs);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_MOD:
#pragma region mod
      rem(os, rd, rd, rs);
      releaseReg(rs);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_AND:
#pragma region and
      andr(os, rd, rd, rs);
      releaseReg(rs);
      break;
#pragma endregion

    case koopa_raw_binary_op::KOOPA_RBO_OR:
#pragma region or
      orr(os, rd, rd, rs);
      releaseReg(rs);
      break;
#pragma endregion

    default:
      cerr << op;
      assert(false);
  }
  return rd;
}

#pragma region uncanny feat
int RiscvGenerator::magicInst(Node& left, Node& right, OpType op) {
  int l = left.content.imm;
  int r = right.content.imm;
  switch (op) {
    case koopa_raw_binary_op::KOOPA_RBO_NOT_EQ:
      return l != r;
    case koopa_raw_binary_op::KOOPA_RBO_EQ:
      return l == r;
    case koopa_raw_binary_op::KOOPA_RBO_GT:
      return l > r;
    case koopa_raw_binary_op::KOOPA_RBO_LT:
      return l < r;
    case koopa_raw_binary_op::KOOPA_RBO_GE:
      return l >= r;
    case koopa_raw_binary_op::KOOPA_RBO_LE:
      return l <= r;
    case koopa_raw_binary_op::KOOPA_RBO_ADD:
      return l + r;
    case koopa_raw_binary_op::KOOPA_RBO_SUB:
      return l - r;
    case koopa_raw_binary_op::KOOPA_RBO_MUL:
      return l * r;
    case koopa_raw_binary_op::KOOPA_RBO_DIV:
      return l / r;
    case koopa_raw_binary_op::KOOPA_RBO_MOD:
      return l % r;
    case koopa_raw_binary_op::KOOPA_RBO_AND:
      return l && r;
    case koopa_raw_binary_op::KOOPA_RBO_OR:
      return l | r;
    default:
      assert(false);
  }
}
#pragma endregion

}  // namespace riscv
