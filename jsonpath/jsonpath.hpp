/*======================================
 * JSONPath parser
 *
 * (c)Ventura			2022
 *====================================*/

#ifndef	JSONPATH_HPP
#define	JSONPATH_HPP
#define BOOST_SPIRIT_X3_DEBUG

#include <boost/spirit/home/x3.hpp>
#include <boost/json.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/algorithm.hpp>

//#include <boost/spirit/home/x3/support/ast/variant.hpp>
//#include <boost/config/warning_disable.hpp>


namespace x3 = boost::spirit::x3;

namespace jsonpath {
   namespace ast {
      struct nodelist {
         std::vector<boost::json::value> node_list_;	// The query result
	 boost::json::value* jv;			// The value to apply the querys

	 nodelist() : jv(nullptr) {};
	 nodelist(boost::json::value* j) : jv(j) {}
/*
	 void process(std::string qs)
	 {
	    if (qs == "$") {	// root-selector
	       node_list_.push_back(jval);
	    }
	    std::cout << qs << std::endl;
	 }
*/	 
	 void process(boost::fusion::vector<std::string> qs)
	 {
	    auto parse = [&](std::string s) 
	    	{ 
		   std::cout << s << std::endl; 

		   if (s == "$") {	// root-selector
		      node_list_.push_back(*jv);
		   }
		};

	    boost::fusion::for_each(qs, parse);
	 }
      };
   }

   namespace parser {
      struct jsonpath_class;

      x3::rule<jsonpath_class, jsonpath::ast::nodelist> const jsonpath("jsonpath");

      auto go = [](auto& ctx) { _val(ctx).process(_attr(ctx)); };
      //auto go = [](auto& ctx) { };

      const auto dot_selector = x3::string(".") >> +x3::alpha;

      const auto jsonpath_def = (
	 x3::string("$") >>
	 -dot_selector
	 ) [go]
	 ;

      bool parse(std::string s, jsonpath::ast::nodelist& nd);

      BOOST_SPIRIT_DEFINE(jsonpath);
   }
}

BOOST_FUSION_ADAPT_STRUCT(
jsonpath::ast::nodelist,
(std::vector<boost::json::value>, node_list_),
);
#endif	// JSONPATH_HPP

