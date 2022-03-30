/*======================================
 * JSONPath parser
 *
 * (c)Ventura			2022
 *====================================*/

#include <iostream>
#include <fstream>
#include <string>
#include "jsonpath.hpp"


int main()
{
   //----------------------------------------------
   // Read input file to string
   std::string name { "test-01.json" };
   std::ifstream inp(name);
   std::string s;

   s.assign((std::istreambuf_iterator<char>(inp)),
            std::istreambuf_iterator<char>());
   //----------------------------------------------
   // JSON setup
   boost::json::error_code ec;
   boost::json::value jv = boost::json::parse(s, ec);
    
   //----------------------------------------------
   jsonpath::ast::nodelist result(jv);

   //std::string qs = { "$.book" };
   std::string qs = { "$" };

   //bool r = jsonpath::parser::parse(qs, result);

   std::cout << "Node list size: " << result.node_list_.size() << std::endl;

   return 0;
}


