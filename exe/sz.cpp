#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    char buf[1024];
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    while(true){
        int numRead = read(0, buf, sizeof(buf));
        if (numRead > 0) {
            printf("get %d\n", numRead);
        }
    }
    return 0;
}
