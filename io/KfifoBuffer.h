/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#ifndef KFIFOBUFFER_H
#define KFIFOBUFFER_H

#include <sys/types.h>
#include <atomic>

namespace nd 
{
    enum BufferStatus
    {
        BufferNotEnoughE = -1,
        BufferOkE = 0,
        BufferHighE = 1,
        BufferLowE = 2
    };

    class KfifoBuffer
    {
    public:
        KfifoBuffer(const int theSizePower);
        ~KfifoBuffer();

        void init();
        void release();

        inline BufferStatus getStatus()
        {
            unsigned s = size();
            return (s > highWaterMarkM) ? BufferHighE
                 : (s > lowWaterMarkM)  ? BufferOkE
                 : BufferLowE;
        }
        inline bool isHealthy()
        {
            return (size() < highWaterMarkM); 
        }
        inline bool empty() {return writeIndexM == readIndexM;}
        inline unsigned size(){return (writeIndexM - readIndexM);}
        inline unsigned unusedSize(){return sizeM - (writeIndexM - readIndexM);}

        unsigned put(const char* const theBuffer, const unsigned theLen);
        unsigned get(char* const theBuffer, const unsigned theLen);
        unsigned peek(char* const theBuffer, const unsigned theLen);
        unsigned putn(const char* const theBuffer, const unsigned theLen);
        unsigned getn(char* const theBuffer, const unsigned theLen);
        unsigned peekn(char* const theBuffer, const unsigned theLen);
        unsigned commitRead(const unsigned theLen);

    private:
        char* rawM;
        unsigned sizeM;
        unsigned maskM;
        std::atomic<unsigned> readIndexM;
        std::atomic<unsigned> writeIndexM;
        unsigned highWaterMarkM;
        unsigned lowWaterMarkM;
    };
}

#endif /* KFIFOBUFFER_H */

