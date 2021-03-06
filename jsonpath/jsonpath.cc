/*======================================
 * JSONPath parser
 *
 * (c)Ventura			2022
 *====================================*/
// Uncomment this if you want to enable debugging
#define BOOST_SPIRIT_X3_DEBUG

#include <boost/spirit/home/x3.hpp>
#include <boost/json.hpp>

#include <boost/fusion/container.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <variant>
#include <deque>
#include <iostream>

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
                std::vector<boost::variant<std::vector<int>, std::string, int>> lsts;

                sel(selId i) : id(i)
                {
                    std::cout << id << ": sel(selId i)" << std::endl;
                }
                
                sel(selId i, std::vector<int> ele) : id(i), indx(ele)
                {
                    std::cout << id << ": sel(selId i, std::vector<int> ele)" << std::endl;
                }
                
                sel(selId i, std::string ele) : id(i), element(ele)
                {
                    std::cout << id << ": sel(selId i, std::string ele)" << std::endl;
                }
                
                sel(selId i, char ele) : id(i)
                {
                    std::cout << id << ": sel(selId i, char)" << std::endl;
                }
                
                sel(selId i, boost::variant<std::string, int> ele) : id(i)
                {
                    std::cout << id << ": sel(selId i, boost::variant<std::string, int> ele)" << std::endl;

                    if (ele.type() == typeid(int)) {
                        indx.push_back(boost::get<int>(ele));
                    } else {
                        element = boost::get<std::string>(ele);
                        std::cout << element << std::endl;
                    }
                }
                
                sel(selId i, std::vector<boost::variant<std::vector<int>, std::string, int>> ele) : id(i), lsts(ele)
                {
                    std::cout << id << ": sel(selId i, std::vector<boost::variant<std::vector<int>, std::string, int>> ele)" << std::endl;
                }
        

                // filter selector
                sel(selId i, std::vector<boost::variant<boost::variant<std::string, int>, std::string>> ele) : id(i)
                {
                    std::cout << id << ": sel(selId i, std::vector<boost::variant<boost::variant<std::string, int>, std::string>> ele)" << std::endl;
                    std::cout << "================>>>>>>>>> I am here: " << ele.size() << std::endl;
                    for (auto const& it : ele) {
                        if (it.type() == typeid(std::string)) {
                            std::cout << boost::get<std::string>(it) << std::endl;
                        }
                    }
                }
                

                friend inline std::ostream& operator<<(std::ostream& out, sel const& sl) 
                {
                    out << "slId = " << sl.id << " element: " << sl.element << " index: [ ";
                    for (int i : sl.indx) {
                        out << i << " ";
                    }
                    out << "] lsts: [ ";
                    for (auto const& e : sl.lsts) {
                        if (e.type() == typeid(int)) {
                            out << boost::get<int>(e) << ", ";
                        } else
                        if (e.type() == typeid(std::string)) {
                            out << boost::get<std::string>(e) << ", ";
                        } else if (e.type() == typeid(std::vector<int>)) {
                            std::vector<int> v = boost::get<std::vector<int>>(e);
                            out << v[0] << ':' << v[1] << ':' << v[2] << ", ";
                        }
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
                for (auto const& sel : sl.slist) {
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
        x3::rule<jsonpath_class, selector::selList> const json_path("json-path");

        // rule for space S
        auto const blank = x3::blank | x3::char_('\t') | x3::char_('\n') | x3::char_('\r');
        auto const space = +blank;

        /* Rules defined */
        x3::rule<struct root_selector_id, char> const root_selector_ = "root-sel";
        x3::rule<struct dot_selector_id, std::string> dot_selector_ = "dot-sel";
        x3::rule<struct dotw_selector_id, std::string> dotw_selector_ = "dotw-sel";
        x3::rule<struct idxw_selector_id, char> idxw_selector_ = "idxw-sel";
        // index-selector for number
        x3::rule<struct indx_selector_id, boost::variant<std::string, int>> indx_selector_ = "indx";
        x3::rule<struct slice_selector_id, std::vector<int>> slice_selector_ = "slice-sel";
        // list-selector for number
        // ==================================
        x3::rule<struct lsts_selector_id, std::vector<boost::variant<std::vector<int>, std::string, int>>> lsts_selector_ = "lsts-sel";
        // ==================================
        x3::rule<struct desc_selector, boost::variant<std::string, int>> desc_selector_ = "desc-sel";
        
        // filter-selector
        //x3::rule<struct filter_selector, std::vector<boost::variant<boost::variant<std::string, int>, std::string>>> filter_selector_ = "filter";
        //x3::rule<struct filter_selector, std::vector<boost::variant<std::string, int>>> filter_selector_ = "filter";
        x3::rule<struct filter_selector, boost::variant<std::string, int>> filter_selector_ = "filter-sel";
        //x3::rule<struct filter_selector, std::string> filter_selector_ = "filter-sel";

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
        auto const root_selector__def = x3::char_('$'); // attr: char
        // dot-selector (1)
        x3::rule<struct dot_member_name_id, std::string> dot_member_name = "dot-member-name";
        
        auto const name_first = x3::char_('_') | x3::alpha | x3::char_(0x80, 0x10ffff);
        auto const name_char = x3::digit | name_first;
        
        auto const dot_member_name_def = name_first >> *name_char;
        BOOST_SPIRIT_DEFINE(dot_member_name);

        auto const dot_selector__def = '.' >> dot_member_name; // attr: std::string
        // dot-wild-selector (2)
        auto const dotw_selector__def = x3::string(".*"); // attr: std::string
        // index-wild-selector (3)
        auto const idxw_selector__def = '[' >> x3::char_('*') >> ']'; // attr: std::string

        // index-selector for numbers
        auto const element_index = x3::int_;    // attr: int
        
        // index-selector for string
        auto const double_quoted = '\"' >> dot_member_name >> '\"';
        auto const single_quoted = '\'' >> dot_member_name >> '\'';
        auto const string_literal = double_quoted | single_quoted;

        x3::rule<struct quoted_member_name_id, std::string> quoted_member_name = "quoted-member-name";
        auto const quoted_member_name_def = string_literal;     // attr: std::string
        BOOST_SPIRIT_DEFINE(quoted_member_name);

        // split index selector in two rules
        // to avoid variant atrribute problems (4)
        auto const indx_selector__def = '[' >> (element_index | quoted_member_name) >> ']'; // attr: boost::varian<std::string, int>

        // slice-selector [<start>:<end>:<step>] (5)
        x3::rule<struct slice_index_id, std::vector<int>> slice_index = "slice-index";
        auto const slice_index_def = (x3::int_ | x3::attr(0)) >>
             (':' >> x3::int_  | ':' >> x3::attr(INT_MAX)) >>
            ((':' >> x3::int_) | ':' >> x3::attr(1) | x3::attr(1));
        BOOST_SPIRIT_DEFINE(slice_index);

        auto const slice_selector__def = '[' >> slice_index >> ']'; // attr: std::vector<int>

        // list-selector (6)
        x3::rule<struct list_entry_id, boost::variant<std::vector<int>, std::string, int>> list_entry = "list-entry";
        // The order in the rule is important: slice_index -> quoted_member_name -> element_index
        auto const list_entry_def = slice_index | quoted_member_name | element_index; // attr: boost::variant<std::vector<int>, std::string, int>
        BOOST_SPIRIT_DEFINE(list_entry);

        // attr: std::vector<boost::variant<std::string, int, std::vector<int>>>
        auto const lsts_selector__def = '[' >> (list_entry % ',') >> ']';

        // descendant-selector (7)
        auto const desc_selector__def = x3::lit("..") >> (
                                                            dot_member_name |
                                                            indx_selector_  |
                                                            idxw_selector_  |
                                                            x3::string("*")
                                                         );
        // filter-selector (8)
        auto const rel_path = x3::lit('@') >> *(indx_selector_ | dot_selector_);

        auto const path = rel_path | json_path;
        auto const exist_expr = -x3::lit('!') >> path;
        
        //x3::rule<struct bolean_expr_id, std::vector<boost::variant<boost::variant<std::string, int>, std::string>>> bolean_expr = "bolean-expr";
        //x3::rule<struct bolean_expr_id, boost::variant<std::string, int>> bolean_expr = "bolean-expr";
        //x3::rule<struct bolean_expr_id, std::vector<boost::variant<std::string, int>>> bolean_expr = "bolean-expr";
        x3::rule<struct bolean_expr_id> bolean_expr = "bolean-expr";

        auto const comp_op =    x3::string("==") | x3::string("!=") |
                                x3::string("<")  | x3::string(">")   |
                                x3::string("<=") | x3::string(">=");

        auto const exp = (x3::char_('e') | x3::char_('E')) >> -(x3::char_('-') | x3::char_('+')) >> +x3::digit;
        auto const frac = '.' >> +x3::digit;
        auto const true_ = x3::string("true");
        auto const false_ = x3::string("false");
        auto const null_ = x3::string("null");

        auto const number = x3::int_ >> -(frac >> -exp);
        auto const comparable = number | string_literal | true_ | false_ | null_;

        auto const comp_expr = comparable >> comp_op >> comparable;

        auto const regex_expr = +x3::char_;
        auto const relation_expr = comp_expr | regex_expr;

        auto const paren_expr = -x3::lit('!') >> '(' >> bolean_expr >> ')';
 
        auto const basic_expr = exist_expr | paren_expr | relation_expr;
        //auto const basic_expr = exist_expr;
        //auto const basic_expr = paren_expr;
        //auto const basic_expr = relation_expr;
        auto const logical_and_expr = basic_expr % "&&";
        auto const logical_or_expr = logical_and_expr % "||";

        auto const bolean_expr_def = logical_or_expr;
        BOOST_SPIRIT_DEFINE(bolean_expr);

        auto const filter_selector__def = '[' >> x3::lit('?') >> bolean_expr >> ']';

        auto const json_path_def =
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
        BOOST_SPIRIT_DEFINE(json_path);


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
        }

        bool parse(std::string qs, boost::json::value& jv, std::vector<boost::json::value>& res)
        {
            namespace json = boost::json;
            std::string::const_iterator iter = qs.begin();
            std::string::const_iterator const end = qs.end();

            bool r = x3::parse(iter, end, x3::skip(parser::space)[jsonpath::parser::json_path], sl_);
                
            // Init json queue
            std::deque<boost::json::value> pt;

            if (r && (iter == end)) {
                namespace selId = jsonpath::ast::selector;

                // eval the jsonpath
                for (auto const& s : sl_.slist) {

                    switch (s.id) {
                        // root-selector
                        case selId::root: 
                            pt.push_back(jv);
                            break;

                        // dot-selector
                        case selId::dot:
                            {
                                size_t sz = pt.size();
                                std::string::const_iterator it = s.element.cbegin();
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

                        // dot-wild-selector
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

                        // index-wild-selector
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
                                    } else
                                    if (jv.is_object()) {
                                        auto& obj = jv.get_object();
                                        if(!obj.empty()) {
                                            // Split to process the same level
                                            for(auto it = obj.begin(); it != obj.end(); ++it) {
                                                pt.push_back(it->value());
                                            }
                                        }
                                    }

                                    pt.erase(pt.begin());
                                }
                            }
                            break;

                        // index-selector
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
                                                for (auto const& item : arr) {
                                                    if (item.is_object())
                                                    {
                                                        auto obj = item.as_object();
                                                        if (!obj[s.element].is_null()) {
                                                            pt.push_back(obj[s.element]);
                                                        }
                                                    }
                                                }
                                            }
                                        } else
                                        if (jv.is_object()) {
                                            auto& obj = jv.get_object();
                                            if(!obj.empty()) {
                                                if (!obj[s.element].is_null()) {
                                                    pt.push_back(obj[s.element]);
                                                }
                                            }
                                        }
                                        pt.erase(pt.begin());
                                    }
                                }
                            }
                            break;

                        // slice-selector
                        case selId::slice:
                            {
                                if (!s.indx.empty()) { // index is number
                                    size_t sz = pt.size();
                                    for (int i = 0; i < sz; i++) {
                                        json::value& jv = pt.front();
                                        if (jv.is_array()) {
                                            auto const& arr = jv.get_array();

                                            auto const normalize = [](int i, int len) -> int {
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

                        // list-selector
                        case selId::lsts:
                            {
                                if (!s.lsts.empty()) {
                                    size_t sz = pt.size();
                                    for (auto const& e : s.lsts) {
                                        if (e.type() == typeid(int)) {
                                            for (int i = 0; i < sz; i++) {
                                                json::value& jv = pt[i];
                                                if (jv.is_array()) {
                                                    auto const& arr = jv.get_array();
                                                    if(!arr.empty())
                                                    {
                                                        int j = boost::get<int>(e);
                                                        int len = arr.size();
                                                        if ((j < len) && (j >= -len)) {
                                                            int k = (len + j) % len;
                                                            pt.push_back(arr[k]);
                                                        }
                                                    }
                                                }
                                            }
                                        } else
                                        if (e.type() == typeid(std::string)) {
                                            for (int i = 0; i < sz; i++) {
                                                json::value& jv = pt[i];
                                                if (jv.is_array()) {
                                                    auto const& arr = jv.get_array();

                                                    if(!arr.empty()) {
                                                        for (auto const& item : arr) {
                                                            if (item.is_object())
                                                            {
                                                                auto obj = item.as_object();
                                                                std::string name = boost::get<std::string>(e);
                                                                if (!obj[name].is_null()) {
                                                                    pt.push_back(obj[name]);
                                                                }
                                                            }
                                                        }
                                                    }
                                                } else
                                                if (jv.is_object()) {
                                                    auto& obj = jv.get_object();
                                                    if(!obj.empty()) {
                                                        std::string name = boost::get<std::string>(e);
                                                        if (!obj[name].is_null()) {
                                                            pt.push_back(obj[name]);
                                                        }
                                                    }
                                                }
                                            }
                                        } else if (e.type() == typeid(std::vector<int>)) {
                                            for (int i = 0; i < sz; i++) {
                                                json::value& jv = pt[i];
                                                std::vector<int> v = boost::get<std::vector<int>>(e);
                                                if (jv.is_array()) {
                                                    auto const& arr = jv.get_array();

                                                    auto const normalize = [](int i, int len) -> int {
                                                        if (i >= 0) {
                                                            return i;
                                                        } else {
                                                            return len + i;
                                                        }
                                                    };

                                                    int n_start = normalize(v[0], arr.size());
                                                    int n_end   = (v[1] == INT_MAX) ? (int)arr.size() : v[1];

                                                    n_end       = normalize(n_end, arr.size());
                                                    int step    = v[2];

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
                                                    } else
                                                    if (step < 0) {
                                                        int i = upper;
                                                        while (lower <= i) {
                                                            pt.push_back(arr[i]);
                                                            i = i + step;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    for (int i = 0; i < sz; i++) {
                                        pt.erase(pt.begin());
                                    }
                                }
                            }
                            break;

                        // descendant-selector
                        case selId::desc:
                            {
                                size_t sz = pt.size();
                                if (s.element == "*") {
                                    // catch all tree
                                    for (int i = 0; i < sz; i++) {
                                        json::value& jv = pt.front();
                                        f_visit fv;
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

                        // filter-selector
                        case selId::filter:
                            {
                                size_t sz = pt.size();
                                for (int i = 0; i < sz; i++) {
                                    json::value& jv = pt.front();
                                    if (jv.is_array()) {
                                        f_visit fv;
                                        visit(fv, jv, pt);
                                        //std::cout << jv << std::endl;
                                    }
                                    pt.erase(pt.begin());
                                }
                            }
                            break;
                    }
                }


                for (auto const& jv : pt) {
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

