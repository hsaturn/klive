#pragma once
#include <core/hardware/computer.h>
#include <string>

using exprtype = int32_t;
std::string getlex(std::string& s);
bool parseExpression(std::string& s, exprtype& result, hw::Cpu* c=nullptr);
