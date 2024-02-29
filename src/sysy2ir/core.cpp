#include "sysy2ir.h"

extern FILE* yyin;
extern int yyparse(unique_ptr<BaseAST>& ast);

string sysy2ir(const char* input,
               const char* output,
               bool output2file,
               bool output2stdout = false) {
  yyin = fopen(input, "r");
  assert(yyin);

  // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);

  stringstream ss;
  ast->Dump(ss, -1);

  if (output2stdout) {
    cout << "IR code:\n" << ss.str() << endl;
  }

  if (output2file) {
    // 输出到文件
    ofstream outfile(output);
    if (outfile.is_open()) {
      outfile << ss.str();
      outfile.close();
    } else {
      cerr << "无法打开文件：" << output << endl;
    }
  }
  return ss.str();
}