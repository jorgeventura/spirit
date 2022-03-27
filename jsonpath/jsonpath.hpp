/*======================================
 * JSONPath parser
 *
 * (c)Ventura			2022
 *====================================*/

#ifndef	JSONPATH_HPP
#define	JSONPATH_HPP
//#define BOOST_SPIRIT_X3_DEBUG

#include <boost/spirit/home/x3.hpp>
#include <boost/json.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
//#include <boost/spirit/home/x3/support/ast/variant.hpp>
//#include <boost/config/warning_disable.hpp>


namespace x3 = boost::spirit::x3;

namespace jsonpath {
   namespace ast {
      struct jsonpath_ {
         std::vector<boost::json::value> node_list_;
      };
   }

   namespace parser {
      struct jsonpath_class;

      x3::rule<jsonpath_class, jsonpath::ast::jsonpath_> const jsonpath("jsonpath");

      auto foo = [](auto& ctx) { std::cout << "I am here" << std::endl; };

      auto const jsonpath_def =
         //x3::lit("$")[foo]
         x3::lit("$")
	 ;

      BOOST_SPIRIT_DEFINE(jsonpath);
   }
}

BOOST_FUSION_ADAPT_STRUCT(
jsonpath::ast::jsonpath_,
(std::vector<boost::json::value>, node_list_)
);
#endif	// JSONPATH_HPP

