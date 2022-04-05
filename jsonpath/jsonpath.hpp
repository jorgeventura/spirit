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
        x3::rule<struct slice_selector, std::string> slice_selector_ = "slice";
        x3::rule<struct desc_selector, std::string> desc_selector_ = "desc";
        x3::rule<struct filter_selector, std::string> filter_selector_ = "filter";

        auto root_action    = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::root, _attr(ctx))); };
        auto dot_action     = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::dot, _attr(ctx))); };
        auto dotw_action    = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::dotw, _attr(ctx))); };
        auto idxs_action    = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::idxs, _attr(ctx))); };
        auto idxw_action    = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::idxw, _attr(ctx))); };
        auto lsts_action    = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::lsts, _attr(ctx))); };
        auto slice_action   = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::slice, _attr(ctx))); };
        auto desc_action    = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::desc, _attr(ctx))); };
        auto filter_action  = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::filter, _attr(ctx))); };

        // root-selector
        const auto root_selector__def = x3::char_("$");
        // dot-selector
        const auto dot_selector__def = x3::char_(".") >> +x3::alpha;
        // dot-wild-selector
        const auto dotw_selector__def = x3::char_(".") >> x3::char_("*");
        // index-selector
        const auto idxs_selector__def = x3::char_("[") >> (+x3::alpha | +x3::digit) >> x3::char_("]");
        // index-wild-selector
        const auto idxw_selector__def = x3::char_("[") >> x3::char_("*") >> x3::char_("]");
        // list-selector
        const auto lsts_selector__def = x3::lit("end");
        // slice-selector
        const auto slice_selector__def = x3::lit("end");
        // descendant-selector
        const auto desc_selector__def = x3::lit("end");
        // filter-selector
        const auto filter_selector__def = x3::lit("end");

        const auto jsonpath_def = 
            // Begin grammar
            root_selector_[root_action]
            >> *(
                    dot_selector_[dot_action]       |
                    dotw_selector_[dotw_action]     |
                    idxs_selector_[idxs_action]     |
                    idxw_selector_[idxw_action]     |
                    lsts_selector_[lsts_action]     |
                    slice_selector_[slice_action]   |
                    desc_selector_[desc_action]     |
                    filter_selector_[lsts_action]
                )
            // End Gramar
            ;

        BOOST_SPIRIT_DEFINE(root_selector_);
        BOOST_SPIRIT_DEFINE(dot_selector_);
        BOOST_SPIRIT_DEFINE(dotw_selector_);
        BOOST_SPIRIT_DEFINE(idxs_selector_);
        BOOST_SPIRIT_DEFINE(idxw_selector_);
        BOOST_SPIRIT_DEFINE(lsts_selector_);
        BOOST_SPIRIT_DEFINE(slice_selector_);
        BOOST_SPIRIT_DEFINE(desc_selector_);
        BOOST_SPIRIT_DEFINE(filter_selector_);
        BOOST_SPIRIT_DEFINE(jsonpath);
            
        void init_ptree(boost::json::value& jv, std::deque<boost::property_tree::ptree>& pt)
        {
            std::stringstream js;
            boost::property_tree::ptree jt;

            js << jv;
            std::istream is(js.rdbuf());

            boost::property_tree::json_parser::read_json(is, jt);
            pt.push_back(jt);
        }

        bool parse(std::string qs, boost::json::value& jv, std::vector<boost::json::value>& res)
        {
            std::string::const_iterator iter = qs.begin();
            std::string::const_iterator const end = qs.end();

            bool r = x3::parse(iter, end, jsonpath::parser::jsonpath, sl_);
                
            // Init property tree with initial json value
            std::deque<boost::property_tree::ptree> pt;
            init_ptree(jv, pt);

            if (r && (iter == end)) {
                namespace selId = jsonpath::ast::selector;

                // eval the jsonpath
                for (const auto& s : sl_.slist) {

                    switch (s.id) {
                        case selId::root: 
                            break;

                        case selId::dot:
                            {
                                size_t sz = pt.size();
                                std::string::const_iterator it = ++(s.element.cbegin());
                                std::string tname(it, s.element.end());

                                for (int i = 0; i < sz; i++) {
                                    boost::property_tree::ptree& jp = pt.front();
                                    bool is_item = false;

                                    for (const auto& ele : jp) {
                                        if (ele.first == tname) {
                                            // The element is a vector
                                            if (ele.second.size() > 2) {
                                                for (const auto& e : ele.second) {
                                                    pt.push_back(e.second);
                                                }
                                            } else {
                                                // Check if the element is an item
                                                if (ele.second.size() > 0) {
                                                   pt.push_back(ele.second);
                                                } else {
                                                   // Create new json obj { item: value }
                                                   boost::property_tree::ptree jpv;
                                                   jpv.put(ele.first, ele.second.data());
                                                   pt.push_back(jpv);
                                                }
                                            }
                                        }
                                    }

                                    if (pt.size() > sz) {
                                       pt.erase(pt.begin());
                                    }
                                }
                            }
                            break;

                        case selId::dotw: 
                            {
                            }
                            break;

                        case selId::idxs:
                            {
                            }
                            break;

                        case selId::idxw:
                            {
                            }
                            break;

                        case selId::lsts:
                            {
                            }
                            break;

                        case selId::slice:
                            {
                            }
                            break;

                        case selId::desc:
                            {
                            }
                            break;

                        case selId::filter:
                            {
                            }
                            break;
                    }
                }


                for (const auto& jt : pt) {
                    // Convert from json tree element to json string
                    if (jt.size() > 0) {
                        std::stringstream js;
                        boost::property_tree::json_parser::write_json(js, jt);

                        // Convert from json string to jason value
                        boost::json::error_code ec;
                        boost::json::value jv = boost::json::parse(js.str(), ec);

                        // Add to result node list (vector)
                        res.push_back(jv);
                    } else {

                        std::cout << jt.size() << std::endl;
                        for (const auto& e : jt) {
                            std::cout << e.first << std::endl;
                        }
                        //res.push_back(jt.second);
                    }
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
