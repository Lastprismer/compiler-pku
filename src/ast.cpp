#include "ast.h"

void CompUnitAST::Print(int indent) const {
  make_indent(cout, indent);
  cout << "CompUnitAST {" << endl;
  func_def->Print(indent + 1);
  make_indent(cout, indent);
  cout << " }," << endl;
}

void CompUnitAST::Dump(ostream& os, int indent) const {
  func_def->Dump(os, indent + 1);
}

void FuncDefAST::Print(int indent) const {
  make_indent(cout, indent);
  cout << "FuncDefAST {" << endl;
  func_type->Print(indent + 1);
  make_indent(cout, indent + 1);
  cout << "IDENT: \"" << ident << "\"," << endl;
  block->Print(indent + 1);
  make_indent(cout, indent);
  cout << " }," << endl;
}

void FuncDefAST::Dump(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "fun @" << ident << "(): ";
  func_type->Dump(os, indent + 1);
  os << " {" << endl;
  block->Dump(os, indent);
  make_indent(os, indent);
  os << "}" << endl;
}

void FuncTypeAST::Print(int indent) const {
  make_indent(cout, indent);
  cout << "FuncTypeAST: int," << endl;
}

void FuncTypeAST::Dump(ostream& os, int indent) const {
  os << "i32";
}

void BlockAST::Print(int indent) const {
  make_indent(cout, indent);
  cout << "BlockAST {" << endl;
  stmt->Print(indent + 1);
  make_indent(cout, indent);
  cout << " }," << endl;
}

void BlockAST::Dump(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "%"
     << "entry:" << endl;
  stmt->Dump(os, indent + 1);
}

void StmtAST::Print(int indent) const {
  make_indent(cout, indent);
  cout << "StmtAST {" << endl;
  number->Print(indent + 1);
  make_indent(cout, indent);
  cout << " }," << endl;
}

void StmtAST::Dump(ostream& os, int indent) const {
  make_indent(os, indent);
  os << "ret ";
  number->Dump(os, indent);
  os << endl;
}

void NumberAST::Print(int indent) const {
  make_indent(cout, indent);
  cout << "NumberAST {" << endl;
  make_indent(cout, indent + 1);
  cout << "int_const: " << int_const << endl;
  make_indent(cout, indent);
  cout << " }," << endl;
}

void NumberAST::Dump(ostream& os, int indent) const {
  os << int_const;
}

void make_indent(ostream& os, int indent) {
  string idt(INDENT_LEN, ' ');
  for (int i = 0; i < indent; i++) {
    os << idt;
  }
}
