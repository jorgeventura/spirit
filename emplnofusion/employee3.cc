#define BOOST_SPIRIT_X3_DEBUG
#include <iostream>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <string>
#include <vector>
#include <variant>

namespace x3 = boost::spirit::x3;

namespace ast {
   struct employee
   {
       std::vector<int> age;

       employee() {}

       friend std::ostream& operator<<(std::ostream& os, employee const& e)
       {
	   os << "[ ";
	   for (const int& i : e.age) {
	       os << i << ", ";
	   }
	   os << " ]" << std::endl;
           return os;
       }


   };
       
}

BOOST_FUSION_ADAPT_STRUCT(ast::employee, age);


namespace parser {
    namespace x3 = boost::spirit::x3;

    x3::rule<struct employee_class, ast::employee> const employee_ = "employee";

    const auto employee__def =
            x3::lit("employee")
            >> '{'
            >>  x3::int_ % x3::char_(',')
            >> '}'
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
            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded\n";
            std::cout << "got: " << emp << std::endl;
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


