#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

struct Node {
    int data;
    struct Node* next;
};

struct LinkedList {
    struct Node* head;
};

void printList(struct LinkedList* list) {
    struct Node* current = list->head;
    if (current == NULL) {
        printf("EMPTY\n");
        return;
    }
    while (current->next != NULL) {
        printf("%d -> ", current->data);
        current = current->next;
    }
    printf("%d\n", current->data);
}

void insertAtBeginning(struct LinkedList* list, int data) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (newNode == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = list->head;
    list->head = newNode;
}

void insertAtEnd(struct LinkedList* list, int data) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (newNode == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;
    if (list->head == NULL) {
        list->head = newNode;
        return;
    }
    struct Node* current = list->head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
}

void deleteAtIndex(struct LinkedList* list, int index) {
    if (list->head == NULL) {
        return;
    }
    if (index == 0) {
        struct Node* temp = list->head;
        list->head = list->head->next;
        free(temp);
        return;
    }
    struct Node* current = list->head;
    int pos = 0;
    while (current->next != NULL && pos < index - 1) {
        current = current->next;
        pos++;
    }
    if (current->next == NULL || pos < index - 1) {
        printf("index out of range\n");
        return;
    }
    struct Node* temp = current->next;
    current->next = current->next->next;
    free(temp);
}

int getValueAt(struct LinkedList* list, int index) {
    struct Node* current = list->head;
    int pos = 0;
    while (current != NULL && pos < index) {
        current = current->next;
        pos++;
    }
    if (current == NULL) {
        printf("Index out of range\n");
        return -1;
    }
    return current->data;
}

struct LinkedList list;
int brCitaca, brPisaca, brBrisaca, running = 10, t;
int pisacaCeka,brisacaCeka,citacaCeka;
int len;
pthread_mutex_t m;
pthread_cond_t uv1,uv2,uv3;


void print() {
    printf("----------\n");
    printf("t = %d | citaci: %d, pisaci: %d, brisaci: %d\n", t, brCitaca, brPisaca, brBrisaca);
    printf("Lista(LEN: %d): ",len);
    printList(&list);
    printf("----------\n");
}
void upisiUListu(int val, int I) {
    pthread_mutex_lock(&m);
    if (running) printf("t = %d  | Pisac %d zeli dodati element %d na kraj liste.\n",t,I,val);
    pisacaCeka++; 
    while (brPisaca > 0) {
        pthread_cond_wait(&uv1,&m);
    } 
    while (brisacaCeka > 0 || brBrisaca > 0) {
        pthread_cond_wait(&uv3, &m);
    }
    brPisaca++;
    pisacaCeka--;
    pthread_mutex_unlock(&m);
    sleep((int)(5 + (int)(rand() % 6)));
    pthread_mutex_lock(&m);
    insertAtEnd(&list,val);
    len++;
    if (running) printf("t = %d | Pisac %d je dodao element %d na kraj liste.\n",t,I,val); 
    --brPisaca;
    if (brPisaca == 0) 
        pthread_cond_signal(&uv3);
    if (brPisaca == 0 && pisacaCeka > 0)
        pthread_cond_signal(&uv1); 
    pthread_mutex_unlock(&m);
}

void* glavna_pisac(void* arg) {
    int I = *((int*)arg);
    while(running) {
        int val = rand() % 100 + 1;
        upisiUListu(val, I) ;
        sleep((int)(5 +(int)(rand() % 6)));   
    }
    printf("KRAJ\n");
    pthread_mutex_lock(&m);
    pthread_cond_broadcast(&uv1);
    pthread_cond_broadcast(&uv3);
    pthread_mutex_unlock(&m);
    return NULL;
}

