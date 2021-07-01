#pragma once
#include <core/hardware/computer.h>
#include <string>

using exprtype = int32_t;
using std::string;

std::string getlex(string& s);
bool parseExpression(string& s, exprtype& result, hw::Cpu* c=nullptr);
string getword(string& s, char cSep=' ');
void trim(string& s);
