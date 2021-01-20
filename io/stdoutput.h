#ifndef OUTPUT_H
#define OUTPUT_H

//#include <iostream>
#include <stdio.h>

static inline void send_data(const char* buffer, const int len)
{
    write(1, buffer, len);
}

#endif /* OUTPUT_H */

