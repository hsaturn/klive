#pragma once

// A scope var is a var that'll revert to its initial value
// when going out of the scope. Same idea as lock_guard
template<class Type>
class ScopeVar
{
public:
    ScopeVar(Type& var) : var_(var), initial_(var) {}
    ~ScopeVar() { var_ = initial_; }

    operator Type() { return var_; }

    Type& operator++() { var_++; return var_; }
    Type operator++(int) { Type old=var_; operator ++(); return old; }
    Type& operator--() { var_--; return var_; }
    Type operator--(int) { Type old=var_; operator --(); return old; }

private:
    Type& var_;
    Type initial_;
};
