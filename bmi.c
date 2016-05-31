#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "bmi.h"

void myprint(int i)
{
   printk(KERN_EMERG "%d\n", i);
}

