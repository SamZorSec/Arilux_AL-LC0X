#pragma once

#include "HSB.h"

class Store {
public:
    virtual const HSB getHSB() const = 0;
    virtual bool storeHSB(HSB hsb) = 0;
    virtual void initStore(HSB hsb) = 0;
};
