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
   std::vector<boost::json::value> result;

   //std::string qs = { "$" };
   std::string qs = { "$.store.books.*.test" };

   // qs = jsonpath query string
   // jv = json object for the query
   // result = std::vector with node results
   bool r = jsonpath::parser::parse(qs, jv, result);

   std::cout << "Node list size: " << result.size() << std::endl;
   //std::cout << "Selector list size: " << jsonpath::ast::selector::sel_list_.size() << std::endl;
   /*
   std::cout << jsonpath::ast::selector::full_selector() << std::endl;

   for (auto& jv : result) {
       std::cout << jv << std::endl;
   }
   */

   return 0;
}


