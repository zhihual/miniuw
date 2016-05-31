/* Not sure headfile is enough or not*/
/* our target is to make minimal footprint, but support sta*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "bmi.h"

int miniuwdrv_init(void) // kernel initialize function
{
  int i = 0;
  printk(KERN_EMERG "Hello world!\n");
  
  printk(KERN_EMERG "tring to allocate modern device\n");
  
  myprint(10);
  return 0;
}

void miniuwdrv_exit(void) // kernel clean up function
{
  printk("<1>""good bye! and good night!\n");
}



/*OS Frame Work for driver load/unload*/
module_init(miniuwdrv_init); // point to insmod load entry point
module_exit(miniuwdrv_exit); // point to exit entry point

MODULE_LICENSE("GPL"); //GPL licence
MODULE_AUTHOR("coolzh_ts@hotmail.com"); //Author
MODULE_VERSION("1.0");// First version
