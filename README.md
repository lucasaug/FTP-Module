# FTP Module

Servidor e cliente simples de uma aplicação FTP. Trabalho prático para a disciplina de Redes de Computadores da Universidade Federal de Minas Gerais. Esse módulo é compatível com IPv6.

## Instalação

O projeto possui um Makefile para o cliente e outro para o servidor. Basta entrar na pasta desejada e utilizar o comando make para compilar cada módulo.


## Utilização

O código da aplicação cliente se encontra na pasta [client](client) e do servidor na pasta [server](server). Os executáveis se encontram na pasta bin de cada diretório. Para utilização do servidor, basta utilizar o seguinte comando:

```
servidorFTP <porta do servidor> <tam_buffer> <diretório a ser utilizado>
```

Onde tam_buffer representa o tamanho em bytes do buffer.

O cliente possui uma operação para listagem dos arquivos no diretório e outra para a transmissão de um arquivo.

Listagem:

```
clienteFTP list <nome ou IPv6 do servidor> <porta do servidor> <tam_buffer>
```

Transmissão de arquivo:

```
clienteFTP get <nome do arquivo> <nome ou IPv6 do servidor> <porta do servidor> <tam_buffer>
```

## Licença

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
