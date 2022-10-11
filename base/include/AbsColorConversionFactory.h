#pragma once
#include "ColorConversion.h"

class AbsColorConversionFactory
{
public:
    static boost::shared_ptr<DetailAbs> create(std::string &version, uint16_t type);
};