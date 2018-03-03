#ifndef D_MATH_H
#define D_MATH_H

#include <bouge/Math.hpp>

namespace FTS {

    using bouge::Vector;

    using bouge::Quaternion;

    using bouge::AffineMatrix;
    using bouge::Base4x4Matrix;
    using bouge::General4x4Matrix;

    using bouge::TimeFunction;
    using bouge::ConstantTF;
    using bouge::CycleTF;
    using bouge::FadeInTF;
    using bouge::FadeOutTF;
    using bouge::HoldTF;
    using bouge::LinearTF;
    using bouge::RepeatTF;

    using bouge::pi;
    using bouge::rad2deg;
    using bouge::deg2rad;
    using bouge::nearZero;
    using bouge::clamp;
    using bouge::lerp;
    using bouge::fract;
    using bouge::power_of_two;

} // namespace FTS

#endif // D_MATH_H
