#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#define MAX_LEN 50


int LEN,bud,brd,bri;
char UMS[MAX_LEN][MAX_LEN],IMS[MAX_LEN][MAX_LEN] = {};
sem_t prazan[MAX_LEN],pun[MAX_LEN],bsem[MAX_LEN],bsem2[MAX_LEN];
int nv[MAX_LEN];
int running = 1,t = 0;
int ul[MAX_LEN], iz[MAX_LEN],ul2[MAX_LEN],iz2[MAX_LEN];
void print() {
    printf("--------------------\n");
    printf("t = %d\n",t);
    printf("UMS[]: ");
    for(int i = 0 ; i < brd ; i++) {
        for (int j = 0; j < LEN ; j++) {
            printf("%c",UMS[i][j]);
        }
        printf(" ");
    }
    printf("\n");
    printf("IMS[]: ");
    for(int i = 0 ; i < bri ; i++) {
        for (int j = 0; j < LEN ; j++) {
            printf("%c",IMS[i][j]);
        }
        printf(" ");
    }
    printf("\n--------------------\n");
}
char proizvedi_clan() {
    int rNo = rand() % 26;
    char c = rNo + 65;
    return c;
}
void *ulazna(void *arg) {
    int J = *(int*)arg;
    while (running) {
        sleep(5);
        int I = rand() % brd;
        //sem_wait(&prazan);  
        char c = proizvedi_clan(),e;
        int vr = rand() % 2;
        vr += 2;
        sleep(vr); // obrada
        
        sem_wait(&bsem[I]);
        e = UMS[I][ul[I]];
        UMS[I][ul[I]] = c;
        ul[I] = (ul[I]+1) % LEN;
        if (ul[I] == iz[I]) {
            iz[I] = (iz[I] + 1) % LEN;
        }
        if (running) printf("U%d, Napravio sam clan %c, pisem u %d\n",J,c,I); 
        if (running) print();
        if (e == '-') {
            sem_post(&pun[I]);
        }
        sleep(1);
        sem_post(&bsem[I]);
    }
    for (int l = 0 ; l < brd; l++) {
        sem_post(&pun[l]);
        sem_post(&bsem[l]);
    } 
 
}
void *radna(void *arg) {
    int I =*(int*)arg;
    //printf("USAO %d \n",I);
    while (running) { sem_wait(&pun[I]); 
        sem_wait(&bsem[I]); 
        char c;
        c = UMS[I][iz[I]]; 
        UMS[I][iz[I]] = '-';
        iz[I] = (iz[I]+1) % LEN;
        if (running) printf("R%d, Prebacujem %c\n",I,c);
        if (running) print();
        sem_post(&bsem[I]); 
        sleep(3); // obrada
        int J = rand() % bri;
        sem_wait(&bsem2[J]);
        if (running) printf("****zapisujem %c u %d****\n",c,J);
        IMS[J][ul2[J]] = c;
        ul2[J] = (ul2[J] + 1) % LEN;
        if (ul2[J] == iz2[J]) {
            iz2[I] = (iz2[J] + 1) % LEN;
        }
        nv[J]++;
        sem_post(&bsem2[J]);
    }
    for (int l = 0 ; l < brd; l++) {
        sem_post(&pun[l]);
        sem_post(&bsem[l]);
    }
    for (int l = 0 ; l < bri; l++) {
        sem_post(&bsem2[l]);
    }
}
void *izlazna(void *arg) { 
    int J =*(int*)arg;
    while (running) {
        sem_wait(&bsem2[J]); 
        if (nv[J] > 1) {
            --nv[J];
            iz2[J] = (iz2[J] + 1) % LEN;
        } 
        char c = IMS[J][iz2[J]]; 
        if (running) {
            if (c == '-') printf("I%d, Ispisujem 0 %d\n",J,iz2[J]);
            else printf("I%d, Ispisujem %c %d\n",J,c,iz2[J]);
        } 
        
        sem_post(&bsem2[J]);
        sleep(5);
    } 
    for (int l = 0 ; l < bri; l++) {
        sem_post(&bsem2[l]);
    }
}
int main() {
    for (int i = 0; i < MAX_LEN; i++) {
        for (int j = 0; j < MAX_LEN; j++) {
            IMS[i][j] = '-';
            UMS[i][j] = '-';
        }
    }
    scanf("%d %d %d %d",&LEN,&bud,&brd,&bri);
    srand(time(NULL));
    /*int  randomNumber = rand() % 26; 
    printf("%d", randomNumber);*/
    for (int i = 0 ; i < brd ; i++) {
        sem_init(&pun[i],0,0);
        sem_init(&bsem[i],0,1);
    }
    for (int i = 0 ; i < bri ; i++) {
        sem_init(&bsem2[i],0,1);
    }
    pthread_t ulTh[bud], rTh[brd], iTh[bri];
    int a = 0;
    char input[10];
    int ind[bud];
    for (int i = 0 ; i < bud; i++) {
        ind[i] = i; 
        pthread_create(&ulTh[i], NULL, ulazna, (void *)&(ind[i])); 
        print();
        t++;
        sleep(2);
    }
    int indexes[brd];
    for (int i = 0 ; i < brd; i++) {
        indexes[i] = i ;
        pthread_create(&rTh[i], NULL, radna, (void *)&(indexes[i]));
        print();
        t++;
        sleep(1);
    }
    int ind2[bri];
    for (int i = 0 ; i < bri; i++) {
        ind2[i] = i ;
        pthread_create(&iTh[i], NULL, izlazna, (void *)&(ind2[i]));
        t++;
        sleep(1);
    }
    for (int i = 0 ; i < 60 ;i++) {
        print();
        t++;
        sleep(1);
    }
    printf("running\n");
    running=0;
    //printf("%d",running);
    for(int i = 0 ;i < bud ;i++)
        pthread_join(ulTh[i], NULL);
    for (int i = 0; i < brd; i++)
        pthread_join(rTh[i], NULL);
    for (int i = 0; i < bri; i++)
        pthread_join(iTh[i], NULL);
    for (int i = 0 ; i < brd ; i++) {
        sem_destroy(&pun[i]);
        sem_destroy(&bsem[i]);
    }
    for (int i = 0 ; i < bri ; i++) {
        sem_destroy(&bsem2[i]);
    }
    printf("KRAJ");
    return 0;
}
