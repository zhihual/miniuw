/* Not sure headfile is enough or not*/
/* our target is to make minimal footprint, but support sta*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/if_arp.h>
#include <linux/usb/hcd.h>

enum hif_usb_drv_unload_state {
	HIF_USB_UNLOAD_STATE_NULL = 0,
	HIF_USB_UNLOAD_STATE_DRV_DEREG,
	HIF_USB_UNLOAD_STATE_TARGET_RESET,
	HIF_USB_UNLOAD_STATE_DEV_DISCONNECTED,
};

struct mini_usbdev_t{
	struct device *dev;
	struct usb_dev *pdev;
	short unsigned int devid;
};


static int hif_usb_unload_dev_num = -1;
static wait_queue_head_t hif_usb_unload_event_wq;
static atomic_t hif_usb_unload_state;
struct mini_usbdev *usb_sc = NULL;
static int is_usb_driver_register = 0;

typedef struct mini_usbdev_t mini_usbdev;

MODULE_DEVICE_TABLE(usb, hif_usb_id_table);

#define FALSE 0
#define TURE  1

/*
 * Disable lpm feature of usb2.0.
 */
static int hif_usb_disable_lpm(struct usb_device *udev)
{
	struct usb_hcd *hcd;
	int ret = -EPERM;
	printk("Enter:%s,Line:%d\n", __func__, __LINE__);
	if (!udev || !udev->bus) {
		printk("Invalid input parameters\n");
	} else {
		hcd = bus_to_hcd(udev->bus);
		if (udev->usb2_hw_lpm_enabled) {
			if (hcd->driver->set_usb2_hw_lpm) {
				ret = hcd->driver->set_usb2_hw_lpm(hcd,
							udev, FALSE);
				if (!ret) {
					udev->usb2_hw_lpm_enabled = FALSE;
					udev->usb2_hw_lpm_capable = FALSE;
					printk("%s: LPM is disabled\n",
								__func__);
				} else {
					printk("%s: Fail to disable LPM\n",
								__func__);
				}
			} else {
				printk("%s: hcd doesn't support LPM\n",
							__func__);
			}
		} else {
			printk("%s: LPM isn't enabled\n", __func__);
		}
	}

	printk("Exit:%s,Line:%d\n", __func__, __LINE__);
	return ret;
}


static int
hif_usb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int ret = 0;
	mini_usbdev* sc = NULL;
	struct usb_device *pdev = interface_to_usbdev(interface);
	int vendor_id, product_id;

	printk("hif_usb_probe\n");
	usb_get_dev(pdev);
	vendor_id = le16_to_cpu(pdev->descriptor.idVendor);
	product_id = le16_to_cpu(pdev->descriptor.idProduct);

	ret = 0;

	sc = kzalloc(sizeof(*sc), GFP_KERNEL);
	if (!sc) {
		ret = -ENOMEM;
		goto err_alloc;
	}

	memset(sc, 0, sizeof(*sc));
	sc->pdev = (void *)pdev;
	sc->dev = &pdev->dev;
	sc->devid = id->idProduct;

	printk("pdev=%x dev=%x devid=%x\n", pdev, pdev->dev, id->idProduct);

	if ((usb_control_msg(pdev, usb_sndctrlpipe(pdev, 0),
			     USB_REQ_SET_CONFIGURATION, 0, 1, 0, NULL, 0,
			     HZ)) < 0) {
		printk("%s[%d]\n\r", __func__, __LINE__);
	}
	usb_set_interface(pdev, 0, 0);
	/* disable lpm to avoid usb2.0 probe timeout */
	hif_usb_disable_lpm(pdev);

	usb_sc = sc;
	return 0;


err_alloc:
	usb_put_dev(pdev);

	return ret;
}


static void hif_usb_remove(struct usb_interface *interface)
{
	//HIF_DEVICE_USB *device = usb_get_intfdata(interface);
	struct usb_device *udev = interface_to_usbdev(interface);
	mini_usbdev *sc = usb_sc;
	

	/* Attach did not succeed, all resources have been
	 * freed in error handler
	 */
	if (!sc)
		return;

	printk("Try to remove hif_usb!\n");

#if 0
	/* disable lpm to avoid following cold reset will
	 *cause xHCI U1/U2 timeout
	 */
	usb_disable_lpm(udev);

	/* wait for disable lpm */
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(msecs_to_jiffies(DELAY_FOR_TARGET_READY));
	set_current_state(TASK_RUNNING);
#endif
	
	usb_put_dev(interface_to_usbdev(interface));
	
    
	kfree(sc);
	usb_sc = NULL;
	printk("hif_usb_remove!!!!!!\n");
}

#define VENDOR_ATHR             0x0CF3

static struct usb_device_id hif_usb_id_table[] = {
	{USB_DEVICE_AND_INTERFACE_INFO(VENDOR_ATHR, 0x9378, 0xFF, 0xFF, 0xFF)},
	{}			/* Terminating entry */
};


struct usb_driver hif_usb_drv_id = {

	.name = "miniuw_usb",
	.id_table = hif_usb_id_table,
	.probe = hif_usb_probe,
	.disconnect = hif_usb_remove,
#if 0
	.suspend = hif_usb_suspend,
	.resume = hif_usb_resume,
	.reset_resume = hif_usb_reset_resume,
#endif

#if 0
	.supports_autosuspend = true,
#endif
};

#define A_MSLEEP(msecs)                                                    \
{                                                                          \
    set_current_state(TASK_INTERRUPTIBLE);                                 \
    schedule_timeout((HZ * (msecs)) / 1000);                               \
    set_current_state(TASK_RUNNING);                                       \
}

int hif_register_driver(void)
{
	int status = 0;
	int probe_wait_cnt = 0;
	is_usb_driver_register = 1;
	//init_waitqueue_head(&hif_usb_unload_event_wq);
	//atomic_set(&hif_usb_unload_state, HIF_USB_UNLOAD_STATE_NULL);
	//usb_register_notify(&hif_usb_dev_nb);
 
    printk("call usb_register...\n");	
	status = usb_register(&hif_usb_drv_id);
    printk("done status=%x\n",status);
	
	/* wait for usb probe done, 2s at most*/
	while(!usb_sc && probe_wait_cnt < 10) {
		A_MSLEEP(200);
		probe_wait_cnt++;
	}

	if (usb_sc && status == 0)
		return 0;
	else
		return -1;
}

void hif_unregister_driver(void)
{
	if (is_usb_driver_register) {
		long timeleft = 0;
		printk("Try to unregister hif_driver\n");


		is_usb_driver_register = 0;
		usb_deregister(&hif_usb_drv_id);
		//usb_unregister_notify(&hif_usb_dev_nb);
		printk("hif_unregister_driver!!!!!!\n");
	}
}


int miniuwdrv_init(void) // kernel initialize function
{
  int i = 0;
  printk(KERN_EMERG "Hello world!\n");
  
  printk(KERN_EMERG "tring to allocate modern device\n");
  hif_register_driver();

  return 0;
}

void miniuwdrv_exit(void) // kernel clean up function
{
  hif_unregister_driver();

  printk("<1>""good bye! and good night!\n");
}



/*OS Frame Work for driver load/unload*/
module_init(miniuwdrv_init); // point to insmod load entry point
module_exit(miniuwdrv_exit); // point to exit entry point

MODULE_LICENSE("GPL"); //GPL licence
MODULE_AUTHOR("coolzh_ts@hotmail.com"); //Author
MODULE_VERSION("1.0");// First version
