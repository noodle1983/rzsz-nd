#include "Singleton.hpp"

#include <string>
#include <vector>

class ZmodemFile;

namespace nd{

struct Options{
    // from commander line
    unsigned debugLevel;
    std::vector<ZmodemFile*> files;
};

#define g_options nd::Singleton<nd::Options>::instance()

}

