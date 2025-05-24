#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void obradi_sigint(int sig);
void prekid1();
void prekid2();
void prekid3();
void prekid4();
void ispis_debug();
void print_kz();

void prihvat_prekida(int p);
void blokiraj_odblokiraj_signale(int blokiraj)
{
        sigset_t signali;
        sigemptyset(&signali);
        sigaddset(&signali, SIGTERM);
        sigaddset(&signali, SIGINT);
        if (blokiraj)
                pthread_sigmask(SIG_BLOCK, &signali, NULL);
        else
                pthread_sigmask(SIG_UNBLOCK, &signali, NULL);
}
void print_sigaction_flags(int flags)
{
        printf("sa_flags value: %d\n", flags);

        if (flags & SA_NOCLDSTOP)
                printf("SA_NOCLDSTOP is set\n");
        if (flags & SA_RESTART)
                printf("SA_RESTART is set\n");
}
int oznaka_cekanja[5], kontekst[5] = {-1, -1, -1, -1, -1};
int trenutni = 0;
int p = -1;
int nije_kraj = 1;
int stanje = 1;

int main()
{
        struct sigaction act;

        act.sa_handler = obradi_sigint;
        sigemptyset(&act.sa_mask);
        sigaction(SIGINT, &act, NULL);
        printf("Program s PID=%ld krenuo s radom\n", (long)getpid());

        int i = 1;
        while (nije_kraj) {
                if (stanje == 1) {
                        printf("U glavnom programu!\n");
                        stanje = 0;
                }
        }

        printf("Program s PID=%ld zavrsio s radom\n", (long)getpid());
        return 0;
}

void obradi_sigint(int sig)
{
        int a;
        stanje = 1;

        printf("Primio signal SIGINT, unesi prioritet: ");
        scanf("%d", &a);
        if (a == 0)
                exit(1);
        p = a;
        oznaka_cekanja[a] = 1;
        prihvat_prekida(a);
}
void prihvat_prekida(int p)
{ // prima prioritet prekida
        printf("Prihvat prekida \n");
        print_kz();
        int I = 4;
        while (oznaka_cekanja[I] != 1 && I > 0)
                --I;
        while (I > trenutni && oznaka_cekanja[I] > 0) {
                // print_kz();
                oznaka_cekanja[I] = 0;
                kontekst[I] = trenutni;
                trenutni = I;
                print_kz();
                blokiraj_odblokiraj_signale(0);
                struct sigaction act;
                act.sa_handler = obradi_sigint;
                sigemptyset(&act.sa_mask);
                // printf("TEST %d\n", I);
                sigaction(SIGINT, &act, NULL);
                if (I == 4)
                        prekid4();
                if (I == 3)
                        prekid3();
                if (I == 2)
                        prekid2();
                if (I == 1)
                        prekid1();
                blokiraj_odblokiraj_signale(1);
                trenutni = kontekst[I];
                kontekst[I] = -1;
                print_kz();
                while (oznaka_cekanja[I] != 1 && I > 0)
                        --I;
        }

        // Priprema seta signala
}
void prekid1()
{
        printf("U prekidu: %d\n", 1);
        for (int i = 0; i < 5; i++)
                sleep(1);
        printf("GOTOV sa prekidom %d\n", 1);
}
void prekid2()
{
        printf("U prekidu: %d\n", 2);
        for (int i = 0; i < 5; i++)
                sleep(1);
        printf("GOTOV sa prekidom %d\n", 2);
}
void prekid3()
{
        printf("U prekidu: %d\n", 3);
        for (int i = 0; i < 5; i++)
                sleep(1);
        printf("GOTOV sa prekidom %d\n", 3);
}
void prekid4()
{
        printf("U prekidu: %d\n", 4);
        for (int i = 0; i < 5; i++)
                sleep(1);
        printf("GOTOV sa %d\n", 4);
}
void print_kz()
{
        printf("-------------------------------\n");
        printf("Tekuci prioritet: %d, ", trenutni);
        printf("K_z: ");
        for (int i = 1; i < 5; i++) {
                printf("%d", oznaka_cekanja[i]);
        }
        printf("\n");
        printf("Kontekst: ");
        for (int i = 1; i < 5; i++) {
                printf("%d ", kontekst[i]);
        }
        printf("\n");
        printf("-------------------------------\n");
}
