# Soluções para a Lista de Threads
[Link](https://docs.google.com/document/d/1GtarXAcxEVgvUsS3mW_Zwt7xRa5frm1oUq_fQcRl59I/edit)

## Como compilar cada arquivo
```gcc a.c -lpthread -fsanitize=thread -o a```
A flag `-fsanitize=thread` vai testar o código em execução para alguma race condition.

## Problemas
- [1](tree/master/threads/Lista/1)
- [6 - Blocking Queue](tree/master/threads/Lista/6)