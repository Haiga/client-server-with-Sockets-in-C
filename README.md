# ClienteSocketC

Servidor para comunicação via Socket que implementa Produtor-Consumidor

## Getting Started

Implementação de um servidor para comunicação de chat de mensagens.
Implementa a estratégia do Produtor-Consumidor: Cada cliente conectado gera uma thread consumidora, cuja mensagem será adicionada em um buffer e replicada para cada cliente conectado.
A comunicação pode ser utilizada com o cliente para Android, produzido com Kotlin, disponível em:
https://github.com/Haiga/MyChatApp
A comunicação também pode ser utilizada com o cliente.cpp

### Prerequisites

Mínima versão requirida cmake: 3.14
Instale através:
```
sudo apt-get install cmake
```

### Running

```
cd ClienteSocketC
g++ servidor.cpp -o servidor -pthread
```

## Built With

* [nlohmann/json](https://github.com/nlohmann/json) - JSON for Modern C++
