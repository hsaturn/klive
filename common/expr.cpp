#include "expr.h"
#include <core/hardware/cpu.h>

using namespace std;
static hw::Cpu* cpu;	// TODO Grrrr horrible

static exprtype parseLogical(string& s);

static void trim(string& s)
{
    while(s[0]==' ') s.erase(0,1);
}

string getlex(string& s)
{
    static string alone="+-*/(){}[]^<>!?=";
    string lex;
    trim(s);
    if (s.length()==0) return "";
    char c=s[0];
    // string (not implemented)
    if (false && (c=='"' || c=='\''))
    {
        char quote=c;
        lex=c;
        s.erase(0,1);
        c=s[0];
        while(c && c!=quote)
        {
            lex += c;
            if (c==quote)
            {
                break;
            }
            else if (c=='\\' && s[1]==quote)
            {
                lex += quote;
                s.erase(0,1);
            }
            s.erase(0,1);
            c=s[0];
        }
    }
    else if (isalpha(c) || c=='_')
    {
        while(isalnum(c) || c=='_')
        {
            lex += c;
            s.erase(0,1);
            c=s[0];
        }
        if (s[0]=='\'')	// z80, last quote allowed for af' hl' ...
        {
            lex += s[0];
            s.erase(0,1);
        }
    }
    // =?
    else if (s[1]=='=' &&
             (c=='=' || c=='!' || c=='<' || c=='>'))
    {
        lex=s.substr(0,2);
        s.erase(0,2);
    }
    // char that can be doubled
    else if (c=='|' || c=='&' || c=='<' || c=='>')
    {
        s.erase(0,1);
        lex=c;
        if (s[0]==c)
        {
            s.erase(0,1);
            lex += c;
        }
    }
    else if (c=='0' && s[1]=='b' && (s[2]=='0' || s[2]=='1'))
    {
        s.erase(0,2);
        lex="0b";
        while(s[0]=='0' || s[0]=='1')
        {
            lex+=s[0];
            s.erase(0,1);
        }
    }
    else if (c=='0' && s[1]=='x' && isxdigit(s[2]))
    {
        lex="0x";
        s.erase(0,2);
        c=s[0];
        while(isxdigit(c))
        {
            lex+=c;
            s.erase(0,1);
            c=s[0];
        }
    }
    else if (isdigit(c))
    {
        while(isdigit(c))
        {
            lex += c;
            s.erase(0,1);
            c=s[0];
        }
    }
    else if (alone.find(c) != string::npos)
    {
        s.erase(0,1);
        lex=c;
    }
    trim(s);
    return lex;
}

static exprtype parseAtom(string& s)
{
    string lex=getlex(s);

    if (lex.length()>2 && lex[0]=='0' && lex[1]=='b')   // binary scalar
    {
        long int n=0;
        for(unsigned int i=2; i<lex.length(); i++)
        {
            n <<=1;
            if (lex[i]=='1') n+=1;
        }
        return static_cast<exprtype>(n);	// ...grrr
    }
    else if (isdigit(lex[0])) // hex/dec/octal
    {
        return static_cast<exprtype>(strtol(lex.c_str(), nullptr, 0));	// .... grrr
    }
    else if (isalpha(lex[0]))
    {
        if (s[0]=='(')
        {
            throw "FUNCALL NYI";
        }
        else
        {
            if (cpu)
                return cpu->regs()->get(lex);
            throw "VAR NYI";
        }
    }
    else if (lex=="(")
    {
        exprtype e=parseLogical(s);
        string clos=getlex(s);
        if (clos!=")")
        {
            s=clos+s;
            throw "Missing closing parenthesis";
        }
        return e;
    }
    return 0;
}

static exprtype parseUnary(string& s)
{
    string unary=getlex(s);
    if (unary=="-")
    {
        return -parseAtom(s);
    }
    else if (unary=="+")
    {
        return parseAtom(s);
    }
    else if (unary=="!" || unary=="not")
    {
        return not parseAtom(s);
    }
    else if (unary=="~")
    {
        return ~parseAtom(s);
    }
    else if (unary=="*")
    {
        return cpu->getMemory()->peek(parseAtom(s));
    }
    else
    {
        s=unary+' '+s;
        return parseAtom(s);
    }
}

static exprtype parseMulDiv(string& s)
{
    exprtype left = parseUnary(s);
    if (s.length()==0) return left;

    for(; s.length() ;)
    {
        string op = getlex(s);

        if (op == "*")
            left *= parseUnary(s);
        else if (op == "/")
        {
            exprtype right = parseUnary(s);
            if (right)
            {
                left /= right;
            }
            else
                throw "Div0";
        }
        else
        {
            s = op+' '+s;
            return left;
        }
    }
    return left;
}

static exprtype parseAddSub(string& s)
{
    exprtype left = parseMulDiv(s);
    if (s.length()==0) return left;

    for(;s.length();)
    {
        string op = getlex(s);

        if (op == "+")
            left += parseMulDiv(s);
        else if (op == "-")
            left -= parseMulDiv(s);
        else
        {
            s=op + ' ' + s;
            return left;
        }
    }
    return left;
}

static exprtype parseShift(string& s)
{
    exprtype left = parseAddSub(s);
    if (s.length()==0) return left;

    for(;s.length();)
    {
        string op=getlex(s);

        if (op=="<<")
            left <<= parseAddSub(s);
        else if (op == ">>")
            left >>= parseAddSub(s);
        else
        {
            s = op + ' ' + s;
            return left;
        }
    }
    return left;
}

static exprtype parseCompOps(string& s)
{
    exprtype left = parseShift(s);
    if (s.length()==0) return left;

    for(;s.length();)
    {
        string op=getlex(s);

        if (op=="<=")
            left = left <= parseShift(s);
        else if (op=="<")
            left = left < parseShift(s);
        else if (op==">=")
            left = left >= parseShift(s);
        else if (op==">")
            left = left > parseShift(s);
        else
        {
            s = op + ' ' + s;
            return left;
        }
    }
    return left;
}

static exprtype parseRelationnal(string& s)
{
    exprtype left = parseCompOps(s);
    for(;s.length();)
    {
        string op=getlex(s);

        if (op=="==" or op=="=")
            left = left == parseCompOps(s);
        else if (op=="!=")
            left = left != parseCompOps(s);
        else
        {
            s = op + ' ' + s;
            return left;
        }
    }
    return left;
}

static exprtype parseBitwise(string& s)
{
    exprtype left = parseRelationnal(s);
    for(;s.length();)
    {
        string op=getlex(s);

        if (op=="|")
            left |= parseRelationnal(s);
        else if (op=="^")
            left ^= parseRelationnal(s);
        else if (op=="&")
            left &= parseRelationnal(s);
        else
        {
            s = op + ' ' + s;
            return left;
        }
    }
    return left;
}

static exprtype parseLogical(string& s)
{
    exprtype left = parseBitwise(s);
    for(;s.length();)
    {
        string op=getlex(s);

        if (op=="||" or op=="or")
            left = left || parseBitwise(s);
        else if (op=="&&" or op=="and")
            left = left && parseBitwise(s);
        else
        {
            s = op + ' ' + s;
            return left;
        }
    }
    return left;
}

bool parseExpression(hw::Cpu* c, string& s, exprtype& result)
{
    cpu = c;	// TODO better and better .... global vars
    try
    {
        result = parseLogical(s);
        return true;
    } catch (const char* error) {
        cerr << "Gdb Parse error (" << error << ")" << endl;
    }
    return false;
}
