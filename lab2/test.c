#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
int main()
{
        for (int i = 0; i < 5; i++) {
                printf("Hello world %d!\n", i);
                sleep(10);
        }
        return 0;
}
