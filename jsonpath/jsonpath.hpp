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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <deque>
#include <sstream>

//#include <boost/fusion/include/vector.hpp>
//#include <boost/fusion/algorithm.hpp>

//#include <boost/spirit/home/x3/support/ast/variant.hpp>
//#include <boost/config/warning_disable.hpp>


namespace x3 = boost::spirit::x3;

namespace jsonpath {
   namespace ast {

      struct nodelist {
	 boost::property_tree::ptree json_tree;		// JSON tree representation
	 std::deque<boost::property_tree::ptree> ptree_list_;

         std::deque<boost::json::value> node_list_;	// The query result

	 nodelist(boost::json::value& jv)
	 {
	    std::stringstream js;
	    js << jv;
	    std::istream is(js.rdbuf());

	    boost::property_tree::json_parser::read_json(is, json_tree);
         }


	 void root_select(const char c)
	 {
	    std::cout << "root_select" << std::endl;
	    ptree_list_.push_back(json_tree);
	 }


	 void dot_select(std::string qs)
	 {
	    std::cout << "dot_select" << std::endl;
	    size_t sz = node_list_.size();
            std::string::const_iterator iter = ++qs.begin(); // skip dot
            std::string::const_iterator const end = qs.end();
	    std::string tname(iter, end);

	    for (int i = 0; i < sz; i++) {
		boost::json::value jv = node_list_.front();
		node_list_.pop_front();

		try {
		    boost::json::value j = jv.at(tname);
		    std::cout << j << std::endl;
		} catch (std::out_of_range& e) {
		    std::cout << e.what() << std::endl;
		}

		//std::cout << jv << std::endl;
	    }
	 }
	 
	 //void select(boost::fusion::vector<char> qs)
	 //{
	 //   auto parse = [&](const auto s) 
	 //   	{ 
	 //          std::cout << "I am here 3" << std::endl; 
	 //          std::cout << s << std::endl; 

	 //          if (s == '.') {	// root-selector
	 //             //node_list_.push_back(*jv);
	 //          }
	 //       };

	 //   boost::fusion::for_each(qs, parse);
	 //   //std::cout << qs.at_c<1>(qs) << std::endl;
	 //}
      };
   }

   namespace parser {
      struct jsonpath_class;

      x3::rule<jsonpath_class, jsonpath::ast::nodelist> const jsonpath("jsonpath");

      /* Rules defined */
      x3::rule<struct root_selector, char> const root_selector_ = "root";
      x3::rule<struct dot_selector, std::string> dot_selector_ = "dot";

      auto root_action = [](auto& ctx) { _val(ctx).root_select(_attr(ctx)); };
      auto dot_action = [](auto& ctx) { _val(ctx).dot_select(_attr(ctx)); };

      const auto root_selector__def = x3::char_("$");
      const auto dot_selector__def = x3::char_(".") >> +x3::alpha;
      

      const auto jsonpath_def = 
      // Begin grammar
            root_selector_[root_action]
	>> *dot_selector_[dot_action]
      // End Gramar
	 ;

      BOOST_SPIRIT_DEFINE(root_selector_);
      BOOST_SPIRIT_DEFINE(dot_selector_);
      BOOST_SPIRIT_DEFINE(jsonpath);

      inline bool parse(std::string qs, jsonpath::ast::nodelist& nd)
      {
         std::string::const_iterator iter = qs.begin();
         std::string::const_iterator const end = qs.end();
      
      
         bool r = x3::parse(iter, end, jsonpath::parser::jsonpath, nd);
         
         if (r && (iter == end)) {
             std::cout << "-------------------------\n";
             std::cout << "Parsing succeeded\n";
             std::cout << "-------------------------\n";
         } else {
             std::cout << "-------------------------\n";
             std::cout << "Parsing failed\n";
             std::cout << "-------------------------\n";
         }
      
         return r;
      }

   }
}

BOOST_FUSION_ADAPT_STRUCT(
jsonpath::ast::nodelist,
(std::vector<boost::json::value>, node_list_)
);


#endif	// JSONPATH_HPP

