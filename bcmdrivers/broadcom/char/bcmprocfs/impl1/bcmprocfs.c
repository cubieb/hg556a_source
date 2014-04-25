/*
 * <:copyright-broadcom
 *
 *  Copyright (c) 2002 Broadcom Corporation
 *   All Rights Reserved
 *   No portions of this material may be reproduced in any form without the
 *   written permission of:
 *   Broadcom Corporation
 *   16215 Alton Parkway
 *   Irvine, California 92619
 *   All information contained in this document is Broadcom Corporation
 *   company private, proprietary, and trade secret.
 *
 *  :>
 **/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/devfs_fs_kernel.h>	
#include "bcmprocfs.h"

#define VERSION "1.0"
#define BCMPROCFS_MAJOR 212

/* forward declarations for _fops */
static ssize_t bcm_read(struct file *file, char *buf, size_t count, loff_t *offset);
static ssize_t bcm_write(struct file *file, const char *buf, size_t count, loff_t *offset);
static int bcm_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static int bcm_open(struct inode *inode, struct file *file);
static int bcm_release(struct inode *inode, struct file *file);

#define STRINGLEN 32
#define MAXWAN 16

struct fb_data_t {
	char value0[STRINGLEN + 1];
	char value1[STRINGLEN + 1];
};


static struct proc_dir_entry *var_dir, *fyi_dir,*wan_dir,
	*ppp_dir[MAXWAN],
	*daemonstatus_file[MAXWAN],*wanipaddr_file[MAXWAN],*servicename_file[MAXWAN],*status_file[MAXWAN],*wanup_file[MAXWAN],*server_file[MAXWAN],*pid_file[MAXWAN],*subnetmask_file[MAXWAN],*sessinfo_file[MAXWAN],*sys_dir,*dns_file,*gateway_file;

 /*start of  修改sip domain 解析问题，sip domain解析需要按照接口来进行 by s53329  at  20080216*/
/*START ADD:Jaffen for pvc dns setting A36D03768*/
//#ifdef SINGAPORE_LOGIN
static struct proc_dir_entry *wandns_file[MAXWAN];
struct fb_data_t wandns_data[MAXWAN];
//#endif
/*END ADD:Jaffen for pvc dns setting A36D03768*/
 /*end  of  修改sip domain 解析问题，sip domain解析需要按照接口来进行 by s53329  at  20080216*/

struct fb_data_t daemonstatus_data[MAXWAN], wanipaddr_data[MAXWAN],servicename_data[MAXWAN];
struct fb_data_t status_data[MAXWAN],wanup_data[MAXWAN], server_data[MAXWAN], subnetmask_data[MAXWAN], sessinfo_data[MAXWAN], pid_data[MAXWAN];
struct fb_data_t dns_data,gateway_data;
static struct file_operations bcm_fops = {
	.owner		= THIS_MODULE,
	.read		= bcm_read,
	.write		= bcm_write,
	.ioctl		= bcm_ioctl,
	.open		= bcm_open,
	.release	= bcm_release
};

static int bcm_ioctl(struct inode *inode, struct file *file,
		       unsigned int cmd, unsigned long arg)
{
	/* ln -s src dst */
	struct symlink buffer;
	struct proc_dir_entry *link;

	switch (cmd) {
		case CREATE_SYMLINK:
		copy_from_user(&buffer,(void *)arg,sizeof(struct symlink));
		// printk("buffer.src=%s buffer.dst=%s, size=%d \n",buffer.src,buffer.dst, sizeof(struct symlink));
		
		/* create symlink */
			
		link = proc_symlink(buffer.dst,wan_dir,buffer.src);
			if (link == NULL) {
			   printk("bcm_ioctl: proc_symlink failed \n");
			   return -ENOMEM;
			}
			link->owner = THIS_MODULE;
			
			break;
		case CREATE_FILE:
			//copy_from_user(buffer,(char *)&arg,16);
			break;
		case RENAME_TELNETD:
			strncpy(current->comm,"telnetd",8);
			break;
		case RENAME_HTTPD:
			strncpy(current->comm,"httpd",6);
			break;
		case RENAME_SSHD:
			strncpy(current->comm,"sshd",5);
			break;
		case RENAME_SNMP:
			strncpy(current->comm,"snmpd",6);
			break;
		case RENAME_TR69C:
			strncpy(current->comm,"tr69c",6);
			break;
		/* start of maintain Auto Upgrade by zhangliang 60003055 2006年5月22日 */
		case RENAME_AUGD:
			strncpy(current->comm,"augd",5);
			break;
		/* end of maintain Auto Upgrade by zhangliang 60003055 2006年5月22日 */
        case RENAME_TMPFILE:
            strncpy(current->comm,"tmpfile",8);
			break;
        case RENAME_MSGPROC:
            strncpy(current->comm,"msgproc",8);
            break;
        case RENAME_ALIVE:
            strncpy(current->comm,"alive",6);
            break;
        case  RENAME_DNSRELAY:
	    strncpy(current->comm,"dnspr",6);
            break;
        case  RENAME_BFTPD:
            strncpy(current->comm,"bftpd",6);
            break;
        /*add by z67625 增加防火墙日志写flash进程重命名 start*/
        case RENAME_FWLOGRECORD:   
            strncpy(current->comm, "fwlog", 6);
            break;
        /*add by z67625 增加防火墙日志写flash进程重命名 end*/
	 case RENAME_SYSLOGSAVED:
             strncpy(current->comm, "hwlog", 6);
            break;
     case RENAME_TR64:
        strncpy(current->comm, "tr064", 6);
        break;
     default: 
        {
            printk("ioctl: no such command\n");
            return -ENOTTY;
        }
	}

	return 0;
}

