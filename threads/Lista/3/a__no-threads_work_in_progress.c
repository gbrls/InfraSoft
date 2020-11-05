#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Considerações
   - n1 mod p = 0
   - n2 < n1/p
   - s1 é uniformemente particionada
*/

// Como o enunciado não especifica se quer input dousuŕaio, definimos pré-definidos de n1, n2, p
#define n1 100
#define p 1
#define n2 100//(n1/p)-1

int get_string_size(char string[]){
    // retorna o tamanho da string (sem contar com o '\0')
    int counter = 0;
    while(string[counter] != '\0'){
        counter++;
    }
    return ++counter;
}

int quantidade_substring(char s1[], char s2[]){
    // Retorna a quantidade de substrings em s1 que são iguais à s2
    int quantidade_de_substrings = 0;
    int s1_size = get_string_size(s1); 
    int s2_size = get_string_size(s2); 
    int aux_counter;
    int quantitade_de_substrings = 0;
    int substring_test_failed;
    if(s1[0] != '\0'){
        // para cada caractere de s1 <-- IMPLEMENTAR THREADS AQUI
        for(int i = 0; i < s1_size; i++){
            // se o caractere é igual ao primeiro caractere de s2,
            if(s1[i] == s2[0]){
                // cheque repetidamente se o próximo caractere de s1 é igual ao próximo caractere de s2 até terminar. se for tudo igual, incrementa o contador de substrings, senão, não incremente e volta para o for externo.
                substring_test_failed = 0;
                aux_counter = 0;
                for(int j = i; j < s2_size+i-1; j++){
                    if(s1[j]!=s2[aux_counter]){
                        substring_test_failed = 1;
                        j = s2_size+i; 
                    }
                    aux_counter++;
                }
                // se achou substring completa (não falhou o teste), incrementa a quantidade de substrings.
                if(substring_test_failed==0){
                    quantidade_de_substrings++;
                }
            }
        }
        printf("\n");
    }
    else{
        return 0;
    }

    return quantidade_de_substrings;

}

int main(){
    char s1[n1] = "mateus ferreira borges soares\0";
    char s2[n2] = "rre\0";
    
    int quantidade_de_substrings = quantidade_substring(s1, s2);
    printf("%d\n", quantidade_de_substrings);
    return 1;
}