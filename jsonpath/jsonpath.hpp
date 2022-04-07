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
#include <variant>
#include <sstream>
#include <functional>

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
        typedef boost::property_tree::basic_ptree<std::string, boost::json::value> json_tree;

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


        struct f_visit {
            json_tree jtree;

            void kv(boost::json::value v) { std::cout << "value: " << boost::json::serialize(v) << std::endl; };

            void kv(bool b) { std::cout << "bool: " << b << std::endl; };
            void kv(double d) { std::cout << "double: " << d << std::endl; };
            void kv(std::int64_t i) { std::cout << "int64: " << i << std::endl; };
            void kv(std::uint64_t ui) { std::cout << "uint64: " << ui << std::endl; };
            void kv(boost::json::array a) { std::cout << "array: " << boost::json::serialize(a) << std::endl; };
            void kv(boost::json::string s) { std::cout << "string: " << s << std::endl; };
            void kv(boost::json::string_view k, boost::json::value v)
            { 
                std::cout << "object: " << k << " -> " << boost::json::serialize(v) << std::endl; 
            };
        };

        void visit(f_visit fv, boost::json::value const& jv)
        {
            if (jv.is_object()) {
                auto const& obj = jv.get_object();
                if(!obj.empty())
                {
                    // Split to process the same level
                    for(auto it = obj.begin(); it != obj.end(); ++it) {
                        fv.kv(it->key(), it->value());
                    }
                    
                    // Split to process the same level
                    for(auto it = obj.begin(); it != obj.end(); ++it) {
                        visit(fv, it->value());
                    }
                }
            } else
            if (jv.is_array()) {
                auto const& arr = jv.get_array();
                if(!arr.empty())
                {
                    fv.kv(arr);
                    // Split to process the same level
                    for(auto it = arr.begin(); it != arr.end(); ++it) {
                        fv.kv(*it);
                    }

                    // Split to process the same level
                    for(auto it = arr.begin(); it != arr.end(); ++it) {
                        visit(fv, *it);
                    }
                }
            } 
            /*
            else
            if (jv.is_string()) {
                fv.kv(jv.get_string());
            } else
            if (jv.is_int64()) {
                fv.kv(jv.get_int64());
            } else
            if (jv.is_uint64()) {
                fv.kv(jv.get_uint64());
            } else
            if (jv.is_double()) {
                fv.kv(jv.get_double());
            } else
            if (jv.is_bool()) {
                fv.kv(jv.get_bool());
            } else
            if (jv.is_null()) {
                fv.kv("null");
            }
            */
        }

        void init_json_tree(boost::json::value& jv, std::deque<json_tree>& pt)
        {
            json_tree jtree;
            jtree.put_value(jv);
            pt.push_back(jtree);
        }



        bool parse(std::string qs, boost::json::value& jv, std::vector<boost::json::value>& res)
        {
            std::string::const_iterator iter = qs.begin();
            std::string::const_iterator const end = qs.end();
            
                //std::cout << k << std::endl; 
                //std::cout << boost::json::serialize(v) << std::endl; 
            visit(f_visit(), jv);

            bool r = x3::parse(iter, end, jsonpath::parser::jsonpath, sl_);
                
            // Init property tree with initial json value
            //std::deque<boost::property_tree::ptree> pt;
            std::deque<json_tree> pt;
            init_json_tree(jv, pt);


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
                                    json_tree& jp = pt.front();
                                        
                                    for (const auto& ele : jp) {
                                        if (ele.first == tname) {
                                            pt.push_back(ele.second);
                                        }
                                    }

                                    if (pt.size() != sz) {
                                       pt.erase(pt.begin());
                                    }
                                }
                            }
                            break;

                        case selId::dotw: 
                            {
                                size_t sz = pt.size();

                                for (int i = 0; i < sz; i++) {
                                    json_tree& jp = pt.front();
                                        
                                    for (const auto& ele : jp) {
                                        pt.push_back(ele.second);
                                    }

                                    if (pt.size() != sz) {
                                       pt.erase(pt.begin());
                                    }
                                }
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
                     res.push_back(jv);
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

