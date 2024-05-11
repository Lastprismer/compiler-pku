#pragma once

#include <iostream>
using namespace std;

// 输出相关信息
struct GenSettings {
  // 输出流
  ostream* os;
  // 当前缩进
  int indent;
  // 当前只有return后面的表达式是实际需要写入的
  bool shouldWriting;

  GenSettings();
  GenSettings& setOs(ostream& o);
  GenSettings& setIndent(int val);
  int& getIndent();
  ostream& getOs();
  const string getIndentStr();
};