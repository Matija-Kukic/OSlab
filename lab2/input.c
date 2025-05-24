#include <stdio.h>
#include <unistd.h>

int main()
{
        int a;
        scanf("%d", &a);
        for (int i = 0; i < 5; i++) {
                printf("Ispisujem %d po %d-ti put\n", a, i);
                sleep(5);
        }
}
