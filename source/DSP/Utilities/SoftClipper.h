#pragma once
#include <cmath>

class SoftClipper
{
public:
    void setThreshold (float threshold) { threshold_ = threshold; }

    float processSample (float input) const
    {
        if (std::abs (input) <= threshold_)
            return input;
        return threshold_ * std::tanh (input / threshold_);
    }

private:
    float threshold_ = 0.95f;
};
