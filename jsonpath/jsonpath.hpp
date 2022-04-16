/*======================================
 * JSONPath parser
 *
 * (c)Ventura			2022
 *====================================*/

#ifndef	JSONPATH_HPP
#define	JSONPATH_HPP

#include <boost/json.hpp>

namespace jsonpath {
    namespace parser {
        bool parse(std::string qs, boost::json::value& jv, std::vector<boost::json::value>& res);
    }
}

#endif	// JSONPATH_HPP
