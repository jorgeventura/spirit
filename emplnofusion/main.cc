#define BOOST_SPIRIT_X3_DEBUG
#include <iostream>
#include <boost/spirit/home/x3.hpp>
#include <string>
#include <vector>
#include <variant>

namespace ast {
   struct employee
   {
       int age;
       std::string gname;
       std::string sname;
       float sal;

       employee() : age(0), sal(0.0) {}

       friend std::ostream& operator<<(std::ostream& os, employee const& e)
       {
	   os << "[ " << e.age << ", \"" << e.gname << "\", \"" << e.sname << "\", " << float(e.sal) << " ]" << std::endl;
           return os;
       }


   };
}

namespace parser {
    namespace x3 = boost::spirit::x3;

    x3::rule<struct employee_class, ast::employee> const employee_ = "employee";

    x3::real_parser<float, x3::strict_real_policies<float> > float_;

    auto const quoted_string = x3::lexeme['"' >> +(x3::char_ - '"') >> '"'];

    auto assign_age   = [](auto ctx) { _val(ctx).age = _attr(ctx); };
    auto assign_gname = [](auto ctx) { _val(ctx).gname = _attr(ctx); };
    auto assign_sname = [](auto ctx) { _val(ctx).sname = _attr(ctx); };
    auto assign_sal   = [](auto ctx) { _val(ctx).sal   = _attr(ctx); };
    //auto assign_sal   = [](auto ctx) { std::cout << _attr(ctx) << std::endl; };

    const auto employee__def =
            x3::lit("employee")
            >> '{'
            >>  (x3::int_ >> ',')[assign_age]
            >>  (quoted_string >> ',')[assign_gname]
            >>  (quoted_string >> ',')[assign_sname]
            >>  float_[assign_sal]
            >>  '}'
            ;


    BOOST_SPIRIT_DEFINE(employee_)
    
    const auto entry_point = x3::skip(x3::space) [ employee_ ];

}

int main()
{
    namespace x3 = boost::spirit::x3;

    std::cout << "/////////////////////////////////////////////////////////\n\n";
    std::cout << "\t\tAn employee parser for Spirit...\n\n";
    std::cout << "/////////////////////////////////////////////////////////\n\n";

    std::cout
        << "Give me an employee of the form :"
        << "employee{age, \"forename\", \"surname\", salary } \n";
    std::cout << "Type [q or Q] to quit\n\n";

    using iterator_type = std::string::const_iterator;

    std::string str;
    while (getline(std::cin, str))
    {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        ast::employee emp;

        iterator_type iter = str.begin();
        iterator_type const end = str.end();

        bool r = phrase_parse(iter, end, parser::entry_point, x3::space, emp);

        if (r && iter == end)
        {

            //std::cout << boost::fusion::tuple_open('[');
            //std::cout << boost::fusion::tuple_close(']');
            //std::cout << boost::fusion::tuple_delimiter(", ");

            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded\n";
            //std::cout << "got: " << emp << std::endl;
            std::cout << "\n-------------------------\n";
        }
        else
        {
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed\n";
            std::cout << "-------------------------\n";
        }
    }

    std::cout << "Bye... :-) \n\n";
    return 0;
}


