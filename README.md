# Aplicação Cliente/Servidor de Chat em TCP/UDP

O sistema consiste em um servidor que pode aceitar múltiplos clientes e realizar troca de mensagens entre eles, incluindo o envio de arquivos, utilizando notificações para informar o início e fim da transferência.

- **Cliente**: responsável por interpretar os comandos do usuário, estabelecer comunicação com o servidor e exibir as informações da aplicação para o usuário.
  
- **Servidor**: atua como um hub central, gerenciando a aplicação. Ele registra os usuários, gerencia as conexões e transações entre os clientes, permitindo a troca de mensagens e arquivos.

## Funcionamento TCP

O TCP (Transmission Control Protocol) é um protocolo orientado à conexão, ou seja, garante a entrega confiável dos dados entre o cliente e o servidor ao estabelecer uma conexão antes de iniciar a transmissão.

1. **Servidor:**
   - O servidor é configurado para escutar conexões na porta 8081. Ele usa a função `socket()` para criar um socket, `bind()` para associar o socket a um endereço IP e porta específicos, e `listen()` para colocar o socket em modo de escuta.
   - Ao receber uma conexão de um cliente via `accept()`, ele cria uma nova thread para lidar com cada cliente, garantindo que múltiplos clientes possam se conectar simultaneamente.
   - As mensagens enviadas por um cliente são retransmitidas para todos os outros clientes conectados (broadcast).
   - O servidor também é capaz de receber arquivos através de notificações específicas.

2. **Cliente:**
   - O cliente se conecta ao servidor especificando o endereço IP do servidor e a porta 8081.
   - O cliente pode enviar mensagens de texto ao servidor, que as retransmite para outros clientes.
   - O cliente também pode iniciar a transferência de arquivos, utilizando um comando específico (`/FILE`) para notificar o servidor do início da transferência.

 
 ## Funcionamento UDP
O UDP (User Datagram Protocol) é um protocolo não orientado à conexão, o que significa que não há garantia de entrega dos pacotes de dados, nem de sua ordem. Isso torna o UDP mais rápido e eficiente em comparação ao TCP, embora não seja ideal para todas as aplicações que requerem confiabilidade.

1. **Servidor:**
   - O servidor é configurado para escutar mensagens na porta 8082 utilizando sockets UDP. Ele cria um socket com a função `socket()`, associa o socket a um endereço IP e porta específicos com `bind()`, e aguarda por mensagens de clientes usando `recvfrom()`.
   - Ao receber uma mensagem, o servidor processa a entrada (por exemplo, convertendo o texto para caixa alta) e envia a resposta de volta ao cliente, utilizando `sendto()`.
   - O servidor pode atender múltiplos clientes de forma concorrente, mas sem o overhead de gerenciamento de conexões, permitindo uma comunicação rápida e eficiente.

2. **Cliente:**
   - O cliente se conecta ao servidor UDP especificando o endereço IP do servidor e a porta 8082.
   - O cliente pode enviar mensagens de texto ao servidor usando `sendto()`, e aguardar a resposta do servidor com `recvfrom()`. 
   - O cliente não precisa estabelecer uma conexão antes de enviar mensagens, tornando a comunicação mais leve.
   - Para transferências de arquivos, o cliente deve gerenciar o envio em partes, utilizando uma lógica própria para dividir o arquivo e notificar o servidor sobre o início e o fim da transferência.

## Compilação 

Certifique-se de ter as seguintes dependências instaladas:

- **g++** (compilador C++)
- **make** (ferramenta para automatizar a compilação)

Você pode instalar as ferramentas necessárias no **Debian/Ubuntu** usando o seguinte comando:

```bash
sudo apt update && sudo apt install build-essential
```

Para compilar o projeto, abra um terminal e navegue até o diretório onde o Makefile está localizado. Em seguida, execute o seguinte comando:

```bash
make
```

Este comando irá compilar todos os executáveis: `client_tcp`, `client_udp`, `server_tcp` e `server_udp`. 
Os binários compilados serão armazenados no diretório `bin`.

### Limpeza do Projeto

Se você precisar limpar os binários compilados, pode usar o comando:

```bash
make clean
```

### Observações

- Para executar o projeto em **Windows**, você precisará modificar o código fonte para usar a biblioteca Winsock.
- As instruções de compilação são aplicáveis a sistemas Unix-like (Linux, macOS). Para compilar no Windows, utilize uma ferramenta como Cygwin ou WSL.
