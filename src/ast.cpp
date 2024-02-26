#include "ast.h"

void CompUnitAST::Print(int indent) const {
  make_indent(indent);
  std::cout << "CompUnitAST {" << std::endl;
  func_def->Print(indent + 1);
  make_indent(indent);
  std::cout << " }," << std::endl;
}

void CompUnitAST::Dump(std::ostream& os, int indent) const {
  func_def->Dump(os, indent + 1);
}

void FuncDefAST::Print(int indent) const {
  make_indent(indent);
  std::cout << "FuncDefAST {" << std::endl;
  func_type->Print(indent + 1);
  make_indent(indent + 1);
  std::cout << "IDENT: \"" << ident << "\"," << std::endl;
  block->Print(indent + 1);
  make_indent(indent);
  std::cout << " }," << std::endl;
}

void FuncDefAST::Dump(std::ostream& os, int indent) const {
  make_indent(indent);
  os << "fun @" << ident << "(): ";
  func_type->Dump(os, indent + 1);
  os << " {" << std::endl;
  block->Dump(os, indent);
  make_indent(indent);
  os << "}" << std::endl;
}

void FuncTypeAST::Print(int indent) const {
  make_indent(indent);
  std::cout << "FuncTypeAST: int," << std::endl;
}

void FuncTypeAST::Dump(std::ostream& os, int indent) const {
  os << "i32";
}

void BlockAST::Print(int indent) const {
  make_indent(indent);
  std::cout << "BlockAST {" << std::endl;
  stmt->Print(indent + 1);
  make_indent(indent);
  std::cout << " }," << std::endl;
}

void BlockAST::Dump(std::ostream& os, int indent) const {
  make_indent(indent);
  os << "%"
     << "entry:" << std::endl;
  stmt->Dump(os, indent + 1);
}

void StmtAST::Print(int indent) const {
  make_indent(indent);
  std::cout << "StmtAST {" << std::endl;
  number->Print(indent + 1);
  make_indent(indent);
  std::cout << " }," << std::endl;
}

void StmtAST::Dump(std::ostream& os, int indent) const {
  make_indent(indent);
  os << "ret ";
  number->Dump(os, indent);
  os << std::endl;
}

void NumberAST::Print(int indent) const {
  make_indent(indent);
  std::cout << "NumberAST {" << std::endl;
  make_indent(indent + 1);
  std::cout << "int_const: " << int_const << std::endl;
  make_indent(indent);
  std::cout << " }," << std::endl;
}

void NumberAST::Dump(std::ostream& os, int indent) const {
  os << int_const;
}

void make_indent(int indent) {
  std::string idt(INDENT_LEN, ' ');
  for (int i = 0; i < indent; i++) {
    std::cout << idt;
  }
}
