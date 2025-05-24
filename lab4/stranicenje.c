#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_N 30
#define MAX_M 30
int n, m, t;
short tablica[MAX_N][16];
char ram[MAX_M][64];
char disk[MAX_N][16][64];
int popunjeno;
int dohvati_fizicku_adresu(int p, int x)
{
        short tp = (short)x;
        tp = tp & 960;
        tp = tp >> 6;
        printf("----------------\n");
        printf("proces: %d\n", p);
        printf("     t: %d\n", t);
        printf("     tp: %d\n", tp);
        short to = tablica[p][tp];
        short prisutna = to & 32;
        short adr, okv;
        short p1 = x & 63;
        if (prisutna == 0) {
                printf("    promasaj!\n");
                if (popunjeno < m) {
                        okv = popunjeno;
                        adr = okv * 64 + p1;
                        printf("    PRVI PUT DODIJELJENO okv: %d\n   fiz. adr: "
                               "%d\n",
                               okv, adr);
                        short lru = t;
                        if (t >= 31) {
                                for (int i = 0; i < n; i++) {
                                        for (int j = 0; j < 16; j++) {
                                                tablica[i][j] =
                                                    tablica[i][j] & 65504;
                                        }
                                }
                                lru = 1;
                                t = 0;
                        }
                        tablica[p][tp] = (okv << 6) + 32 + lru;
                        printf("    test tablica %d %d %d\n", p, tp,
                               tablica[p][tp]);
                        printf("    Trenutni lru: %d\n", lru);
                        popunjeno += 1;
                } else {
                        int max = 2000, I = 0, J = 0;
                        for (int i = 0; i < n; i++) {
                                for (int j = 0; j < 16; j++) {
                                        short tmp = tablica[i][j];
                                        int z = tmp & 32;
                                        if (z == 32) {
                                                short p_lru = tmp & 31;
                                                if (p_lru < max) {
                                                        I = i;
                                                        J = j;
                                                        max = p_lru;
                                                }
                                        }
                                }
                        }
                        short lru = t;
                        if (t >= 31) {
                                for (int i = 0; i < n; i++) {
                                        for (int j = 0; j < 16; j++) {
                                                tablica[i][j] =
                                                    tablica[i][j] & 65504;
                                        }
                                }
                                lru = 1;
                                t = 0;
                        }
                        printf("        debug: %d %d %d\n", tablica[I][J], I,
                               J);
                        short i_lru = tablica[I][J] & 31;
                        okv = tablica[I][J] >> 6;
                        short c = 32;
                        tablica[p][tp] = (okv << 6) + c + lru;
                        printf("        debug2: %d\n", tablica[p][tp]);

                        tablica[I][J] = 0;
                        printf("        lru izbacene stranice: 0x%04X\n",
                               i_lru);
                        printf("        Dodijeljen okivr 0x%04X\n", okv);
                        adr = (okv * 64) + p1;
                        for (int i = 0; i < 64; i++) {
                                disk[I][J][i] = ram[okv][i];
                        }
                        for (int i = 0; i < 64; i++) {
                                ram[okv][i] = disk[p][tp][i];
                        }
                }

        } else {
                okv = tablica[p][tp] >> 6;
                adr = (okv * 64) + p1;
                // ram[okv][p1] = disk[p][x];
        }
        printf("    log adresa: 0x%04X\n    fizicka adresa: 0x%04X\n    "
               "sadrzaj: %d\n",
               x, adr, ram[okv][p1]);
        printf("----------------\n");
        return adr;
}

int dohvati_sadrzaj(int p, int x)
{
        int y = dohvati_fizicku_adresu(p, x);
        short okv = y / 64, pom = y % 64;
        char i = ram[okv][pom];
        return i;
}

void zapisi_vrijednost(int p, int x, char i)
{
        int y = dohvati_fizicku_adresu(p, x);
        short okv = y / 64, pom = y % 64;
        printf("***TESTIRANJE RAMA %d\n", ram[okv][pom]);
        ram[okv][pom] = i;
}

int main()
{
        srand(time(NULL));
        scanf("%d %d", &n, &m);
        t = 0;
        for (int i = 0; i < n; i++) {
                for (int j = 0; j < 16; j++) {
                    for (int k = 0 ; k < 64; k++)
                        disk[i][j][k] = 0;
                }
        }
        while (1) {
                for (int i = 0; i < n; i++) {
                        int x = (int)(rand()) % 1024;
                        //int x = 593;
                        //int x = (int)(rand()) % 10;
                        x = x & 0x3fe;
                        char k = dohvati_sadrzaj(i, x);
                        k = k + 1;
                        zapisi_vrijednost(i, x, k);
                        t = t + 1;
                        sleep(1);
                }
        }
        return 0;
}
