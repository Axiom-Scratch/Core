#pragma once

#include "Math.h"

namespace Core::Math
{

inline Mat4 Translate(const Vec3& position)
{
    return glm::translate(Mat4(1.0f), position);
}

inline Mat4 Scale(const Vec3& scale)
{
    return glm::scale(Mat4(1.0f), scale);
}

}
