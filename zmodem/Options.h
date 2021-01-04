#include "Singleton.hpp"

#include <string>
#include <vector>

namespace nd{

struct Options{
    // from commander line
    unsigned debugLevel;
    std::vector<std::string> files;
};

#define g_options nd::Singleton<nd::Options>::instance()

}

