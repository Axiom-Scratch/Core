#pragma once

#include "Math.h"

namespace Core::Math
{

inline float Length(const Vec4& v)
{
    return glm::length(v);
}

inline Vec4 Normalize(const Vec4& v)
{
    return glm::normalize(v);
}

}
