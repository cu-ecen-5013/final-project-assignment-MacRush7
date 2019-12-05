/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes and Sarah Stephany
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h> 		// file_operations
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include "aesdchar.h"
#include "aesd_ioctl.h"

int writeHead = 0, readTail = 0;
int aesd_major =   0; 		// use dynamic major
int aesd_minor =   0;
char *checkRealloc = 0;
int rdcount = 0;

MODULE_AUTHOR("Sarah Stephany");
MODULE_LICENSE("Dual BSD/GPL");

DEFINE_MUTEX(lock);

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
	PDEBUG("open start");
	
	// device info
	struct aesd_dev *aesdData;

	// find struct	
	aesdData = container_of(inode->i_cdev, struct aesd_dev, cdev);

	// give device access	
	filp->private_data = aesdData;
	
	PDEBUG("open stop");
	return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	PDEBUG("read start");
	struct aesd_dev *dev = filp->private_data;
	int i, b = 0, tmp_read, tmpcount, offset = 0, cmd = 0;
	loff_t tempPos = 0;
	ssize_t readRetval = 0;
	
	PDEBUG("READ f_pos: %ld", *f_pos);
	PDEBUG("READ writeHead: %d and readTail: %d", dev->writeHead, dev->readTail);

	// lock
	mutex_lock_interruptible(&lock);
	PDEBUG("READ after lock");

	// check if read done
	if(dev->readCount > 0)
	{
		// reset readCount (count of characters read)
		dev->readCount = 0;
		PDEBUG("return 0");
		mutex_unlock(&lock);
		return 0;
	}

	// check if circular buffer empty
	dev->test = dev->circularBuffer[dev->readTail];
	if(!dev->test)
	{
		printk("circular buffer empty");
		PDEBUG("circular buffer empty");
		mutex_unlock(&lock);
		return -EAGAIN;
	}

	// check if NULL pointer
	if (dev == NULL || !dev->circularBuffer[dev->readTail])
	{
		PDEBUG("NULL pointer");
		goto out;
	}
	
	// find f_pos
	while((offset == 0))
	{
		PDEBUG("inside while");
		tmpcount = dev->countBuffer[b];
		tempPos += tmpcount;
		if((tempPos > *f_pos) && (b == 0))
		{
			PDEBUG("inside while if");
			offset = *f_pos;
		}
		else if ((tempPos > *f_pos) && (b != 0))
		{
			PDEBUG("inside while else if");
			offset = *f_pos % (tempPos - (tmpcount));
			PDEBUG("offset %d, tempPos %ld, tmpcount %d", offset, tempPos, tmpcount);
		}
		else
			PDEBUG("offset %d, tempPos %ld, tmpcount %d", offset, tempPos, tmpcount);
		b++;
	} 
	cmd = b; 
	
	PDEBUG("cmd: %d", cmd);

	tmp_read = readTail + cmd-1;

	for(i = 0; (b-1) < BUFFER_SIZE; i++)
	{
		PDEBUG("inside copy to user loop. offset: %d", offset);

		// read from circular buffer
		if(copy_to_user((buf+(dev->readCount)), (dev->circularBuffer[tmp_read])+offset, (dev->countBuffer[tmp_read])-offset))
		{
			mutex_unlock(&lock);
			PDEBUG("return -EFAULT from copy to user");
			return -EFAULT;
		}
		
		dev->readCount += dev->countBuffer[tmp_read]-offset;	
		PDEBUG("count: %d", dev->readCount);

		PDEBUG("READ circularBuffer[%d] contents: %s", tmp_read, dev->circularBuffer[tmp_read]);

		// advance readTail
		tmp_read = (tmp_read + 1) % BUFFER_SIZE;

		PDEBUG("READ buf contents: %s", buf);
	
		*f_pos = 0;
		offset = 0;
		b++;

		readRetval = dev->readCount;
		PDEBUG("readCount %d", dev->readCount);
	}

	PDEBUG("read stop");
