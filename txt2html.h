#ifndef ___YING_TXT_2_HTML__
#define ___YING_TXT_2_HTML__
#include <string>
#include <sstream>

class YTxt2Html
{
public:
    YTxt2Html();

public:
    void prepare();
    void append(const std::string& in);
    const std::string finish();

protected:
    std::ostringstream ostr;
};
#endif /* ___YING_TXT_2_HTML__ */
