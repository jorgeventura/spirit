
///////////////////////////////////////////////////////////////////////////////
// Version without pack symbols and parser 
// Ventura, J. 
///////////////////////////////////////////////////////////////////////////////
#include <boost/spirit/home/x3.hpp>

#include <iostream>
#include <string>

namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;
///////////////////////////////////////////////////////////////////////////////
//  Parse roman hundreds (100..900) numerals using the symbol table.
//  Notice that the data associated with each slot is the parser's attribute
//  (which is passed to attached semantic actions).
///////////////////////////////////////////////////////////////////////////////
struct hundreds_ : x3::symbols<unsigned>
{
    hundreds_()
    {
        add
            ("C"    , 100)
            ("CC"   , 200)
            ("CCC"  , 300)
            ("CD"   , 400)
            ("D"    , 500)
            ("DC"   , 600)
            ("DCC"  , 700)
            ("DCCC" , 800)
            ("CM"   , 900)
        ;
    }
} hundreds;
///////////////////////////////////////////////////////////////////////////////
//  Parse roman tens (10..90) numerals using the symbol table.
///////////////////////////////////////////////////////////////////////////////
struct tens_ : x3::symbols<unsigned>
{
    tens_()
    {
        add
            ("X"    , 10)
            ("XX"   , 20)
            ("XXX"  , 30)
            ("XL"   , 40)
            ("L"    , 50)
            ("LX"   , 60)
            ("LXX"  , 70)
            ("LXXX" , 80)
            ("XC"   , 90)
        ;
    }
} tens;
///////////////////////////////////////////////////////////////////////////////
//  Parse roman ones (1..9) numerals using the symbol table.
///////////////////////////////////////////////////////////////////////////////
struct ones_ : x3::symbols<unsigned>
{
    ones_()
    {
        add
            ("I"    , 1)
            ("II"   , 2)
            ("III"  , 3)
            ("IV"   , 4)
            ("V"    , 5)
            ("VI"   , 6)
            ("VII"  , 7)
            ("VIII" , 8)
            ("IX"   , 9)
        ;
    }
} ones;

    

///////////////////////////////////////////////////////////////////////////////
//  Main program
///////////////////////////////////////////////////////////////////////////////
int
main()
{
    std::cout << "/////////////////////////////////////////////////////////\n\n";
    std::cout << "\t\tRoman Numerals Parser\n\n";
    std::cout << "/////////////////////////////////////////////////////////\n\n";
    std::cout << "Type a Roman Numeral ...or [q or Q] to quit\n\n";

    auto set_zero = [](auto& ctx){ _val(ctx) = 0; };

    auto add1000 = [](auto& ctx){ _val(ctx) += 1000; };
    auto add = [](auto& ctx){ _val(ctx) += _attr(ctx); };
    
    typedef std::string::const_iterator iterator_type;

    /* 
     * The declaration below is assigning a variable with type
     * because the compiler deduction is insufficient
     */
    auto const roman_p = x3::rule<class _, uint> {} = x3::eps[set_zero]
        >>
        (
            -(+x3::lit('M')[add1000])
            >>  -hundreds   [add]
            >>  -tens       [add]
            >>  -ones       [add]
        )
    ;

    std::string str;
    unsigned result;
    while (std::getline(std::cin, str))
    {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        iterator_type iter = str.begin();
        iterator_type const end = str.end();
        bool r = parse(iter, end, roman_p, result);

        if (r && iter == end)
        {
            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded\n";
            std::cout << "result = " << result << std::endl;
            std::cout << "-------------------------\n";
        }
        else
        {
            std::string rest(iter, end);
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed\n";
            std::cout << "stopped at: \": " << rest << "\"\n";
            std::cout << "-------------------------\n";
        }
    }

    std::cout << "Bye... :-) \n\n";
    return 0;
}