static ssize_t bcm_read(struct file *file, char *buf, size_t count,
			  loff_t *offset)
{
    unsigned short minor;

    /* Select which minor device */
    minor = MINOR(file->f_dentry->d_inode->i_rdev);
	return 0;
}
static ssize_t bcm_write(struct file *file, const char *buf, 
				size_t count, loff_t *offset)
{
    unsigned short minor;

    /* Select which minor device */
    minor = MINOR(file->f_dentry->d_inode->i_rdev);
	return 0;
}

static int bcm_open(struct inode *inode, struct file *file)
{
	
	if (file->f_mode & FMODE_READ) {
		//printk("opened for reading\n");
	} else if (file->f_mode & FMODE_WRITE) {
		//printk("opened for writing \n");
	}

	//printk("major: %d minor: %d\n", MAJOR(inode->i_rdev), MINOR(inode->i_rdev));

	return 0;
}

static int bcm_release(struct inode *inode, struct file *file)
{
	//printk("bcm_release\n");
	return 0;
}




static int proc_read_dns_string(char *page, char **start,
			    off_t off, int count, 
			    int *eof, void *data)
{
	int len;
	struct fb_data_t *fb_data = (struct fb_data_t *)data;
	
	/*
	len = sprintf(page, "%s\n%s\n", 
		      fb_data->value0,fb_data->value1);
		      */
	len = sprintf(page, "%s%s", 
		      fb_data->value0,fb_data->value1);

	return len;
}

static int proc_write_dns_string(struct file *file,
			     const char *buffer,
			     unsigned long count, 
			     void *data)
{
	int len;
	struct fb_data_t *fb_data = (struct fb_data_t *)data;


	if(count > STRINGLEN)
		len = STRINGLEN;
	else
		len = count;
	/*
	printk("fb_data->value0[0]=0x%x \n",fb_data->value0[0]);
	printk("buffer[0]=0x%x \n",buffer[0]);
	*/
	if ( buffer[0] == 0xa ) { /* clean the entries */
		fb_data->value0[0]='\0';
		fb_data->value1[0]='\0';
		return len;
	}
	if (fb_data->value0[0] == '\0') {
	  if(copy_from_user(fb_data->value0, buffer, len)) {
		return -EFAULT;
	  }

	  fb_data->value0[len] = '\0';
	}
	else {
	 if(copy_from_user(fb_data->value1, buffer, len)) {
		return -EFAULT;
	 }

	 fb_data->value1[len] = '\0';
	}

	return len;
}
static int proc_read_string(char *page, char **start,
			    off_t off, int count, 
			    int *eof, void *data)
{
	int len;
	struct fb_data_t *fb_data = (struct fb_data_t *)data;
	
	len = sprintf(page, "%s\n", 
		      fb_data->value0);

	return len;
}


static int proc_write_string(struct file *file,
			     const char *buffer,
			     unsigned long count, 
			     void *data)
{
	int len;
	struct fb_data_t *fb_data = (struct fb_data_t *)data;

	if(count > STRINGLEN)
		len = STRINGLEN;
	else
		len = count;

	if(copy_from_user(fb_data->value0, buffer, len)) {
		return -EFAULT;
	}

	fb_data->value0[len] = '\0';

	return len;
}


