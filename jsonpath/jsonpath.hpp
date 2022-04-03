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
                dotw,           // '.*'
                idxs,           // "[" S (quoted-member-name | element-index) S "]"
                idxw,           // "[" "*" "]"
                lsts,
                slice,
                desc,
                filter
            };

            struct sel {
                selId id;
                std::string element;
    
                sel(selId i, std::string ele) : id(i), element(ele) {}
                sel(selId i, const char ele) : id(i), element(std::string(1,ele)) {}
            };

            struct selList {
                std::vector<jsonpath::ast::selector::sel> slist;
            };
        
            inline std::string full_selector(selector::selList& sl)
            {
                std::string fsel;
                for (const auto& sel : sl.slist) {
                    fsel += sel.element;
                }
                return fsel;
            }

        }
    }

    namespace parser {
        namespace selector = jsonpath::ast::selector;

        struct jsonpath_class;
        // List of selectors to apply in the jv input
        selector::selList sl_;

        // Main rule
        //x3::rule<jsonpath_class, jsonpath::ast::selector::sel> const jsonpath("jsonpath");
        x3::rule<jsonpath_class, selector::selList> const jsonpath("jsonpath");

        /* Rules defined */
        x3::rule<struct root_selector, char> const root_selector_ = "root";
        x3::rule<struct dot_selector, std::string> dot_selector_ = "dot";
        x3::rule<struct dotw_selector, std::string> dotw_selector_ = "dotw";
        x3::rule<struct idxs_selector, std::string> idxs_selector_ = "idxs";
        x3::rule<struct idxw_selector, std::string> idxw_selector_ = "idxw";
        x3::rule<struct lsts_selector, std::string> lsts_selector_ = "lsts";

        auto root_action = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::root, _attr(ctx))); };
        auto dot_action  = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::dot, _attr(ctx))); };
        auto dotw_action = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::dotw, _attr(ctx))); };
        auto idxs_action = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::idxs, _attr(ctx))); };
        auto idxw_action = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::idxw, _attr(ctx))); };
        auto lsts_action = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::lsts, _attr(ctx))); };

        const auto root_selector__def = x3::char_("$");
        const auto dot_selector__def = x3::char_(".") >> +x3::alpha;
        const auto dotw_selector__def = x3::char_(".") >> x3::char_("*");
        const auto idxs_selector__def = x3::char_("[") >> (+x3::alpha | +x3::digit) >> x3::char_("]");
        const auto idxw_selector__def = x3::char_("[") >> x3::char_("*") >> x3::char_("]");
        const auto lsts_selector__def = x3::lit("end");

        const auto jsonpath_def = 
            // Begin grammar
            root_selector_[root_action]
            >> *(
                    dot_selector_[dot_action]           |
                    dotw_selector_[dotw_action]         |
                    idxs_selector_[idxs_action]         |
                    idxw_selector_[idxw_action]         |
                    lsts_selector_[lsts_action]
                )
            // End Gramar
            ;

        BOOST_SPIRIT_DEFINE(root_selector_);
        BOOST_SPIRIT_DEFINE(dot_selector_);
        BOOST_SPIRIT_DEFINE(dotw_selector_);
        BOOST_SPIRIT_DEFINE(idxs_selector_);
        BOOST_SPIRIT_DEFINE(idxw_selector_);
        BOOST_SPIRIT_DEFINE(lsts_selector_);
        BOOST_SPIRIT_DEFINE(jsonpath);
            
        inline void init_ptree(boost::json::value& jv, std::deque<boost::property_tree::ptree>& pt)
        {
            std::stringstream js;
            boost::property_tree::ptree jt;

            js << jv;
            std::istream is(js.rdbuf());

            boost::property_tree::json_parser::read_json(is, jt);
            pt.push_back(jt);
        }

        inline bool parse(std::string qs, boost::json::value& jv, std::vector<boost::json::value>& res)
        {
            std::string::const_iterator iter = qs.begin();
            std::string::const_iterator const end = qs.end();

            bool r = x3::parse(iter, end, jsonpath::parser::jsonpath, sl_);

            if (r && (iter == end)) {
                // Init property tree with initial json value
                std::deque<boost::property_tree::ptree> pt;
                init_ptree(jv, pt);

                // eval the jsonpath
                for (const auto& s : sl_.slist) {
                    std::cout << s.id << std::endl;
                }

                /*
                   for (auto& jt : nd.ptree_list_) {
                // Convert from json tree element to json string
                std::stringstream js;
                boost::property_tree::json_parser::write_json(js, jt);

                // Convert from json string to jason value
                boost::json::error_code ec;
                boost::json::value jv = boost::json::parse(js.str(), ec);

                // Add to nodelist
                nd.node_list_.push_back(jv);
                }*/

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
jsonpath::ast::selector::selList,
(std::vector<jsonpath::ast::selector::sel>, slist)
);


#endif	// JSONPATH_HPP

/*
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
*/
