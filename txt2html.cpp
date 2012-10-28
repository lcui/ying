#include "txt2html.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <ctype.h>

#define _TABSIZE    1

using namespace std;

int tabsize = _TABSIZE;
static string html(const string& s);

class token {
public:
    token(ostringstream& ostr) : _what(code), ostr(ostr), new_line(true) {}
protected:
    enum type {code, comment, pp, keyword, diff_plus, diff_minus};
    string _str;
    type _what;
    ostringstream& ostr;
    bool new_line;
    friend istream& operator>>(istream&, token&);
    friend ostream& operator<<(ostream&, const token&);
    friend class YTxt2Html;
};

static bool iskeyword(const string& s)
{
    static const char* keywords[] = {
        "and",
        "and_eq",
        "asm",
        "auto",
        "bitand",
        "bitor",
        "bool",
        "break",
        "case",
        "catch",
        "char",
        "class",
        "compl",
        "const",
        "const_cast",
        "continue",
        "default",
        "delete",
        "do",
        "double",
        "dynamic_cast",
        "else",
        "enum",
        "explicit",
        "export",
        "extern",
        "false",
        "float",
        "for",
        "friend",
        "goto",
        "if",
        "inline",
        "int",
        "long",
        "mutable",
        "namespace",
        "new",
        "not",
        "not_eq",
        "operator",
        "or",
        "or_eq",
        "private",
        "protected",
        "public",
        "register",
        "reinterpret_cast",
        "return",
        "short",
        "signed",
        "sizeof",
        "static",
        "static_cast",
        "struct",
        "switch",
        "template",
        "this",
        "throw",
        "true",
        "try",
        "typedef",
        "typeid",
        "typename",
        "union",
        "unsigned",
        "using",
        "virtual",
        "void",
        "volatile",
        "wchar_t",
        "while",
        "xor",
        "xor_eq"
    };

    for (int i = 0; i < sizeof(keywords) / sizeof(char*); i++)
        if (string(keywords[i]) == s)
            return true;

    return false;
}

static bool containspp(const string& s)
{
    static const char* pptokens[] = {
        "define",
        "elif",
        "else",
        "endif",
        "error",
        "if",
        "ifdef",
        "ifndef",
        "include",
        "line",
        "pragma",
        "undef"
    };

    for (int i = 0; i < sizeof(pptokens) / sizeof(char*); i++)
        if (s.find(pptokens[i]) != string::npos)
            return true;

    return false;
}
ostream& operator<<(ostream& os, const token& t)
{
    if (t._what == token::code)
        t.ostr << html(t._str);
    else if (t._what == token::comment)
        t.ostr << "<span class=comment>" << html(t._str) << "</span>";
    else if (t._what == token::keyword)
        t.ostr << "<span class=keyword>" << html(t._str) << "</span>";
    else if (t._what == token::pp)
        t.ostr << "<span class=pp>" << html(t._str) << "</span>";
    else if (t._what == token::diff_plus)
        t.ostr << "<span class=plus>" << html(t._str) << "</span>";
    else if (t._what == token::diff_minus)
        t.ostr << "<span class=minus>" << html(t._str) << "</span>";
    else
        t.ostr << html(t._str);
    return os;
}

