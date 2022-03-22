#include <boost/spirit/home/x3.hpp>
#include <iostream>
#include <vector>
#include <variant>

namespace x3 = boost::spirit::x3;
int main()
{
    std::string input("G,56, c ,89,3  |    T");
    std::string::iterator it = input.begin();

    using var_t = std::variant<int, char>;

    std::vector<var_t> vv;

    //auto Fi = [&vv](int i) { vv.push_back(var_t(i)); };
    //auto Fc = [&vv](char c) { vv.push_back(var_t(c)); };
    //auto Fs = [&vv](std::string s) { vv.push_back(var_t(s)); };
    
    auto Fi = [&vv](auto& ctx) { vv.push_back(x3::_attr(ctx)); };
    auto Fc = [&vv](auto& ctx) { vv.push_back(x3::_attr(ctx)); };
    
    auto r0 = x3::int_[Fi] | x3::alpha[Fc];
    auto sep = *x3::blank >> (x3::char_(',') | '|') >> *x3::blank;
    auto r1 = r0 % sep;

    if (x3::parse(it, input.end(), r1))
    {
        std::cout << vv.size() << " elements parsed\n";
    }

    for (var_t v : vv) {
        std::visit([](auto arg) { std::cout << arg << std::endl; }, v);
    }
}


