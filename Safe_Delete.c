#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/namei.h>
#include <linux/fsnotify.h>
#include <linux/mount.h>
#include <linux/mutex.h>

#define DEVICE_NAME "delete_monitor_driver"
#define CLASS_NAME "delete_monitor_class"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Seu Nome");
MODULE_DESCRIPTION("Driver de módulo para monitorar a exclusão de arquivos");

static int majorNumber;
static struct class* deleteMonitorClass = NULL;
static struct device* deleteMonitorDevice = NULL;

// Mutex para proteger a manipulação da lista de arquivos
static DEFINE_MUTEX(file_list_mutex);

// Estrutura para armazenar informações sobre arquivos
struct MonitoredFile {
    char* path;
    void* buffer;
};

// Lista de arquivos monitorados
static LIST_HEAD(monitored_files);

static int monitor_file(const char *path);
static void unmonitor_file(const char *path);
static void delete_file_content(struct file *file);

static int dev_open(struct inode *inodep, struct file *filep);
static int dev_release(struct inode *inodep, struct file *filep);

static struct file_operations fops = {
    .open = dev_open,
    .release = dev_release,
};

static int __init deleteMonitor_init(void) {
    // Registro do driver com o kernel
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0) {
        printk(KERN_ALERT "Falha ao registrar o driver com o número %d\n", majorNumber);
        return majorNumber;
    }

    // Registro da classe do dispositivo
    deleteMonitorClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(deleteMonitorClass)) {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Falha ao registrar a classe do dispositivo\n");
        return PTR_ERR(deleteMonitorClass);
    }

    // Registro do dispositivo no sistema
    deleteMonitorDevice = device_create(deleteMonitorClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(deleteMonitorDevice)) {
        class_destroy(deleteMonitorClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Falha ao criar o dispositivo\n");
        return PTR_ERR(deleteMonitorDevice);
    }

    printk(KERN_INFO "Driver de módulo carregado!\n");

    return 0;
}

static void __exit deleteMonitor_exit(void) {
    // Remoção do dispositivo
    device_destroy(deleteMonitorClass, MKDEV(majorNumber, 0));

    // Remoção da classe do dispositivo
    class_unregister(deleteMonitorClass);
    class_destroy(deleteMonitorClass);

    // Desregistro do driver
    unregister_chrdev(majorNumber, DEVICE_NAME);

    printk(KERN_INFO "Driver de módulo descarregado!\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Dispositivo aberto\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Dispositivo liberado\n");
    return 0;
}

static void delete_file_content(struct file *file) {
    mm_segment_t old_fs;
    loff_t pos = 0;

    // Ajustar o modo de segmento para o kernel
    old_fs = get_fs();
    set_fs(get_ds());

    // Preencher o arquivo com zeros
    vfs_write(file, "\0", 1, &pos);

    // Restaurar o modo de segmento
    set_fs(old_fs);
}

// Callback para eventos de exclusão no sistema de arquivos
static void delete_callback(struct fsnotify_group *group, struct inode *inode, struct fsnotify_mark *mark, u32 mask, const void *data, int data_type) {
    const char *path = data;

    printk(KERN_INFO "Arquivo excluído: %s\n", path);

    // Realizar ações específicas no arquivo excluído
    struct file *file = filp_open(path, O_RDWR | O_CREAT, 0644);
    if (IS_ERR(file)) {
        printk(KERN_ALERT "Falha ao abrir o arquivo excluído para preenchimento com zeros\n");
        return;
    }

    // Preencher o conteúdo do arquivo com zeros
    delete_file_content(file);

    // Fechar o arquivo
    filp_close(file, NULL);

    printk(KERN_INFO "Conteúdo do arquivo preenchido com zeros\n");
}

// Marcador de evento de exclusão
static struct fsnotify_group *delete_group;

static int setup_fsnotify(void) {
    int ret;

    // Criar um grupo de notificação para eventos de exclusão
    delete_group = fsnotify_alloc_group(&delete_callback, NULL);
    if (!delete_group) {
        printk(KERN_ALERT "Falha ao alocar grupo de notificação para eventos de exclusão\n");
        return -ENOMEM;
    }

    // Adicionar marcador de evento de exclusão
    ret = fsnotify_add_mark(delete_group, FS_DELETE, NULL, 0, &fops);
    if (ret) {
        printk(KERN_ALERT "Falha ao adicionar marcador de evento de exclusão\n");
        fsnotify_put_group(delete_group);
        return ret;
    }

    return 0;
}

static void cleanup_fsnotify(void) {
    fsnotify_remove_mark(delete_group, &fops);
    fsnotify_put_group(delete_group);
}

module_init(deleteMonitor_init);
module_exit(deleteMonitor_exit);
