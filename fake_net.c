#include <linux/module.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

// Ponteiro para representar o dispositivo de rede virtual
struct net_device *fake_net;

// Função chamada ao abrir o dispositivo de rede
static int fake_net_open(struct net_device *net_dev)
{
    // Endereço de hardware arbitrário para o dispositivo
    u8 hw_address[6] = { 0x00, 0x12, 0x34, 0x56, 0x78, 0x00 };

    // Imprime informações sobre a abertura do dispositivo
    pr_info("%s - %s(%pK):\n", THIS_MODULE->name, __func__, net_dev);

    // Define o endereço de hardware (dependendo da versão do kernel)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
    eth_hw_addr_set(net_dev, hw_address);
#else
    memcpy(net_dev->dev_addr, hw_address, 6);
#endif

    // Inicia a fila de rede
    netif_start_queue(net_dev);

    return 0;
}

// Função chamada ao parar o dispositivo de rede
static int fake_net_stop(struct net_device *net_dev)
{
    // Imprime informações sobre a parada do dispositivo
    pr_info("%s - %s(%pK):\n", THIS_MODULE->name, __func__, net_dev);

    // Desabilita a transmissão de pacotes
    netif_tx_disable(net_dev);

    return 0;
}

// Função chamada ao iniciar a transmissão de um pacote
static int fake_net_start(struct sk_buff *sk_b, struct net_device *src)
{
    // Imprime informações sobre o início da transmissão
    pr_info("%s -%s(%pK, %pK)\n", THIS_MODULE->name, __func__, sk_b, src);

    // Libera o buffer do pacote
    dev_kfree_skb(sk_b);

    return NETDEV_TX_OK;
}

// Estrutura de operações do dispositivo de rede
static const struct net_device_ops fake_ops = {
    .ndo_open       = fake_net_open,
    .ndo_stop       = fake_net_stop,
    .ndo_start_xmit = fake_net_start,
};

// Função de configuração do dispositivo de rede
static void fake_net_setup(struct net_device *net_dev)
{
    // Imprime informações sobre a configuração do dispositivo
    pr_info("%s - %s(%pK)\n", THIS_MODULE->name, __func__, net_dev);

    // Configuração básica do dispositivo Ethernet
    ether_setup(net_dev);

    // Associa as operações do dispositivo à estrutura
    net_dev->netdev_ops = &fake_ops;
}

// Função chamada durante a inicialização do módulo
static int __init fake_net_init(void)
{
    // Imprime informações sobre a inicialização do módulo
    pr_info("%s - %s()\n", THIS_MODULE->name, __func__);

    // Aloca um dispositivo de rede virtual
    fake_net = alloc_netdev(0, "ex%d", NET_NAME_UNKNOWN, fake_net_setup);

    // Verifica se a alocação foi bem-sucedida
    if (fake_net == NULL)
        return -ENOMEM;

    // Registra o dispositivo de rede virtual
    if (register_netdev(fake_net) != 0) {
        unregister_netdev(fake_net);
        free_netdev(fake_net);
        return -ENODEV;
    }

    return 0;
}

// Função chamada durante a saída do módulo
static void fake_net_exit(void)
{
    // Imprime informações sobre a saída do módulo
    pr_info("%s - %s()\n", THIS_MODULE->name, __func__);

    // Desregistra e libera o dispositivo de rede virtual
    if (fake_net != NULL) {
        unregister_netdev(fake_net);
        free_netdev(fake_net);
    }
}

// Macros para especificar as funções de inicialização e saída do módulo
module_init(fake_net_init)
module_exit(fake_net_exit)

// Informações sobre o módulo
MODULE_DESCRIPTION("Uma implementação de um dispositivo ficticio de rede");
MODULE_AUTHOR("JP");
MODULE_LICENSE("GPL");
