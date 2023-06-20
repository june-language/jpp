#ifndef june_result_hpp
#define june_result_hpp

#define RESULT_NAMESPACE bws

#include "Option.hpp"
#include "SFINAE.hpp"
#include "bws_result.hpp"
#include <exception>
#include <optional>
#include <ostream>
#include <string>
#include <variant>

namespace june {
namespace err {

using namespace june::functional;

template <typename O, typename E>
using Result = bws::result<O, E>;

using Errors = err::Result<void, std::string>;

} // namespace err
} // namespace june

#endif
