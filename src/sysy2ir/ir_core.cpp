#include "ir_sysy2ir.h"

extern FILE* yyin;
extern int yyparse(unique_ptr<BaseAST>& ast);

namespace ir {

string sysy2ir(const char* input, const char* output, bool output2file) {
  yyin = fopen(input, "r");
  assert(yyin);

  // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);

  stringstream prt, dp;

  IRGenerator::getInstance().setting.setIndent(0).setOs(dp);

  ast->Print(cout, 0);
  ast->Dump();

  if (true) {
    cout << "Structure: \n" << prt.str() << endl;
    cout << "IR code:\n" << dp.str() << endl;
  } else {
    // 输出到文件
    ofstream outfile(output);
    if (outfile.is_open()) {
      outfile << dp.str();
      outfile.close();
    } else {
      cerr << "无法打开文件：" << output << endl;
    }
  }
  return dp.str();
}

}  // namespace ir