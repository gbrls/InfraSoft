#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



pthread_mutex_t *mut;
char **matriz;
char colors[8][10] = {"\e[41m", "\e[43m", "\e[44m", "\e[45m", "\e[42m", "\e[40m", "\e[46m", "\e[47m"};
char reset[10] = "\e[0m";

typedef struct{
    int n;
    int num_linhas;
    char **s;
} mat;

typedef struct{
    int i;
    int num_linhas;
} aux_;

void printa_matriz(int num_linhas){
    system("clear");
    for(int i=0; i<num_linhas; i++){
        printf("%s" "%s" "%s", colors[i], matriz[i], reset);
    }
}

void *func(void *k){
    aux_ *a = (aux_ *) k;
    int t = a->i;
    int num_linhas = a->num_linhas;
    mat aux;
    char buffer[7] = {};
    sprintf(buffer, "%d.txt", t+1);
    FILE *arq = NULL;
    arq = fopen(buffer, "rt");
    if(arq == NULL){
        exit(-1);
    }

    char str[40];
    int j=0;
    aux.s = NULL;
    while(fgets(str, 40, arq) != NULL){
        aux.s = (char **) realloc(aux.s, (j+1)*sizeof(char *));
        aux.s[j] = (char *) malloc(40*sizeof(char));
        strcpy(aux.s[j], str);
        j++;
    }
    aux.n = j;

    free(arq);

    int n = aux.n;
    for(int i=0; i<n; i+=2){
        int l = atoi(aux.s[i]);
        l--;
        pthread_mutex_lock(&mut[l]);
        //while(pthread_mutex_trylock(&mut[l]) != 0){;}
        strcpy(matriz[l], aux.s[i+1]);
        pthread_mutex_unlock(&mut[l]);
    }
    //printa_matriz(num_linhas);

    for(int i=0; i<n; i++){
        free(aux.s[i]);
    }
    free(aux.s);

    pthread_exit(NULL);
}

void thread(int num_threads, int num_linhas){

    pthread_t *threads = NULL;
    threads = (pthread_t *) malloc(num_threads*sizeof(pthread_t));

    aux_ *index = NULL;
    index = (aux_ *) malloc(num_threads*sizeof(aux_));

    for(int i=0; i<num_threads; i++){
        index[i].i = i;
        index[i].num_linhas = num_linhas;
        int ret = pthread_create(&threads[i], NULL, func, (void *) &index[i]);
    }

    for(int i=0; i<num_threads; i++){
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(index);
    
}

void init(int *l, int *n){
    matriz = NULL;
    *l = 7;
    mut = (pthread_mutex_t *) malloc((*l)*sizeof(pthread_mutex_t));
    for(int i=0; i<*l; i++){
        pthread_mutex_init(mut+i, NULL);
    }

    printf("Entre com o numero de arquivos: ");
    scanf("%d", n);
    printf("\n");

    matriz = (char **) malloc((*l+1)*sizeof(char *));
    for(int i=0; i<*l; i++){
        matriz[i] = (char *) malloc(26*sizeof(char));
        for(int j=0; j<24; j++){
            matriz[i][j] = ' ';
        }
        matriz[i][24] = '\n';
        matriz[i][25] = '\0';
    }
}

void free_(int num_linhas){
    for(int i=0; i<=num_linhas; i++){
        free(matriz[i]);
    }
    free(matriz);
}


int main() {

    int num_threads, num_linhas;

    init(&num_linhas, &num_threads);

    thread(num_threads, num_linhas);

    printa_matriz(num_linhas);

    free_(num_linhas);

    exit(0);

    return 0;
}