#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>            
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int ID,ID2,ID3;
int *t;
int *disk;
int (*ram)[16];
short (*tablica_stranicenja)[16];

void proces_i(int i) {
    printf("Usao sam u proces %d\n",i);
    sleep(2);
    int b = (int)(rand()) % 1024;
    b = b & 0x3FE;
    printf("0x%04X, %d izgenerirao sam ovu memoriju!\n",b,b); 
    printf("t = %d\n",*t);
    *t = *t + 1;
    sleep(1);
}
void brisi(int sig){
    printf("Usao u brisanje\n");
    (void) shmdt((char *) t);
    (void) shmctl(ID, IPC_RMID, NULL);
    printf("Brisanje 1\n");
    (void) shmdt((char *) ram);
    (void) shmctl(ID2, IPC_RMID, NULL);
    printf("Brisanje 2\n");
    (void) shmdt((char *) tablica_stranicenja);
    (void) shmctl(ID3, IPC_RMID, NULL); 
    printf("Brisanje 3\n");
}
int main () {
     
    srand(time(NULL));
    pid_t pid; 
    pid_t pidovi[50];
    int n,m;
    
    ID = shmget(IPC_PRIVATE,sizeof(int), 0600);    
    if (ID == -1) {
        exit(1);
    }
    t = (int *) shmat(ID,NULL,0);
    *t = 0;
    signal(SIGINT,brisi);
    /*
    struct sigaction sa;
    sa.sa_handler = brisi;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    */
      
    scanf("%d %d",&n,&m);
    ID2 = shmget(IPC_PRIVATE,sizeof(int) * 16 * m, 0600);
    if (ID2 == -1) {
        exit(-1);
    }
    ram = shmat(ID2,NULL,0);
    for (int i = 0 ; i < m ; i++) {
        for (int j = 0 ; j < 16; j++) {
            ram[i][j] = 0;
            //printf("%d ",ram[i][j]);
        }
        //printf("\n");
    }

    ID3 = shmget(IPC_PRIVATE,sizeof(short) * 16 * n,0600);
    if (ID3 == -1) {
        exit(-1);
    }
    tablica_stranicenja = shmat(ID3,NULL,0); 
     
    for (int i = 0 ; i < n ;i++) {
        if ((pidovi[i] = fork()) == 0)  {
            for (int j = 0; j < 16; j++) {
                tablica_stranicenja[i][j] = 0; 
                //printf("%d", tablica_stranicenja[i][j]); 
            } 
            //printf("\n");
            proces_i(i); 
            exit(0);
        }
        else {
            printf("Glavna pokrenula %i\n",i);
            sleep(1);
        }
    }

    for (int i = 0; i < n; i++) {
        waitpid(pidovi[i],NULL,0);
    }
     
    sleep(1);
    brisi(0);
    printf("Sve je gotovo\n");
    return 0; 
}

