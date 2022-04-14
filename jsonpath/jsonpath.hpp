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
//#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/json_parser.hpp>

#include <deque>
#include <variant>
#include <sstream>
#include <string_view>
#include <algorithm>

#define DD  std::cout << "I am here" << std::endl

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
                idxw,           // "[" "*" "]"
                indx,           // "[" S (quoted-member-name | element-index) S "]"
                slice,          // "[" i ":" j ":" "k" "]"
                lsts,
                desc,
                filter
            };

            struct sel {
                selId id;
                std::string element;
                std::vector<int> indx;

                sel(selId i, std::string ele) : id(i), element(ele)
                { std::cout << id << ": sel(selId i, std::string ele) " << ele << std::endl; }
                
                sel(selId i, const char ele) : id(i), element(std::string(1,ele))
                { std::cout << id << ": sel(selId i, const char ele)" << std::endl; }
                
                sel(selId i, int j) : id(i)
                { 
                    std::cout << id << ": sel(selId i, int j)" << std::endl;
                    indx.push_back(j);

                }
                
                sel(selId i, boost::variant<std::string, int> ele) : id(i)
                {
                    std::cout << id << ": sel(selId i, boost::variant<std::string, int> ele)" << std::endl;

                    if (ele.type() == typeid(int)) {
                        indx.push_back(boost::get<int>(ele));
                    } else {
                        element = boost::get<std::string>(ele);
                    }
                }
                
                sel(selId i, std::vector<int> ele) : id(i), indx(ele)
                {
                    std::cout << id << ": sel(selId i, std::vector<int> ele)" << std::endl;
                }
                

                friend inline std::ostream& operator<<(std::ostream& out, sel const& sl) 
                {
                    out << "slId = " << sl.id << " element: " << sl.element << " index: [ ";
                    for (int i : sl.indx) {
                        out << i << " ";
                    }
                    out << ']' << std::endl;
                    return out;
                }
            };
            
            struct selList {
                std::vector<jsonpath::ast::selector::sel> slist;
            };
        
            std::string full_selector(selector::selList& sl)
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
        x3::rule<jsonpath_class, selector::selList> const jsonpath("jsonpath");

        /* Rules defined */
        x3::rule<struct root_selector, char> const root_selector_ = "root";
        x3::rule<struct dot_selector, std::string> dot_selector_ = "dot";
        x3::rule<struct dotw_selector, std::string> dotw_selector_ = "dotw";
        x3::rule<struct idxw_selector, std::string> idxw_selector_ = "idxw";

        // index-selector for number
        x3::rule<struct indx_selector, boost::variant<std::string, int>> indx_selector_ = "indx";

        x3::rule<struct slice_selector, std::vector<int>> slice_selector_ = "slice";
        x3::rule<struct lsts_selector, std::string> lsts_selector_ = "lsts";
        x3::rule<struct desc_selector, boost::variant<std::string, int>> desc_selector_ = "desc";
        x3::rule<struct filter_selector, std::string> filter_selector_ = "filter";

        auto root_action    = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::root, _attr(ctx))); };
        auto dot_action     = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::dot, _attr(ctx))); };
        auto dotw_action    = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::dotw, _attr(ctx))); };
        auto idxw_action    = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::idxw, _attr(ctx))); };
        auto indx_action    = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::indx, _attr(ctx))); };
        auto slice_action   = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::slice, _attr(ctx))); };
        auto lsts_action    = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::lsts, _attr(ctx))); };
        auto desc_action    = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::desc, _attr(ctx))); };
        auto filter_action  = [](auto& ctx) { sl_.slist.push_back(selector::sel(selector::selId::filter, _attr(ctx))); };

        // root-selector (0)
        const auto root_selector__def = x3::char_('$');
        // dot-selector (1)
        const auto dot_member_name = x3::char_('.') >> +x3::alpha;
        const auto dot_selector__def = dot_member_name;
        // dot-wild-selector (2)
        const auto dotw_selector__def = x3::string(".*");
        // index-wild-selector (3)
        const auto idxw_selector__def = x3::skip(x3::space)['[' >> x3::char_('*') >> ']'];

        // index-selector for numbers
        const auto element_index = x3::int_;
        
        // index-selector for string
        const auto double_quoted = '\"' >> +x3::alpha >> '\"';
        const auto single_quoted = '\'' >> +x3::alpha >> '\'';
        const auto string_literal = double_quoted | single_quoted;
        const auto quoted_member_name = string_literal;

        // split index selector in two rules
        // to avoid variant atrribute problems (4)
        const auto indx_selector__def = x3::skip(x3::space)['[' >> (element_index | quoted_member_name) >> ']'];

        // slice-selector [<start>:<end>:<step>] (5)
        const auto slice_selector__def = x3::skip(x3::space)['[' >> (x3::int_ | x3::attr(0)) >>
             (':' >> x3::int_  | ':' >> x3::attr(INT_MAX)) >>
            ((':' >> x3::int_) | ':' >> x3::attr(1) | x3::attr(1)) >> ']'];

        // list-selector (6)
        const auto lsts_selector__def = x3::lit("end");
        // descendant-selector (7)
        const auto desc_selector__def = (x3::lit("..") >> x3::string("[*]")) | (x3::lit("..[") >> x3::int_ >> ']') | (x3::lit("..") >> (x3::string("*") | +x3::alpha));
        // filter-selector (8)
        const auto filter_selector__def = x3::lit("end");

        const auto jsonpath_def = 
            // Begin grammar
            root_selector_[root_action]
            >> *(
                    dot_selector_[dot_action]       |
                    dotw_selector_[dotw_action]     |
                    idxw_selector_[idxw_action]     |
                    indx_selector_[indx_action]     |
                    slice_selector_[slice_action]   |
                    lsts_selector_[lsts_action]     |
                    desc_selector_[desc_action]     |
                    filter_selector_[filter_action]
                )
            // End Gramar
            ;

        BOOST_SPIRIT_DEFINE(root_selector_);
        BOOST_SPIRIT_DEFINE(dot_selector_);
        BOOST_SPIRIT_DEFINE(dotw_selector_);
        BOOST_SPIRIT_DEFINE(idxw_selector_);

        BOOST_SPIRIT_DEFINE(indx_selector_);

        BOOST_SPIRIT_DEFINE(slice_selector_);
        BOOST_SPIRIT_DEFINE(lsts_selector_);
        BOOST_SPIRIT_DEFINE(desc_selector_);
        BOOST_SPIRIT_DEFINE(filter_selector_);
        BOOST_SPIRIT_DEFINE(jsonpath);


        struct f_visit {
            void kv(boost::json::value v, std::deque<boost::json::value>& pt)
            { 
                //std::cout << "value: " << boost::json::serialize(v) << std::endl;
                pt.push_back(v);
            }
            void kv(bool b, std::deque<boost::json::value>& pt) 
            { 
                //std::cout << "bool: " << b << std::endl;
                pt.push_back(b);
            }
            void kv(double d, std::deque<boost::json::value>& pt) 
            { 
                //std::cout << "double: " << d << std::endl;
                pt.push_back(d);
            }
            void kv(std::int64_t i, std::deque<boost::json::value>& pt)
            {
                //std::cout << "int64: " << i << std::endl;
                pt.push_back(i);
            }
            void kv(std::uint64_t ui, std::deque<boost::json::value>& pt)
            {
                //std::cout << "uint64: " << ui << std::endl;
                pt.push_back(ui);
            }
            void kv(boost::json::array a, std::deque<boost::json::value>& pt)
            {
                //std::cout << "array: " << boost::json::serialize(a) << std::endl;
                pt.push_back(a);
            }
            void kv(boost::json::string s, std::deque<boost::json::value>& pt)
            {
                //std::cout << "string: " << s << std::endl;
                pt.push_back(s);
            }
            void kv(boost::json::string_view k, boost::json::value v, std::deque<boost::json::value>& pt)
            { 
                //std::cout << "object: " << k << " -> " << boost::json::serialize(v) << std::endl; 
                pt.push_back(v);
            }
        };

        void visit(f_visit fv, boost::json::value const& jv, std::deque<boost::json::value>& pt)
        {
            if (jv.is_object()) {
                auto const& obj = jv.get_object();
                if(!obj.empty())
                {
                    // Split to process the same level
                    for(auto it = obj.begin(); it != obj.end(); ++it) {
                        fv.kv(it->key(), it->value(), pt);
                    }
                    
                    // Split to process the same level
                    for(auto it = obj.begin(); it != obj.end(); ++it) {
                        visit(fv, it->value(), pt);
                    }
                }
            } else
            if (jv.is_array()) {
                auto const& arr = jv.get_array();
                if(!arr.empty())
                {
                    // Split to process the same level
                    for(auto it = arr.begin(); it != arr.end(); ++it) {
                        fv.kv(*it, pt);
                    }

                    // Split to process the same level
                    for(auto it = arr.begin(); it != arr.end(); ++it) {
                        visit(fv, *it, pt);
                    }
                }
            } 
            /*
                Do not include because the json::object
                was already included

            else
            if (jv.is_string()) {
                fv.kv(jv.get_string(), pt);
            } else
            if (jv.is_int64()) {
                fv.kv(jv.get_int64(), pt);
            } else
            if (jv.is_uint64()) {
                fv.kv(jv.get_uint64(), pt);
            } else
            if (jv.is_double()) {
                fv.kv(jv.get_double(), pt);
            } else
            if (jv.is_bool()) {
                fv.kv(jv.get_bool(), pt);
            } else
            if (jv.is_null()) {
                fv.kv("null", pt);
            }
            */
        }

        bool parse(std::string qs, boost::json::value& jv, std::vector<boost::json::value>& res)
        {
            namespace json = boost::json;
            std::string::const_iterator iter = qs.begin();
            std::string::const_iterator const end = qs.end();

            bool r = x3::parse(iter, end, jsonpath::parser::jsonpath, sl_);
                
            // Init json queue
            std::deque<boost::json::value> pt;

            if (r && (iter == end)) {
                namespace selId = jsonpath::ast::selector;

                // eval the jsonpath
                for (const auto& s : sl_.slist) {

                    switch (s.id) {
                        case selId::root: 
                            pt.push_back(jv);
                            break;

                        case selId::dot:
                            {
                                size_t sz = pt.size();
                                std::string::const_iterator it = ++(s.element.cbegin());
                                std::string tname(it, s.element.end());

                                for (int i = 0; i < sz; i++) {
                                    json::value& jv = pt.front();

                                    if (jv.is_object()) {
                                        auto const& obj = jv.get_object();
                                        if(!obj.empty()) {
                                            // Split to process the same level
                                            for(auto it = obj.begin(); it != obj.end(); ++it) {
                                                if (it->key() == tname) {
                                                   pt.push_back(it->value());
                                                   break;
                                                }
                                            }
                                        }
                                    }

                                    pt.erase(pt.begin());
                                }
                            }
                            break;

                        case selId::dotw: 
                            {
                                size_t sz = pt.size();

                                for (int i = 0; i < sz; i++) {
                                    json::value& jv = pt.front();

                                    if (jv.is_object()) {
                                        auto const& obj = jv.get_object();
                                        if(!obj.empty()) {
                                            // Split to process the same level
                                            for(auto it = obj.begin(); it != obj.end(); ++it) {
                                                pt.push_back(it->value());
                                            }
                                        }
                                    } else
                                    if (jv.is_array()) {
                                        auto const& arr = jv.get_array();
                                        if(!arr.empty())
                                        {
                                            // Split to process the same level
                                            for(auto it = arr.begin(); it != arr.end(); ++it) {
                                                pt.push_back(*it);
                                            }
                                        }
                                    }

                                    pt.erase(pt.begin());
                                }
                            }
                            break;

                        case selId::idxw:
                            {
                                size_t sz = pt.size();

                                for (int i = 0; i < sz; i++) {
                                    json::value& jv = pt.front();

                                    if (jv.is_array()) {
                                        auto const& arr = jv.get_array();
                                        if(!arr.empty())
                                        {
                                            // Split to process the same level
                                            for(auto it = arr.begin(); it != arr.end(); ++it) {
                                                pt.push_back(*it);
                                            }
                                        }
                                    }

                                    pt.erase(pt.begin());
                                }
                            }
                            break;

                        case selId::indx:
                            {
                                size_t sz = pt.size();

                                if (s.element.empty()) { // index is number
                                    for (int i = 0; i < sz; i++) {
                                        json::value& jv = pt.front();
                                        if (jv.is_array()) {
                                            auto const& arr = jv.get_array();
                                            
                                            if(!arr.empty())
                                            {
                                                int j = s.indx[0];
                                                int len = arr.size();

                                                if ((j < len) && (j >= -len)) {
                                                    int k = (len + j) % len;
                                                    pt.push_back(arr[k]);
                                                }
                                            }
                                            pt.erase(pt.begin());
                                        }
                                    }
                                } else {
                                    for (int i = 0; i < sz; i++) {
                                        json::value& jv = pt.front();
                                        if (jv.is_array()) {
                                            auto const& arr = jv.get_array();
                                            
                                            if(!arr.empty()) {
                                                for (const auto& item : arr) {
                                                    if (item.is_object())
                                                    {
                                                        auto obj = item.as_object();
                                                        if (!obj[s.element].is_null()) {
                                                            pt.push_back(obj[s.element]);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        pt.erase(pt.begin());
                                    }
                                }
                            }
                            break;

                        case selId::slice:
                            {
                                size_t sz = pt.size();
                                if (!s.indx.empty()) { // index is number
                                    for (int i = 0; i < sz; i++) {
                                        json::value& jv = pt.front();
                                        if (jv.is_array()) {
                                            auto const& arr = jv.get_array();

                                            const auto normalize = [](int i, int len) -> int {
                                                if (i >= 0) {
                                                    return i;
                                                } else {
                                                    return len + i;
                                                }
                                            };

                                            int n_start = normalize(s.indx[0], arr.size());
                                            int n_end   = (s.indx[1] == INT_MAX) ? (int)arr.size() : s.indx[1];

                                            n_end       = normalize(n_end, arr.size());
                                            int step    = s.indx[2];

                                            int lower, upper;

                                            if (step >= 0) {
                                               lower = std::min(std::max(n_start, 0), (int)arr.size());
                                               upper = std::min(std::max(n_end, 0), (int)arr.size());
                                            } else {
                                               lower = std::min(std::max(n_start, -1), ((int)arr.size() - 1));
                                               upper = std::min(std::max(n_end, -1), ((int)arr.size() - 1));
                                            }

                                            if (step > 0) {
                                                int i = lower;
                                                while (i < upper) {
                                                    pt.push_back(arr[i]);
                                                    i = i + step;
                                                }
                                            } else if (step < 0) {
                                                int i = upper;
                                                while (lower <= i) {
                                                    pt.push_back(arr[i]);
                                                    i = i + step;
                                                }
                                            }
                                        }
                                        pt.erase(pt.begin());
                                    }
                                }
                            }
                            break;

                        case selId::lsts:
                            {
                            }
                            break;

                        case selId::desc:
                            {
                                size_t sz = pt.size();
                                if (s.element == "*") {
                                    // catch all tree
                                    for (int i = 0; i < sz; i++) {
                                        json::value& jv = pt.front();
                                        f_visit fv;
                                        pt.push_back(jv);
                                        visit(fv, jv, pt);
                                        pt.erase(pt.begin());
                                    }
                                } else
                                if (s.element == "[*]") {
                                    // array only
                                    for (int i = 0; i < sz; i++) {
                                        json::value& jv = pt.front();
                                        if (jv.is_array()) {
                                            f_visit fv;
                                            visit(fv, jv, pt);
                                        }
                                        pt.erase(pt.begin());
                                    }
                                } else
                                if (!s.element.empty()) {
                                    // dot name only
                                    for (int i = 0; i < sz; i++) {
                                        std::deque<boost::json::value> aux;
                                        json::value& jv = pt.front();
                                        f_visit fv;
                                        aux.push_back(jv);
                                        visit(fv, jv, aux);

                                        for (auto const& obj : aux) {
                                            if (obj.is_object() && !obj.as_object().empty()) {
                                                for(auto it = obj.as_object().begin(); it != obj.as_object().end(); ++it) {
                                                    if (it->key() == s.element) {
                                                        pt.push_back(it->value());
                                                    }
                                                }
                                            }
                                        }

                                        pt.erase(pt.begin());
                                    }
                                } else
                                if (!s.indx.empty()) {
                                    // array by index number
                                    for (int i = 0; i < sz; i++) {
                                        json::value& jv = pt.front();
                                        if (s.indx[0] >= 0) {
                                            std::deque<boost::json::value> aux;
                                            f_visit fv;
                                            aux.push_back(jv);
                                            visit(fv, jv, aux);

                                            for (auto const& obj : aux) {
                                                if (obj.is_array() && !obj.as_array().empty()) {
                                                    if (s.indx[0] < obj.as_array().size()) {
                                                        pt.push_back(obj.as_array()[s.indx[0]]);
                                                    }
                                                }
                                            }
                                        }

                                        pt.erase(pt.begin());
                                    }
                                }
                            }
                            break;

                        case selId::filter:
                            {
                            }
                            break;
                    }
                }


                for (const auto& jv : pt) {
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
/*
        struct f_visit {
            //json_tree jtree;

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

        }
*/
