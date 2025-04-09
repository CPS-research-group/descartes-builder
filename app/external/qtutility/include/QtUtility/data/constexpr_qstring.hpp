#pragma once

#include <QLatin1String>

namespace QtUtility {
namespace data {

struct ConstLatin1String : public QLatin1String
{
    constexpr ConstLatin1String(const char *const s)
        : QLatin1String(s, static_cast<int>(std::char_traits<char>::length(s)))
    {}
};

} // namespace data
} // namespace QtUtility