static string html(const string& s)
{
    string s1;
    string::size_type i;
    for (i = 0; i < s.length(); i++) {
        switch (s[i]) {
        case '&':
            s1 += "&amp;";
            break;
        case '<':
            s1 += "&lt;";
            break;
        case '>':
            s1 += "&gt;";
            break;
        case '"':
            s1 += "&quot;";
            break;
        case '\n':
            s1 += "<br>";
            break;
        case '\t':
            s1.append(tabsize, ' ');
            break;
        default:
            s1 += s[i];
        }
    }
    return s1;
}
istream& operator>>(istream& is, token& t)
{
    t._str = "", t._what = token::code;
    int c = is.get();
    switch (c) {
    case '/':
        c = is.get();
        if (c == '*') {
            t._str = "/*";
            t._what = token::comment;
            while (1) {
                c = is.get();
                if (c == EOF)
                    return is.unget(), is.clear(), is;
                if (c == '/') {
                    if (t._str.length() > 2 &&
                        t._str[t._str.length() - 1] == '*') {
                        return t._str += '/', is;
                    }
                }
                t._str += (char)c;
            }
        } else if (c == '/') {
            t._str = "//";
            t._what = token::comment;
            c = is.get();
            while (c != '\n' && c != EOF) {
                t._str += (char)c;
                c = is.get();
            }
            if (c == '\n') {
                t._str += '\n';
            }
            return is;
        }
        t._str = '/';
        return is.unget(), is.clear(), is;
    case '#':
        t._str = '#';
        c = is.get();
        while (strchr(" \r\n\t", c)) {
            t._str += (char)c;
            c = is.get();
        }
        if (c == EOF)
            return is.unget(), is.clear(), is;
        while (strchr("abcdefghijklmnopqrstuvwxyz", c)) {
            t._str += (char)c;
            c = is.get();
        }
        is.unget(), is.clear();
        if (containspp(t._str))
            t._what = token::pp;
        return is;

    case '\'':
    case '"': {
        char q = (char)c;
        t._str = q;
        while (1) {
            c = is.get();
            if (c == EOF)
                return is.unget(), is.clear(), is;
            if (c == q) {
                if (t._str.length() >= 2) {
                    if (!(t._str[t._str.length() - 1] == '\\' &&
                          t._str[t._str.length() - 2] != '\\'))
                        return t._str += q, is;
                } else {
                    return t._str += q, is;
                }
            }
            t._str += (char)c;                
        }
              }
    case '\n':
        t.new_line = true;
        t._str += (char)c;
        return is;

    case '+':
    case '-':
        t._str += (char)c;
        if (t.new_line) {
            if (c=='+') {
                t._what = token::diff_plus;
            } else {
                t._what = token::diff_minus;
            }
            do {
                c = is.get();
                t._str += (char)c;                
            } while(c != '\n');
        }

        return is;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'i':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
        t._str += (char)c;
        c = is.get();
        while (isalpha(c) || isdigit(c) || c == '_') {
            t._str += (char)c;
            c = is.get();
        }
        is.unget(), is.clear();
        if (iskeyword(t._str))
            t._what = token::keyword;
        return is;
    case EOF:
        return is;
    default:
        t._str += (char)c;
        c = is.get();
        while (c != '/' && c != '#' && !strchr("abcdefgilmnoprstuvwx", c) &&
               c != '\'' && c != '"' && c != EOF) {
            t._str += (char)c;
            c = is.get();
        }
        is.unget(), is.clear();
        return is;
    }
}

YTxt2Html::YTxt2Html() :
    ostr("")
{
}

void YTxt2Html::prepare()
{
    tabsize = _TABSIZE;
    ostr << "<html>" << endl 
        << "<head>" << endl 
        << "<style>" << endl;
    ostr << ".keyword{color:rgb(0,0,255);}" << endl;
    ostr << ".comment{color:rgb(0,128,0);}" << endl;
    ostr << ".plus{color:rgb(0,128,0);}" << endl;
    ostr << ".minus{color:rgb(128,0,0);}" << endl;
    ostr << ".pp{color:rgb(0,0,255);}" << endl;
    ostr << "</style>" << endl << "<body>" << endl;
    ostr << "<pre style=\"font-family:courier;font-size:8pt\">";
}

void YTxt2Html::append(const string& in)
{
    istringstream istr(in);
    token t(ostr);
    while (istr >> t) {
        ostr << t;
        t.new_line = false;
    }
}

const string YTxt2Html::finish()
{
    ostr << "</pre>" << "</body>" 
        << endl << "</html>" << endl;

    return ostr.str();
}

#if defined(MYMAIN)
int main(int argc, char **argv)
{
    string output;
    tabsize = _TABSIZE;
    txt2html("int a = 0;", output);
    cout << output;
    return 0;
}
#endif
