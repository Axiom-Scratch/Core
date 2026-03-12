#pragma once

#include "Math.h"

namespace Core::Math
{

inline float Length(const Vec3& v)
{
    return glm::length(v);
}

inline Vec3 Normalize(const Vec3& v)
{
    return glm::normalize(v);
}

}
