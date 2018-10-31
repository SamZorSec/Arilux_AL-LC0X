#pragma once
#include <algorithm>

class Helpers {

  public:

    template <typename T>
    static T constrain(const T& n, const T& lower, const T& upper) {
      return std::max(lower, std::min(n, upper));
    }
};