static int init_procfs(void)
{
	int i = 0;
	int rv = 0;
	char path[64]="";

	/* create directory */
	var_dir = proc_mkdir("var", NULL);
	if(var_dir == NULL) {
		rv = -ENOMEM;
		goto out;
	}
	
	var_dir->owner = THIS_MODULE;


	/* create fyi directory */
	fyi_dir = proc_mkdir("var/fyi", NULL);
	if(fyi_dir == NULL) {
		rv = -ENOMEM;
		goto out;
	}
	
	fyi_dir->owner = THIS_MODULE;
	
	sys_dir = proc_mkdir("var/fyi/sys", NULL);
	if(sys_dir == NULL) {
		rv = -ENOMEM;
		goto out;
	}
	
	sys_dir->owner = THIS_MODULE;

	dns_file = create_proc_entry("dns", 0644, sys_dir);
	if(dns_file == NULL) {
		rv = -ENOMEM;
		goto out;
	}

	//strcpy(dns_data.value0, "nameserver 10.1.2.3");
	//strcpy(dns_data.value1, "nameserver 10.1.2.4");
	dns_file->data = &dns_data;
	dns_file->read_proc = proc_read_dns_string;
	dns_file->write_proc = proc_write_dns_string;
	dns_file->owner = THIS_MODULE;

	gateway_file = create_proc_entry("gateway", 0644, sys_dir);
	if(gateway_file == NULL) {
		rv = -ENOMEM;
		goto out;
	}

	strcpy(gateway_data.value0, "10.1.2.3");
	gateway_file->data = &gateway_data;
	gateway_file->read_proc = proc_read_string;
	gateway_file->write_proc = proc_write_string;
	gateway_file->owner = THIS_MODULE;

	/* create fyi directory */
	wan_dir = proc_mkdir("var/fyi/wan", NULL);
	if(wan_dir == NULL) {
		rv = -ENOMEM;
		goto out;
	}
	
	wan_dir->owner = THIS_MODULE;

	/* create ppp directory */
	for (i=0; i < MAXWAN; i++) {
	    sprintf(path, "var/fyi/wan/.ppp%d", i);
	    ppp_dir[i] = proc_mkdir(path, NULL);
	    if(ppp_dir[i] == NULL) {
		rv = -ENOMEM;
		goto out;
	    }
	    ppp_dir[i]->owner = THIS_MODULE;
	}

#if 0
	/* create symlink */
	symlink = proc_symlink("ppp77", wan_dir, 
			       "ppp7");
	if(symlink == NULL) {
		rv = -ENOMEM;
		goto out;
	}
	symlink->owner = THIS_MODULE;
#endif

	for (i=0; i < MAXWAN; i++) {

	daemonstatus_file[i] = create_proc_entry("daemonstatus", 0644, ppp_dir[i]);
	strcpy(daemonstatus_data[i].value0, "daemonstatus");
	daemonstatus_file[i]->data = &daemonstatus_data[i];
	daemonstatus_file[i]->read_proc = proc_read_string;
	daemonstatus_file[i]->write_proc = proc_write_string;
	daemonstatus_file[i]->owner = THIS_MODULE;

	wanipaddr_file[i] = create_proc_entry("ipaddress", 0644, ppp_dir[i]);
	strcpy(wanipaddr_data[i].value0, "wanIpaddress");
	wanipaddr_file[i]->data = &wanipaddr_data[i];
	wanipaddr_file[i]->read_proc = proc_read_string;
	wanipaddr_file[i]->write_proc = proc_write_string;
	wanipaddr_file[i]->owner = THIS_MODULE;

	servicename_file[i] = create_proc_entry("servicename", 0644, ppp_dir[i]);
	strcpy(servicename_data[i].value0, "servicename");
	servicename_file[i]->data = &servicename_data[i];
	servicename_file[i]->read_proc = proc_read_string;
	servicename_file[i]->write_proc = proc_write_string;
	servicename_file[i]->owner = THIS_MODULE;

	status_file[i] = create_proc_entry("status", 0644, ppp_dir[i]);
	strcpy(status_data[i].value0, "status");
	status_file[i]->data = &status_data[i];
	status_file[i]->read_proc = proc_read_string;
	status_file[i]->write_proc = proc_write_string;
	status_file[i]->owner = THIS_MODULE;

	wanup_file[i] = create_proc_entry("wanup", 0644, ppp_dir[i]);
	strcpy(wanup_data[i].value0, "wanup");
	wanup_file[i]->data = &wanup_data[i];
	wanup_file[i]->read_proc = proc_read_string;
	wanup_file[i]->write_proc = proc_write_string;
	wanup_file[i]->owner = THIS_MODULE;
/*
	server_file[i] = create_proc_entry("server", 0644, ppp_dir[i]);
	strcpy(server_data[i].value0, "server");
	server_file[i]->data = &server_data[i];
	server_file[i]->read_proc = proc_read_string;
	server_file[i]->write_proc = proc_write_string;
	server_file[i]->owner = THIS_MODULE;
*/
	subnetmask_file[i] = create_proc_entry("subnetmask", 0644, ppp_dir[i]);
	strcpy(subnetmask_data[i].value0, "subnetmask");
	subnetmask_file[i]->data = &subnetmask_data[i];
	subnetmask_file[i]->read_proc = proc_read_string;
	subnetmask_file[i]->write_proc = proc_write_string;
	subnetmask_file[i]->owner = THIS_MODULE;

	sessinfo_file[i] = create_proc_entry("sessinfo", 0644, ppp_dir[i]);
	strcpy(sessinfo_data[i].value0, "");
	sessinfo_file[i]->data = &sessinfo_data[i];
	sessinfo_file[i]->read_proc = proc_read_string;
	sessinfo_file[i]->write_proc = proc_write_string;
	sessinfo_file[i]->owner = THIS_MODULE;

	pid_file[i] = create_proc_entry("pid", 0644, ppp_dir[i]);
	strcpy(pid_data[i].value0, "");
	pid_file[i]->data = &pid_data[i];
	pid_file[i]->read_proc = proc_read_string;
	pid_file[i]->write_proc = proc_write_string;
	pid_file[i]->owner = THIS_MODULE;
 /*start of  修改sip domain 解析问题，sip domain解析需要按照接口来进行 by s53329  at  20080216*/
    /*START ADD:Jaffen for pvc dns setting A36D03768*/
//#ifdef SINGAPORE_LOGIN
	wandns_file[i] = create_proc_entry("dns", 0644, ppp_dir[i]);
	strcpy(wandns_data[i].value0, "");
    strcpy(wandns_data[i].value1, "");
	wandns_file[i]->data = &wandns_data[i];
	wandns_file[i]->read_proc = proc_read_dns_string;
	wandns_file[i]->write_proc = proc_write_dns_string;
	wandns_file[i]->owner = THIS_MODULE;
//#endif
    /*END ADD:Jaffen for pvc dns setting A36D03768*/   
 /*end  of  修改sip domain 解析问题，sip domain解析需要按照接口来进行 by s53329  at  20080216*/
	}


	//------------------------------------------------------------

	/* everything OK */
	printk(KERN_INFO "Broadcom BCMPROCFS %s%s initialized\n",
	       "v", VERSION);
	return 0;

out:
	return rv;
}


static void  cleanup_procfs(void)
{
	int i=0;
	char path[64]="";

	remove_proc_entry("var/fyi/sys", NULL);
	for (i=0; i < MAXWAN; i++) {
	    sprintf(path, "var/fyi/wan/.ppp%d", i);
	    remove_proc_entry(path, NULL);
	}
	remove_proc_entry("var/fyi/wan", NULL);
	remove_proc_entry("var/fyi", NULL);
	remove_proc_entry("var", NULL);

	printk(KERN_INFO "%s%s removed\n",
	       "v", VERSION);
}

static int __init bcmprocfs_init(void)
{
	int res;
	
	/* register device with kernel */
	res = register_chrdev(BCMPROCFS_MAJOR, "bcm", &bcm_fops);
	if (res) {
		printk("bcmprocfs_init: can't register device with kernel\n");
		return res;
	}
	init_procfs();
	return 0;
}

static void __exit bcmprocfs_cleanup(void)
{
        unregister_chrdev(BCMPROCFS_MAJOR, "bcm");
	cleanup_procfs();
}

module_init(bcmprocfs_init);
module_exit(bcmprocfs_cleanup);
MODULE_LICENSE("Proprietary");
MODULE_VERSION(VERSION);
