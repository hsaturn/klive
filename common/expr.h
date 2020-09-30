#pragma once
#include <core/hardware/computer.h>
#include <string>

using exprtype = int32_t;
std::string getlex(std::string& s);
bool parseExpression(hw::Cpu* c, std::string& s, exprtype& result);
