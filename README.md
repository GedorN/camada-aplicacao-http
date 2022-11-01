# Implementação da camada de aplicação HTTP/1.1

Desenvolvido por Anderson Candido(andersoncjs96@gmail.com) e Gedor Neto(gedor.silvaneto@gmail.com)

Implementação da camada de aplicação do protocolo HTTP/1.1. Após aberto um socket com a camada de transporte (TCP),  cada requisição é tratada em uma thread separadamente.

Possui suporte para requisição GET, HEAD. POST com plain text e JSON.

## Para build e execução:
```bash
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
$ ./redes 
```

É necessário ter a biblíoteca Boost na versão 1.80 ou acima para o build do projeto.