void procitaj_element(int val, int I) {
    pthread_mutex_lock(&m);
    if (running) printf("t = %d  | Citac %d zeli procitati index %d.\n",t,I,val);
    citacaCeka++; 
    while (brBrisaca > 0 || brisacaCeka > 0) {
        pthread_cond_wait(&uv2,&m);
    }  
    citacaCeka--;
    brCitaca++;
    pthread_mutex_unlock(&m);
    sleep((int)(2 + (int)(rand() % 2)));   
    pthread_mutex_lock(&m);
    int a; 
    brCitaca--;
    a = getValueAt(&list, val);
    if (running) printf("t = %d  | Citac %d procitao element %d.\n",t,I,a);  
    if (brCitaca == 0) {
        pthread_cond_signal(&uv2);
    }
    pthread_mutex_unlock(&m);
}

void* glavna_citac(void* arg) {
    int I = *(int*)arg;
    while (running) {
        int val = rand() % len;
        procitaj_element(val,I);  
        sleep((int)(5 + (int)(rand() % 6)));   
    }
    printf("KRAJ3\n");
    pthread_mutex_lock(&m);
    pthread_cond_broadcast(&uv2);
    pthread_mutex_unlock(&m);
}

void obrisi_element(int val, int I) {
    pthread_mutex_lock(&m);
    if (running) printf("t = %d  | Brisac %d zeli obrisati index %d.\n",t,I,val);
    brisacaCeka++;
    while (brCitaca > 0) {
        pthread_cond_wait(&uv2, &m);
    }
    while (brPisaca > 0) {
        pthread_cond_wait(&uv3, &m);
    }  
    brisacaCeka--;
    brBrisaca++;
    pthread_mutex_unlock(&m);
    sleep((int)(3 + (int)(rand() % 4)));   
    pthread_mutex_lock(&m);
    brBrisaca--;
    deleteAtIndex(&list, val);
    --len;
    if (running) printf("t = %d  | Brisac %d obrisao index %d.\n",t,I,val);
    if (brBrisaca == 0) {
        pthread_cond_broadcast(&uv3);
        pthread_cond_broadcast(&uv2);
    }
    pthread_mutex_unlock(&m);
}

void* glavna_brisac(void* arg) {
    int I = *(int*)arg;
    while (running) {
        int val = rand() % len;
        obrisi_element(val,I);  
        sleep((int)(2 + (int)(rand() % 2)));   
    }
printf("KRAJ2\n");
    pthread_mutex_lock(&m);
    pthread_cond_broadcast(&uv2);
    pthread_cond_broadcast(&uv3);
    pthread_mutex_unlock(&m);
}
int main() {
    srand(time(NULL));
    list.head = NULL;
    printList(&list);
    printf("1\n");
    pthread_t p[10],c[10],b;
    int ind[10];
    pthread_mutex_init(&m, NULL);
    pthread_cond_init(&uv1, NULL);
    pthread_cond_init(&uv2, NULL);
    for (int i = 0; i < 3; i++) {
        ind[i] = i;
        if (pthread_create(&p[i], NULL, glavna_pisac, (void*)&(ind[i])) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        print();
        t++;
        sleep(1);
    }
    for (int i = 0 ; i < 5 ;i++) {
        print();
        t++;
        sleep(1);
    }
    int ind2[10];
    for (int i = 0; i < 5; i++) {
        ind2[i] = i;
        if (pthread_create(&c[i], NULL, glavna_citac, (void*)&(ind2[i])) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        print();
        t++;
        sleep(1);
    }
 
    int ind3[10];
    ind3[0] = 0;
    if (pthread_create(&b, NULL, glavna_brisac, (void*)&(ind3[0])) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    print();
    t++;
    sleep(1); 

    for (int i = 0; i < 100; i++) {
        print();
        t++;
        sleep(1);
    }
    running = 0;

    for (int i = 0; i < 2; i++) {
        pthread_join(p[i], NULL);
        t++;
    }
    for (int i = 0; i < 5; i++) {
        pthread_join(c[i], NULL);
        t++;
    }
    pthread_join(b,NULL);
    printf("FINALE\n");
    pthread_mutex_destroy(&m);
    pthread_cond_destroy(&uv1);
    pthread_cond_destroy(&uv2);
    return 0;
}
