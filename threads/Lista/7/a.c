#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct{
    int R, G, B;
}pixel;

void read(pixel ***pixels, int *l1, int *c1, int *max1, char *cod){
    
    FILE *arq = NULL;
    arq = fopen("in.txt", "rt");
    if(arq == NULL){
        exit(0);
    }

    char str[20];
    int l, c, max, i = 0;

    while(fgets(str, 15, arq) != NULL){
        if(i == 0){
            strcpy(cod, str);
        }
        else if(i == 1){
            c = str[0]-'0';
            l = str[2]-'0';
            *pixels = (pixel **) malloc(l*sizeof(pixel *));
            if(*pixels == NULL){
                exit(-1);
            }
            for(int j=0; j<l; j++){
                (*pixels)[j] = (pixel *) malloc(c*sizeof(pixel));
                if((*pixels)[j] == NULL){
                    exit(-1);
                }
            }
        }
        else if(i == 2){
            max = atoi(str);
            break;
        }
        i++;
    }
    for(int i=0; i<l; i++){
        for(int j=0; j<c; j++){
            int R, G, B;
            fscanf(arq, "%d %d %d", &R, &G, &B);
            (*pixels)[i][j].R = R;
            (*pixels)[i][j].G = G;
            (*pixels)[i][j].B = B;
        }
    }

    *max1 = max; *l1 = l; *c1 = c;

    fclose(arq);
}

void *colorToGrey(void *pixel_){
    pixel *p = (pixel *) pixel_;
    int C = (p->R)*0.3+(p->G)*0.59+(p->B)*0.11;
    p->R = C; p->G = C; p->B = C;
    pthread_exit(NULL);
}

void printPixels(pixel **pixels, int l, int c){
    for(int i=0; i<l; i++){
        for(int j=0; j<c; j++){
            printf("[%d, %d, %d]  ", pixels[i][j].R, pixels[i][j].G, pixels[i][j].B);
        }
        printf("\n");
    }
}

void thread(pixel **pixels, int l, int c){
    int num_threads = l*c;

    pthread_t **threads = NULL;
    threads = (pthread_t **) malloc(l*sizeof(pthread_t *));
    if(threads == NULL){
        exit(-2);
    }
    for(int i=0; i<l; i++){
        threads[i] = (pthread_t *) malloc(c*sizeof(pthread_t));
        if(threads[i] == NULL){
            exit(-2);
        }
    }
    

    for(int i=0; i<l; i++){
        for(int j=0; j<c; j++){
            int ret = pthread_create(&threads[i][j], NULL, colorToGrey, (void *) &pixels[i][j]);
        }
    }

    for(int i=0; i<l; i++){
        for(int j=0; j<c; j++){
            pthread_join(threads[i][j], NULL);
        }
    }

    for(int i=0; i<l; i++){
        free(threads[i]);
    }
    free(threads);

}

void write(pixel **pixels, int l, int c, int max, char *cod){
    FILE *arq = NULL;
    arq = fopen("out.txt", "wt");
    if(arq == NULL){
        exit(-3);
    }
    fprintf(arq, "%s", cod);
    fprintf(arq, "%d %d\n", c, l);
    fprintf(arq, "%d\n", max);
    for(int i=0; i<l; i++){
        for(int j=0; j<c; j++){
            fprintf(arq, "%d %d %d\n", pixels[i][j].R, pixels[i][j].G, pixels[i][j].B);
        }
    }
    fclose(arq);
}


int main (){   
  
    pixel **pixels = NULL;
    int l, c, max;
    char cod[5];

    read(&pixels, &l, &c, &max, cod);

    printPixels(pixels, l, c);
    printf("\n");

    thread(pixels, l, c);
    
    printPixels(pixels, l, c);

    write(pixels, l, c, max, cod);
    
    for(int i=0; i<l; i++){
        free(pixels[i]);
    }
    free(pixels);

    // chamando exit(0) ao invés de pthread_exit pois com exit(0) estamos explicitamente terminando o processo.
    exit(0); 

    return 0;
}