out:
	PDEBUG("read out");
	mutex_unlock(&lock);
	return readRetval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	PDEBUG("write start");
	struct aesd_dev *dev = filp->private_data;
	ssize_t retval = -ENOMEM;
	int i = 0;

	PDEBUG("WRITE flag: %d", dev->flag);
	PDEBUG("WRITE partial_flag: %d", dev->partial_flag);

	mutex_lock_interruptible(&lock);

	PDEBUG("WRITE writeHead: %d and readTail: %d", dev->writeHead, dev->readTail);

	if (dev == NULL)
	{
		PDEBUG("device null");
		goto out;
	}

	// buffer is full, start freeing memory before overwriting
	if((dev->flag == 1) && (dev->partial_flag == 0))
	{
		// update size
		dev->size -= dev->countBuffer[dev->writeHead];

		// free pointer
		PDEBUG("freeing memory if device is not NULL");	
		if(dev->circularBuffer[dev->writeHead] != NULL)
		{	
			kfree(dev->circularBuffer[dev->writeHead]);
			PDEBUG("freed memory");	
		}
	}

	// check if NULL buffer exists... malloc if true
	if ((!dev->circularBuffer[dev->writeHead]) && (dev->partial_flag == 0)) 
	{
		if(checkRealloc == NULL)
		{
			PDEBUG("realloc");
			dev->circularBuffer[dev->writeHead] = (char *)krealloc(dev->circularBuffer[dev->writeHead], (count*(sizeof(char))), GFP_KERNEL);
		}
		else
		{
			PDEBUG("malloc");
			dev->circularBuffer[dev->writeHead] = (char *)kmalloc((count*(sizeof(char))), GFP_KERNEL);
		}
	}
	// partial write... add more memory
	else if (dev->partial_flag == 1)
	{
		PDEBUG("realloc for partial");
		dev->circularBuffer[dev->writeHead] = (char*)krealloc(dev->circularBuffer[dev->writeHead], ((dev->charsRead+count)*(sizeof(char))), GFP_KERNEL);
	}

	// buffer full, move readTail with write
	if((dev->flag == 1) && (dev->writeHead == dev->readTail))
	{
		PDEBUG("WRITE moving tail for full buffer");
		dev->readTail = (dev->readTail + 1) % BUFFER_SIZE;
	}

	// copy from user
	if ((retval = copy_from_user((dev->circularBuffer[dev->writeHead])+dev->charsRead, buf, count)) != 0) 
	{
		// add to count, but don't advance writeHead
		dev->countBuffer[dev->writeHead] = count;
		goto out;
	}

	PDEBUG("WRITE circularBuffer[%d] contents: %s", dev->writeHead, dev->circularBuffer[dev->writeHead]);
	PDEBUG("WRITE buf contents: %s", buf);

	// update size
	dev->size += count + dev->charsRead;

	// add to count
	dev->countBuffer[dev->writeHead] = count + dev->charsRead;
	
	checkRealloc= strnchr(buf, count, '\n');
	PDEBUG("checkRealloc %p", checkRealloc);
	
	// newline found... write complete
	if(checkRealloc)
	{
		PDEBUG("write complete");
		dev->writeHead = (dev->writeHead + 1) % BUFFER_SIZE;
		dev->partial_flag = 0;
		dev->charsRead = 0;
	}
	// newline not found... partial write
	else
	{
		PDEBUG("partial write");
		dev->charsRead += count;
		dev->partial_flag = 1;
	}

	*f_pos += dev->charsRead;
	retval = count;

	// check if circular buffer full
	if(((dev->writeHead) % BUFFER_SIZE) == 0)
	{
		PDEBUG("circular buffer full");	
		dev->flag = 1;	
		PDEBUG("flag set");
	}

	dev->previous = dev->writeHead;

	for(i = 0; i < BUFFER_SIZE; i++)
	{
		PDEBUG("ALL circularBuffer[%d] contents: %s", i, dev->circularBuffer[i]);
	}

	PDEBUG("write stop");

out:
	PDEBUG("write: OUT");
	mutex_unlock(&lock);
	return retval;
}

