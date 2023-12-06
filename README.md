# Fake Net Kernel Module

Este é um exemplo simples de um módulo de kernel para Linux que cria um dispositivo de rede virtual. O módulo é destinado apenas para fins educacionais e demonstra o básico da criação de módulos de kernel.

## Funcionalidades Principais

O módulo do kernel cria um dispositivo de rede virtual com as seguintes características:

- **Endereço de Hardware (MAC):** O endereço de hardware (MAC address) do dispositivo é configurado durante a inicialização do módulo. A implementação lida com a diferença de versões do kernel para garantir a compatibilidade.

- **Operações do Dispositivo de Rede:** O módulo implementa operações básicas de um dispositivo de rede, como abertura (`open`), parada (`stop`), e início da transmissão (`start`). Estas são manipuladas de maneira simples para fins de demonstração.

## Compilação e Execução

Para compilar o módulo, utilize o seguinte comando:

```bash
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
```

Após a compilação, você pode carregar o módulo com o comando:

```bash
sudo insmod fake_net.ko
```

E verificar os logs do kernel com:

```bash
dmesg | grep fake_net
```

Para remover o módulo, utilize:

```bash
sudo rmmod fake_net
```

## Notas Importantes

- **Aviso:** Este módulo é destinado apenas para fins educacionais. Não deve ser utilizado em ambientes de produção.
  
- **Compatibilidade:** O módulo foi projetado para ser compatível com diferentes versões do kernel, adaptando-se à diferença nas operações de configuração do endereço de hardware.

- **Licença:** Este projeto é distribuído sob a licença [GPL (General Public License)](LICENSE).

## Contribuições e Problemas

Contribuições são bem-vindas! Sinta-se à vontade para abrir uma issue para relatar problemas ou propor melhorias.
