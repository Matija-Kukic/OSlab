#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

struct sigaction prije;
#define MAXARGS 5
#define MAXPROG 12
int pid_proces[MAXPROG];
char ime_procesa[MAXPROG][10];
void obradi_dogadjaj(int sig)
{
        printf("\n[signal SIGINT] proces %d primio signal %d\n", (int)getpid(),
               sig);
        // proslijedi ga ako se program izvodi u prvom planu
}
void obradi_signal_zavrsio_neki_proces_dijete(int id)
{
        // ako je već dole pozvan waitpid, onda na ovaj signal waitpid ne daje
        // informaciju (ponovo)
        pid_t pid_zavrsio = waitpid(-1, NULL, WNOHANG); // ne čeka
        if (pid_zavrsio > 0)
                if (kill(pid_zavrsio, 0) ==
                    -1) { // možda je samo promijenio stanje ili je bas završio
                        printf("\n[roditelj %d - SIGCHLD + waitpid] dijete %d "
                               "zavrsilo s radom\n",
                               (int)getpid(), pid_zavrsio);
                        int c = 0;
                        while (pid_proces[c] != pid_zavrsio) {
                                c++;
                        }
                        pid_proces[c] = 0;

                        for (int i = 0; ime_procesa[c][i] != '\0'; i++)
                                ime_procesa[c][i] = '\0';
                        // printf("DEBUG 3: %s %d\n", ime_procesa[c],
                        // pid_proces[c]);
                }
        // else
        // printf("\n[roditelj %d - SIGCHLD + waitpid] waitpid ne daje
        // informaciju\n", (int) getpid());
}

// primjer stvaranja procesa i u njemu pokretanja programa
pid_t pokreni_program(char *naredba[], int u_pozadini)
{
        pid_t pid_novi;
        if ((pid_novi = fork()) == 0) {
                printf("[dijete %d] krenuo s radom\n", (int)getpid());
                sigaction(SIGINT, &prije, NULL); // resetiraj signale
                setpgid(pid_novi, pid_novi); // stvori novu grupu za ovaj proces
                if (!u_pozadini)
                        tcsetpgrp(STDIN_FILENO,
                                  getpgid(pid_novi)); // dodijeli terminal

                execvp(naredba[0], naredba);
                perror("Nisam pokrenuo program!");
                exit(1);
        }

        return pid_novi; // roditelj samo dolazi do tuda
}