loff_t aesd_llseek(struct file *filp, loff_t offset, int whence)
{
	PDEBUG("llseek start");

	struct aesd_dev *dev = filp->private_data;
	loff_t size = dev->size;
	PDEBUG("size = %ld", size);

	fixed_size_llseek(filp, offset, whence, size);

	PDEBUG("llseek stop");
	return size;
}

long aesd_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	PDEBUG("ioctl start");

	int i, count;
	struct aesd_dev *dev = filp->private_data;
	struct aesd_seekto seek;
	loff_t temp = 0;

	switch(cmd)
	{
		case AESDCHAR_IOCSEEKTO:
		{	
			PDEBUG("ioctl AESDCHAR_IOCSEEKTO case");
			if(copy_from_user(&seek, (const void __user *)arg, sizeof(seek)) != 0)
				return -EFAULT;

			// find f_pos from cmd
			for(i = 0; i < seek.write_cmd-1; i++)
			{
				count = dev->countBuffer[i];
				if(dev->countBuffer[i] == NULL)
				{
					PDEBUG("seeking past available data");
					return -EINVAL;
				}
				else
					temp += count;
			}

			// add offset
			temp += seek.write_cmd_offset;
		
			filp->f_pos = temp;
			PDEBUG("ioctl f_pos: %lld", filp->f_pos);
			break;
		}

		PDEBUG("ioctl f_pos: %lld", filp->f_pos);
	}

	PDEBUG("ioctl stop");
	return filp->f_pos;
}

struct file_operations aesd_fops = 
{
	.owner  	 = 	THIS_MODULE,
	.read   	 = 	aesd_read,
	.write  	 = 	aesd_write,
	.open    	 = 	aesd_open,
	.llseek	 	 = 	aesd_llseek,
//	.unlocked_ioctl	 =	aesd_ioctl,
	.release 	 =	aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
	int err, devno = MKDEV(aesd_major, aesd_minor);

	cdev_init(&dev->cdev, &aesd_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &aesd_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	if (err) {
		printk(KERN_ERR "Error %d adding aesd cdev", err);
	}
	return err;
}

int aesd_init_module(void)
{
	dev_t dev = 0;
	int result;
	result = alloc_chrdev_region(&dev, aesd_minor, 1, "aesdchar");
	aesd_major = MAJOR(dev);
	struct aesd_seekto seek;
	dev = MKDEV(aesd_major, aesd_minor);
	
	printk(KERN_INFO "INIT\n");
	
	if (result < 0) 
	{
		printk(KERN_WARNING "Can't get major %d\n", aesd_major);
		return result;
	}
	memset(&aesd_device, 0 ,sizeof(struct aesd_dev));

	aesd_device.writeHead = 0;
	aesd_device.readTail = 0;
	aesd_device.test = 0;
	aesd_device.flag = 0;
	aesd_device.count = 0;
	aesd_device.f_pos = 0;
	aesd_device.size = 0;
	aesd_device.readCount = 0;
	aesd_device.charsRead = 0;
	aesd_device.previous = 0;
	aesd_device.partial_flag = 0;
	seek.flag = 0;
	seek.write_cmd = 0;
	seek.write_cmd_offset = 0;

	result = aesd_setup_cdev(&aesd_device);

	if( result ) 
	{
		unregister_chrdev_region(dev, 1);
	}
	return result;

}

void aesd_cleanup_module(void)
{
	PDEBUG("cleanup module");
	dev_t devno = MKDEV(aesd_major, aesd_minor);
	int i;
	struct aesd_dev *dev;

	// get rid of char dev
	cdev_del(&aesd_device.cdev);
	
	if(dev->flag == 1)
	{
		// free buffer
		for(i = 0; i < BUFFER_SIZE; i++)
		{
			kfree(dev->circularBuffer[i]);
		}
	}
	else
	{
		for(i = 0; i < dev->writeHead; i++)
		{
			kfree(dev->circularBuffer[i]);
		}
	}
	unregister_chrdev_region(devno, 1);
}

module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
