#ifndef OUTPUT_H
#define OUTPUT_H

#include <iostream>

static inline void send_data(const char* buffer, const int len)
{
    std::cout.write(buffer, len);
}

#endif /* OUTPUT_H */