int main()
{
        struct sigaction act;
        pid_t pid_novi;
        // ostale varijable su definirane neposredno prije koristenja
        // ali SAMO RADI jednostavnijeg praćenja
        // uobicajeno se sve varijable deklariraju ovdje!

        // printf("[roditelj %d] krenuo s radom\n", (int)getpid());
        char dir[4096];
        getcwd(dir, 4096);
        // printf("[%s] $: \n", dir);

        // postavi signale SIGINT i SIGCHLD
        act.sa_handler = obradi_dogadjaj;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGINT, &act, &prije);
        act.sa_handler = obradi_signal_zavrsio_neki_proces_dijete;
        sigaction(SIGCHLD, &act, NULL);
        act.sa_handler = SIG_IGN;
        sigaction(SIGTTOU, &act, NULL); // zbog tcsetpgrp

        struct termios shell_term_settings;
        tcgetattr(STDIN_FILENO, &shell_term_settings);

        // primjer stvaranja procesa i u njemu pokretanja programa
        /*char *naredba_echo[] = {"echo", "-e", "Jedan\nDva\nTri", NULL};
        pid_novi = pokreni_program(naredba_echo, 0);
        waitpid(pid_novi, NULL, 0); // čekaj da završi
        */
        // uzmi natrag kontrolu nad terminalom
        tcsetpgrp(STDIN_FILENO, getpgid(0));

        size_t vel_buf = 128;
        char buffer[vel_buf];

        do {
                // unos teksta i parsiranje
                char dir[4096];
                getcwd(dir, 4096);
                printf("[%s] $: ", dir);

                if (fgets(buffer, vel_buf, stdin) != NULL) {
                        char *argv[MAXARGS];
                        int argc = 0;
                        argv[argc] = strtok(buffer, " \t\n");
                        while (argv[argc] != NULL) {
                                argc++;
                                argv[argc] = strtok(NULL, " \t\n");
                        }
                        printf("DEBUG: ");
                        for (int i = 0; argv[i] != NULL && i < 5; i++) {
                                printf("%s ", argv[i]);
                        }
                        printf("\n");
                        // pokretanje "naredbe" (pretpostavljam da je program)
                        // printf("[roditelj] pokrecem program\n");
                        if (strncmp(argv[0], "cd", 2) == 0) {
                                // Handle 'cd' command
                                char *path = strtok(argv[1], " \t\n");
                                if (path == NULL) {
                                        fprintf(stderr,
                                                "Usage: cd <directory>\n");
                                } else {
                                        if (chdir(path) != 0) {
                                                perror("chdir");
                                        }
                                }
                        } else if (strncmp(argv[0], "ps", 2) == 0) {
                                printf("----------------------------------\n%"
                                       "10s%10s\n",
                                       "IME", "PID");
                                for (int i = 0; i < 12; i++) {
                                        if (pid_proces[i] != 0)
                                                printf("%10s%10d\n",
                                                       ime_procesa[i],
                                                       pid_proces[i]);
                                }
                                printf("----------------------------------\n");
                        } else if (strncmp(argv[0], "exit", 4) == 0) {
                                printf("Gasim sve podprograme!\n");
                                for (int i = 0; i < 12; i++) {
                                        if (pid_proces[i] != 0) {
                                                kill(pid_proces[i], SIGINT);
                                        }
                                }
                        } else if (strncmp(argv[0], "kill", 4) == 0) {
                                printf("Saljem prekid na %s\n", argv[1]);
                                int z = 0;
                                for (int i = 0; i < 12; i++) {
                                        if (pid_proces[i] == atoi(argv[1])) {
                                                kill(atoi(argv[1]),
                                                     atoi(argv[2]));
                                                z++;
                                        }
                                }
                                if (!z)
                                        printf("Nista nije prekinuto, proces "
                                               "nije od ljuske.\n");
                        } else if (strncmp(argv[argc - 1], "&", 1) != 0) {
                                pid_novi = pokreni_program(argv, 0);
                                int c = 0;
                                int len = 0;
                                while (pid_proces[c] != 0) {
                                        c++;
                                }
                                while (argv[0][len] != '\0')
                                        len++;
                                pid_proces[c] = pid_novi;
                                for (int i = 2; i <= len; i++) {
                                        ime_procesa[c][i - 2] = argv[0][i];
                                }
                                // printf("DEBUG 2: %s %d\n", ime_procesa[c],
                                // pid_proces[c]);
                                printf("cekam da zavrsi\n");
                                pid_t pid_zavrsio;
                                do {
                                        pid_zavrsio =
                                            waitpid(pid_novi, NULL, 0); // čekaj
                                        if (pid_zavrsio > 0) {
                                                if (kill(pid_novi, 0) ==
                                                    -1) { // nema ga više? ili
                                                          // samo mijenja stanje
                                                        printf("dijete %d "
                                                               "zavrsilo s "
                                                               "radom\n",
                                                               pid_zavrsio);

                                                        // vraćam terminal
                                                        // ljusci
                                                        tcsetpgrp(STDIN_FILENO,
                                                                  getpgid(0));
                                                        tcsetattr(
                                                            STDIN_FILENO, 0,
                                                            &shell_term_settings);
                                                } else {
                                                        pid_novi =
                                                            (pid_t)0; // nije
                                                                      // gotov
                                                }
                                        } else {
                                                printf(" waitpid gotov ali "
                                                       "ne daje informaciju\n");
                                                break;
                                        }
                                } while (pid_zavrsio <= 0);
                                pid_proces[c] = 0;
                                for (int i = 0; ime_procesa[c][i] != '\0'; i++)
                                        ime_procesa[c][i] = '\0';
                                // printf("DEBUG 3: %s %d\n", ime_procesa[c],
                                // pid_proces[c]);
                        } else {
                                pid_novi = pokreni_program(argv, 1);
                                int c = 0;
                                int len = 0;
                                while (pid_proces[c] != 0) {
                                        c++;
                                }
                                while (argv[0][len] != '\0')
                                        len++;
                                pid_proces[c] = pid_novi;
                                for (int i = 2; i <= len; i++) {
                                        ime_procesa[c][i - 2] = argv[0][i];
                                }
                                // printf("DEBUG 2: %s %d\n", ime_procesa[c],
                                // pid_proces[c]); printf("DEBUG 2: %d\n",
                                // pid_proces[c]);
                        }

                } else {
                        // printf("[roditelj] neka greska pri unosu, vjerojatno
                        // dobio signal\n");
                }
        } while (strncmp(buffer, "exit", 4) != 0);

        return 0;
}
