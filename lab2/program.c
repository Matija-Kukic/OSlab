#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
        for (int i = 1; i < argc; i++) {
                printf("Argument[%d]: %s\n", i, argv[i]);
                sleep(5);
        }
        return 0;
}
