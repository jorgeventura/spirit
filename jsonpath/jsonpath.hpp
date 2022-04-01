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

        namespace selector {
            enum selId {
                root = 0,       // '$'
                dot,            // '.' or '.name'
                dot_wild,       // '.*'
            };
            
            struct sel {
                selId id;
                std::string element;
                sel(selId i, std::string ele) : id(i), element(ele) {}
                sel(selId i, const char ele) : id(i), element(std::string(1,ele)) {}
            };


            std::vector<jsonpath::ast::selector::sel> sel_list_;
            
            std::string full_selector()
            {
                std::string fsel;
                for (const auto& sel : sel_list_) {
                    fsel += sel.element;
                }
                return fsel;
            }
        }

        struct nodelist {
            std::deque<boost::property_tree::ptree> ptree_list_;

            std::deque<boost::json::value> node_list_;	// The query result

            nodelist(boost::json::value& jv)
            {
                std::stringstream js;
                boost::property_tree::ptree jt;
                js << jv;
                std::istream is(js.rdbuf());

                boost::property_tree::json_parser::read_json(is, jt);
                ptree_list_.push_back(jt);
            }


            void root_select(const char c)
            {
                std::cout << "root_select" << std::endl;
            }


            void dot_select(std::string qs)
            {
                std::cout << "dot_select" << std::endl;
                size_t sz = ptree_list_.size();

                std::string tname(++qs.begin(), qs.end());

                for (int i = 0; i < sz; i++) {
                    boost::property_tree::ptree& jp = ptree_list_.front();

                    for (const auto& ele : jp) {
                        if (ele.first == tname) {
                           ptree_list_.push_back(ele.second);
                        }
                    }

                    ptree_list_.erase(ptree_list_.begin());
                }
            }
        };
    }

    namespace parser {
        namespace selector = jsonpath::ast::selector;
        
        struct jsonpath_class;

        x3::rule<jsonpath_class, jsonpath::ast::nodelist> const jsonpath("jsonpath");

        /* Rules defined */
        x3::rule<struct root_selector, char> const root_selector_ = "root";
        x3::rule<struct dot_selector, std::string> dot_selector_ = "dot";

        auto root_action = [](auto& ctx) { selector::sel_list_.push_back(selector::sel(selector::selId::root, _attr(ctx))); };
        auto dot_action = [](auto& ctx) { selector::sel_list_.push_back(selector::sel(selector::selId::dot, _attr(ctx))); };

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
                for (auto& jt : nd.ptree_list_) {
                    // Convert from json tree element to json string
                    std::stringstream js;
                    boost::property_tree::json_parser::write_json(js, jt);

                    // Convert from json string to jason value
                    boost::json::error_code ec;
                    boost::json::value jv = boost::json::parse(js.str(), ec);

                    // Add to nodelist
                    nd.node_list_.push_back(jv);
                }

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

