#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

/* Considerações
   - n1 mod p = 0
   - n2 < n1/p
   - s1 é uniformemente particionada
*/

// Como o enunciado não especifica se quer input dousuŕaio, definimos pré-definidos de n1, n2, p
#define n1 100
#define p 4
#define n2 100//(n1/p)-1

typedef struct  dummy_element{
    char s1[n1];
    char s2[n2];
    // int quantity_of_substrings_found;
}element;

typedef struct dummy_element_and_id{
    element *common_element;
    int id;
}element_and_id;

int get_string_size(char string[]){
    // retorna o tamanho da string (sem contar com o '\0')
    int counter = 0;
    while(string[counter] != '\0'){
        counter++;
    }
    return ++counter;
}

void *quantidade_substring(void *argument){
    /* Retorna a quantidade de substrings em s1 que são iguais à s2 */

    // variável de retorno
    int retorno = 0;

    // De-referenciando
    element_and_id *elemento_com_id = (element_and_id *) argument;

    // Declarações e inicializações iniciais
    int quantidade_de_substrings = 0;
    int s1_size = get_string_size(elemento_com_id->common_element->s1); 
    int s2_size = get_string_size(elemento_com_id->common_element->s2); 
    int aux_counter;
    int quantitade_de_substrings = 0;
    int substring_test_failed;
    if(elemento_com_id->common_element->s1[0] != '\0'){
        // para cada caractere de s1 <-- IMPLEMENTAR THREADS AQUI
        for(int i = elemento_com_id->id; i < s1_size; i += p){
            // se o caractere é igual ao primeiro caractere de s2,
            if(elemento_com_id->common_element->s1[i] == elemento_com_id->common_element->s2[0]){
                // cheque repetidamente se o próximo caractere de s1 é igual ao próximo caractere de s2 até terminar. se for tudo igual, incrementa o contador de substrings, senão, não incremente e volta para o for externo.
                substring_test_failed = 0;
                aux_counter = 0;
                for(int j = i; j < s2_size+i-1; j++){
                    if(elemento_com_id->common_element->s1[j] != elemento_com_id->common_element->s2[aux_counter]){
                        substring_test_failed = 1;
                        j = s2_size+i; 
                    }
                    aux_counter++;
                }
                // se achou substring completa (não falhou o teste), incrementa a quantidade de substrings.
                if(substring_test_failed==0){
                    // quantidade_de_substrings++;
                    printf("thread_id: %d\n", elemento_com_id->id);
                    // elemento_com_id->common_element->quantity_of_substrings_found++;
                    // return 1;
                    retorno += 1;
                }
            }
        }
    }
    return (void*)retorno;
}

int main(){
    // Declarações e inicializações iniciais
        // a struct contendo as duas strings e o contador de quantidade de substrings
    element *data = malloc(sizeof(element));
    strcpy(data->s1, "aaaabaaabaa\0");
    strcpy(data->s2, "aaab\0");
    // data->quantity_of_substrings_found = malloc(sizeof(int));
        // creates all the element_and_id's and already setting its id and common_element (puts it all in one array)
    element_and_id *element_and_id_array[p];
            // for each thread, assigns it with an id ranging from 1 to p
            
    puts("Alocando element and id");
    fflush(stdout);
    for(int i = 0; i < p; i++){
        //quantidade_final += *((int *)(*array_of_returns));
        element_and_id_array[i] = (element_and_id *) malloc(sizeof(element_and_id)); 
        element_and_id_array[i]->common_element = data;
        element_and_id_array[i]->id = i;
    }

    puts("element and id alocados");
    fflush(stdout);

    // pega o tamanho das duas strings
    int s1_size = get_string_size(data->s1); 
    int s2_size = get_string_size(data->s2); 
        // array com as variáveis do tipo thread
    pthread_t threads[p];

    // Chamadas de função e administração de threads
    // para cada variável pthread, cria uma tread rodando a função quantidade_substring()
    for(int i = 0; i < p; i++){
        pthread_create((void *)&threads[i], NULL, quantidade_substring, (void *) element_and_id_array[i]);
    }

    puts("Alocado tudo");
    fflush(stdout);
        // faz a main esperar a execução de todas as threads antes de continuar a sua própria execução
    void* array_of_returns[p];
    for(int i = 0; i < p; i++){
        pthread_join(threads[i], &array_of_returns[i]);
    }

    int quantidade_final = 0;
    for(int i = 0; i < p; i++){
        //quantidade_final += *((int *)(*array_of_returns));
        printf("%d%c", (int)array_of_returns[i], i==p-1?'\n':' ');
        quantidade_final += (int)array_of_returns[i];
    }

    // Print do resultado final (quantas substrings iguais a s2 existem em s1)
    printf("\nQuantidade de substrings encontradas: %d\n\n", quantidade_final); // TODO: isso printa!


    for(int i = 0; i < p; i++){
        free(element_and_id_array[i]);
    }

    free(data);

    return 1;
}