//#define BOOST_SPIRIT_X3_DEBUG
#include <iostream>
//#include <boost/fusion/adapted.hpp>
#include <boost/spirit/home/x3.hpp>
#include <string>
#include <vector>
#include <variant>

struct value : std::variant<int,float,std::vector<value>>
{
    using base_type = std::variant<int,float,std::vector<value>>;
    using base_type::variant;

    friend std::ostream& operator<<(std::ostream& os, base_type const& v) {
        struct {
            std::ostream& operator()(float const& f) const { return _os << "float:" << f; }
            std::ostream& operator()(int const& i)   const { return _os << "int:" << i; }
            std::ostream& operator()(std::vector<value> const& v) const {
                _os << "tuple: [";
                for (auto& el : v) _os << el << ",";
                return _os << ']';
            }
            std::ostream& _os;
        } vis { os };

        return std::visit(vis, v);
    }
};

namespace parser {
    namespace x3 = boost::spirit::x3;

    x3::rule<struct value_class, value> const value_ = "value";
    x3::rule<struct o_tuple_class, std::vector<value> > o_tuple_ = "tuple";

    x3::real_parser<float, x3::strict_real_policies<float> > float_;

    const auto o_tuple__def = "tuple" >> x3::lit(':') >> ("[" >> value_ % "," >> "]");

    const auto value__def
        = "float" >> (':' >> float_)
        | "int" >> (':' >> x3::int_)
        | o_tuple_
        ;

    BOOST_SPIRIT_DEFINE(value_, o_tuple_)

    const auto entry_point = x3::skip(x3::space) [ value_ ];
}

int main()
{
    for (std::string const str : {
            "float: 3.14",
            "int: 3",
            "tuple: [float: 3.14,int: 3]",
            "tuple: [float: 3.14,int: 3,tuple: [float: 4.14,int: 4]]"
    }) {
        std::cout << "============ '" << str << "'\n";

        //using boost::spirit::x3::parse;
        auto first = str.begin(), last = str.end();
        value val;

        if (parse(first, last, parser::entry_point, val))
            std::cout << "Parsed '" << val << "'\n";
        else
            std::cout << "Parse failed\n";

        if (first != last)
            std::cout << "Remaining input: '" << std::string(first, last) << "'\n";
    }
}


