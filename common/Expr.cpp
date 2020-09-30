#include <iostream>
#include <string>
#include <ctype.h>

using namespace std;
using intype=int16_t;

void trim(std::string& s)
{
    while(isblank(s[0])) s.erase(0,1);
}

string getlex(string& s)
{
    static string alone="+-*/(){}[]^<>!";
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


intype parseAtom(string& s)
{
    string lex=getlex(s);

    if (lex.length()>2 && lex[0]=='0' && lex[1]=='b')   // binary number
    {
        long int n=0;
        long int d=1;
        for(unsigned int i=2; i<lex.length(); i++)
        {
            n <<=1;
            if (lex[i]=='1') n+=1;
        }
        return n;
    }
    else if (isdigit(lex[0])) // hex/dec/octal
    {
        return strtol(lex.c_str(), nullptr, 0);
    }
    else if (isalpha(lex[0]))
    {
        cerr << "ERR NOT IMPLEMENTED (" << lex << ' ' << s << ")";
        if (s[0]=='(')
        {
            cerr << "FUNCALL ";
        }
        else
        {
            cerr << "VAR";
        }
        cerr << endl;
    }
    return 0;
}

intype parseUnary(string& s)
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
        cerr << "PEEK" << endl;
        return -1;  // TODO peek
    }
    else
    {
        s=unary+' '+s;
        return parseAtom(s);
    }
}

intype parseMulDiv(string& s)
{
    intype left = parseUnary(s);
    if (s.length()==0) return left;

    for(; s.length() ;)
    {
        string op = getlex(s);

        if (op == "*")
            left *= parseUnary(s);
        else if (op == "/")
        {
            intype right = parseUnary(s);
            if (right)
            {
                left /= right;
            }
            else
                cerr << "Div0 " << __LINE__ << endl;
        }
        else
        {
            s = op+' '+s;
            return left;
        }
    }
    return left;
}

intype parseAddSub(string& s)
{
    intype left = parseMulDiv(s);
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

intype parseShift(string& s)
{
    intype left = parseAddSub(s);
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

intype parseCompOps(string& s)
{
    intype left = parseShift(s);
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

intype parseRelationnal(string& s)
{
    intype left = parseCompOps(s);
    for(;s.length();)
    {
        string op=getlex(s);

        if (op=="==")
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


intype parseBitwise(string& s)
{
    intype left = parseRelationnal(s);
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

intype parseLogical(string& s)
{
    intype left = parseBitwise(s);
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

intype parseExpression(string& s)
{
    return parseLogical(s);
}

int main(int argc, const char* argv[])
{
    for(int arg=1; arg<argc; arg++)
    {
        string s1(argv[arg]);
        cout << "--------------[ " << s1 << " ]----------" << endl;
        string s=s1;
        cout <<"Lexemes : ";
        string lex;
        while((lex=getlex(s)).length())
        {
            cout << lex << ' ';
        }
        cout << endl;
        int result = parseExpression(s1);
        cout << "Value   : " << result;
        if (s1.length()) cout << ", chars left (" << s1 << ")" << endl;
        cout << endl;
    }


    return 0;
}

