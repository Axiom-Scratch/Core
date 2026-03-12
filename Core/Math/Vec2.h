#pragma once

#include "Math.h"

namespace Core::Math
{

inline float Length(const Vec2& v)
{
    return glm::length(v);
}

inline Vec2 Normalize(const Vec2& v)
{
    return glm::normalize(v);
}

}
