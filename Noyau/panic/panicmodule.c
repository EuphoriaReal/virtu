#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MOI");
MODULE_DESCRIPTION("Declenchement kernel panic");
MODULE_VERSION("1.0");

static int __init panic_init(void)
{
    
    /* appel direct à panic() */
    panic("Kernel panic !");
    
    return 0;
}

static void __exit panic_exit(void)
{
    printk(KERN_INFO "Jamais execute\n");
}

module_init(panic_init);
module_exit(panic_exit);