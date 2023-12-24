#include <linc/system/TypedValue.hpp>

namespace linc
{
    const linc::TypedValue linc::TypedValue::invalidValue(Types::invalid_type{});
    const linc::TypedValue linc::TypedValue::voidValue(Types::_void_type{});
}
