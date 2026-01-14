#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MOI");
MODULE_DESCRIPTION("Module avec overflow de tableau");
MODULE_VERSION("1.0");

static int my_array[20];
static int i;

static int __init crash_init(void)
{
    printk(KERN_INFO "=== DEBUT OVERFLOW ===\n");
    
    /* Overflow volontaire : écrit au-delà du tableau */
    for (i = 0; i <= 50; i++) {
        my_array[i] = i * 2;
        printk(KERN_INFO "my_array[%d] = %d\n", i, my_array[i]);
    }
    
    printk(KERN_INFO "=== FIN OVERFLOW ===\n");
    return 0;
}

static void __exit crash_exit(void)
{
    printk(KERN_INFO "Nettoyage du module crash\n");
    for (i = 0; i < 30; i++) {
        printk(KERN_INFO "valeur my_array[%d] : %d\n", i, my_array[i]);
    }
}

module_init(crash_init);
module_exit(crash_exit);