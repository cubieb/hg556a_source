/*
  USB Driver for GSM modems

  Copyright (C) 2005  Matthias Urlichs <smurf@smurf.noris.de>

  This driver is free software; you can redistribute it and/or modify
  it under the terms of Version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  Portions copied from the Keyspan driver by Hugh Blemings <hugh@blemings.org>

  History: see the git log.

  Work sponsored by: Sigos GmbH, Germany <info@sigos.de>

  This driver exists because the "normal" serial driver doesn't work too well
  with GSM modems. Issues:
  - data loss -- one single Receive URB is not nearly enough
  - nonstandard flow (Option devices) control
  - controlling the baud rate doesn't make sense

  This driver is named "option" because the most common device it's
  used for is a PC-Card (with an internal OHCI-USB interface, behind
  which the GSM interface sits), made by Option Inc.

  Some of the "one port" devices actually exhibit multiple USB instances
  on the USB bus. This is not a bug, these ports are used for different
  device features.
*/

#define DRIVER_VERSION "v0.7.1"
#define DRIVER_AUTHOR "Matthias Urlichs <smurf@smurf.noris.de>"
#define DRIVER_DESC "USB Driver for GSM modems"

#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/usb.h>
#include "usb-serial.h"
#include "board.h"

/* Function prototypes */
static int  option_open(struct usb_serial_port *port, struct file *filp);
static void option_close(struct usb_serial_port *port, struct file *filp);
static int  option_startup(struct usb_serial *serial);
static void option_shutdown(struct usb_serial *serial);
static void option_rx_throttle(struct usb_serial_port *port);
static void option_rx_unthrottle(struct usb_serial_port *port);
static int  option_write_room(struct usb_serial_port *port);

static void option_instat_callback(struct urb *urb, struct pt_regs *regs);

static int option_write(struct usb_serial_port *port, int from_user,
			const unsigned char *buf, int count);

static int  option_chars_in_buffer(struct usb_serial_port *port);
static int  option_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg);
static void option_set_termios(struct usb_serial_port *port,
				struct termios *old);
static void option_break_ctl(struct usb_serial_port *port, int break_state);
static int  option_tiocmget(struct usb_serial_port *port, struct file *file);
static int  option_tiocmset(struct usb_serial_port *port, struct file *file,
				unsigned int set, unsigned int clear);
static int  option_send_setup(struct usb_serial_port *port);

static int usb_option_clear_halt(struct usb_device *dev, int pipe);

/* Vendor and product IDs */
#define OPTION_VENDOR_ID                0x0AF0
#define HUAWEI_VENDOR_ID                0x12D1
#define AUDIOVOX_VENDOR_ID              0x0F3D
#define NOVATELWIRELESS_VENDOR_ID       0x1410
#define ANYDATA_VENDOR_ID               0x16d5

#define OPTION_PRODUCT_OLD              0x5000
#define OPTION_PRODUCT_FUSION           0x6000
#define OPTION_PRODUCT_FUSION2          0x6300
#define OPTION_PRODUCT_COBRA            0x6500
#define OPTION_PRODUCT_COBRA2           0x6600
#define OPTION_PRODUCT_GTMAX36          0x6701
#define HUAWEI_PRODUCT_E600             0x1001
#define HUAWEI_PRODUCT_E220             0x1003
#define HUAWEI_PRODUCT_K3715            0x1004
#define AUDIOVOX_PRODUCT_AIRCARD        0x0112
#define NOVATELWIRELESS_PRODUCT_U740    0x1400
#define ANYDATA_PRODUCT_ID              0x6501

/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add begin:*/
extern int g_nHspaTrafficMode ;
/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add end.*/
extern int g_iDataCardIn;

static struct usb_device_id option_ids[] = {
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_OLD) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_FUSION) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_FUSION2) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_COBRA) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_COBRA2) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_GTMAX36) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E600) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E220) },
    { USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_K3715) },
	{ USB_DEVICE(AUDIOVOX_VENDOR_ID, AUDIOVOX_PRODUCT_AIRCARD) },
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID,NOVATELWIRELESS_PRODUCT_U740) },
	{ USB_DEVICE(ANYDATA_VENDOR_ID, ANYDATA_PRODUCT_ID) },
	{ } /* Terminating entry */
};

static struct usb_device_id option_ids1[] = {
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_OLD) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_FUSION) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_FUSION2) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_COBRA) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_COBRA2) },
	{ USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_GTMAX36) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E600) },
	{ USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_E220) },
    { USB_DEVICE(HUAWEI_VENDOR_ID, HUAWEI_PRODUCT_K3715) },
	{ USB_DEVICE(AUDIOVOX_VENDOR_ID, AUDIOVOX_PRODUCT_AIRCARD) },
	{ USB_DEVICE(NOVATELWIRELESS_VENDOR_ID,NOVATELWIRELESS_PRODUCT_U740) },
	{ USB_DEVICE(ANYDATA_VENDOR_ID, ANYDATA_PRODUCT_ID) },
	{ } /* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, option_ids);

static struct usb_driver option_driver = {
	.name       = "option",
	.probe      = usb_serial_probe,
	.disconnect = usb_serial_disconnect,
	.id_table   = option_ids,
//	.no_dynamic_id = 	1,
};

/* The card has three separate interfaces, which the serial driver
 * recognizes separately, thus num_port=1.
 */

static struct usb_serial_device_type option_1port_device = {
	.owner             = THIS_MODULE,
	.name              = "option1",
	.id_table          = option_ids1,
	.num_interrupt_in  = NUM_DONT_CARE,
	.num_bulk_in       = NUM_DONT_CARE,
	.num_bulk_out      = NUM_DONT_CARE,
	.num_ports         = 1,
	.open              = option_open,
	.close             = option_close,
	.write             = option_write,
	.write_room        = option_write_room,
	.chars_in_buffer   = option_chars_in_buffer,
	.throttle          = option_rx_throttle,
	.unthrottle        = option_rx_unthrottle,
	.ioctl             = option_ioctl,
	.set_termios       = option_set_termios,
	.break_ctl         = option_break_ctl,
	.tiocmget          = option_tiocmget,
	.tiocmset          = option_tiocmset,
	.attach            = option_startup,
	.shutdown          = option_shutdown,
	.read_int_callback = option_instat_callback,
};

#ifdef CONFIG_USB_DEBUG
static int debug;
#else
#define debug 0
#endif

/* per port private data */

#define N_IN_URB 4
#define N_OUT_URB 2

/* START:K3772 download speed lower than PC,by ZZQ00195436 at 2011.12.3 */
/* BEGIN: Added by aizhanlong 00169427, 2011/10/29   PN:HSPA upload speed lower than PC*/
//#define IN_BUFLEN 3840
#define IN_BUFLEN 8192
/* END:   Added by aizhanlong 00169427, 2011/10/29 */
//#define OUT_BUFLEN 1024
#define OUT_BUFLEN 2048
/* END:K3772 download speed lower than PC,by ZZQ00195436 at 2011.12.3 */

struct option_port_private {
	/* Input endpoints and buffer for this port */
	struct urb *in_urbs[N_IN_URB];
	char in_buffer[N_IN_URB][IN_BUFLEN];
	/* Output endpoints and buffer for this port */
	struct urb *out_urbs[N_OUT_URB];
	char out_buffer[N_OUT_URB][OUT_BUFLEN];

	/* Settings for the port */
	int rts_state;	/* Handshaking pins (outputs) */
	int dtr_state;
	int cts_state;	/* Handshaking pins (inputs) */
	int dsr_state;
	int dcd_state;
	int ri_state;

	unsigned long tx_start_time[N_OUT_URB];
};

extern int iUsbTtyState;
extern int iUsbModemOpened;
extern char ttyUSB_pid[];
extern struct proc_dir_entry proc_root;



#ifdef LY_OP_DEBUG

typedef struct option_statistic
{
    unsigned long receive[3];
    unsigned long send[3];
    unsigned long err[3];
}OP_STAT, *POP_STAT;

static OP_STAT g_stOpstat;

int option_read_statistic(char *buf, char **start, off_t offset,
                   int count, int *eof, void *data)
{
	int len = 0;
	len += sprintf(buf, "\treceive\t\tsend\t\terror\t\r\nMODEM\t%d\t\t%d\t\t%d\r\nDIAG\t%d\t\t%d\t\t%d\r\nPCUI\t%d\t\t%d\t\t%d\r\n", 
		g_stOpstat.receive[0], g_stOpstat.send[0], g_stOpstat.err[0],
		g_stOpstat.receive[1], g_stOpstat.send[1], g_stOpstat.err[1],
		g_stOpstat.receive[2], g_stOpstat.send[2], g_stOpstat.err[2]);

	*eof = 1;
	return len;
}

#endif

/* when some one writes to proc file this function will be invoked */
static int option_write_procmem (struct file *file,
                             const char *buffer,
                             unsigned long count,
                             void *data)
{
   strcpy(ttyUSB_pid, buffer);
   return count;
}


int option_read_procmem(char *buf, char **start, off_t offset,
                   int count, int *eof, void *data)
{
	int len = 0;
	len += sprintf(buf, "Usb-Serial Port Num:%d\r\nOpen:%d\r\nop:%d\r\n", 
		iUsbTtyState, iUsbModemOpened, g_iDataCardIn);
       // len += sprintf(buf, "ttyUSB status: %d\r\n", iUsbTtyState);

	*eof = 1;
	return len;
}

static void option_create_proc(void)
{

    struct proc_dir_entry* proc_pid = NULL;
#if 0    
	create_proc_read_entry("UsbModem", 0 /* default mode */,
			NULL /* parent dir */, option_read_procmem,
			NULL /* client data */);
#endif
    memset(ttyUSB_pid, 0, 64);

    proc_pid = create_proc_entry("UsbModem", S_IFREG | S_IRUSR , &proc_root ); 
    proc_pid->write_proc = option_write_procmem;
    proc_pid->read_proc = option_read_procmem;

    /*增加USB收发包统计*/
    #ifdef LY_OP_DEBUG
    create_proc_read_entry("usb-stat", 0 /* default mode */,
			NULL /* parent dir */, option_read_statistic,
			NULL /* client data */);
   #endif
}

static void option_remove_proc(void)
{
	/* no problem if it was not registered */
	remove_proc_entry("UsbModem", NULL /* parent dir */);
}

/* Functions used by new usb-serial code. */
static int __init option_init(void)
{
	int retval;
	retval = usb_serial_register(&option_1port_device);
	if (retval)
		goto failed_1port_device_register;
	retval = usb_register(&option_driver);
	if (retval)
		goto failed_driver_register;

    	option_create_proc();

	info(DRIVER_DESC ": " DRIVER_VERSION);

	return 0;

failed_driver_register:
	usb_serial_deregister (&option_1port_device);
failed_1port_device_register:
	return retval;
}

static void __exit option_exit(void)
{
    option_remove_proc();
	usb_deregister (&option_driver);
	usb_serial_deregister (&option_1port_device);
}

module_init(option_init);
module_exit(option_exit);

static void option_rx_throttle(struct usb_serial_port *port)
{
	dbg("%s", __FUNCTION__);
}

static void option_rx_unthrottle(struct usb_serial_port *port)
{
	dbg("%s", __FUNCTION__);
}

static void option_break_ctl(struct usb_serial_port *port, int break_state)
{
	/* Unfortunately, I don't know how to send a break */
	dbg("%s", __FUNCTION__);
}

static void option_set_termios(struct usb_serial_port *port,
			struct termios *old_termios)
{
	dbg("%s", __FUNCTION__);

	option_send_setup(port);
}

static int option_tiocmget(struct usb_serial_port *port, struct file *file)
{
	unsigned int value;
	struct option_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	value = ((portdata->rts_state) ? TIOCM_RTS : 0) |
		((portdata->dtr_state) ? TIOCM_DTR : 0) |
		((portdata->cts_state) ? TIOCM_CTS : 0) |
		((portdata->dsr_state) ? TIOCM_DSR : 0) |
		((portdata->dcd_state) ? TIOCM_CAR : 0) |
		((portdata->ri_state) ? TIOCM_RNG : 0);

	return value;
}

static int option_tiocmset(struct usb_serial_port *port, struct file *file,
			unsigned int set, unsigned int clear)
{
	struct option_port_private *portdata;

	portdata = usb_get_serial_port_data(port);

	if (set & TIOCM_RTS)
		portdata->rts_state = 1;
	if (set & TIOCM_DTR)
		portdata->dtr_state = 1;

	if (clear & TIOCM_RTS)
		portdata->rts_state = 0;
	if (clear & TIOCM_DTR)
		portdata->dtr_state = 0;
	return option_send_setup(port);
}

static int option_ioctl(struct usb_serial_port *port, struct file *file,
			unsigned int cmd, unsigned long arg)
{
	return -ENOIOCTLCMD;
}

/*
 * decide if this payload contain GSM Modem instruction(AT+xxx).
 * <2009.1.5 tanyin>
 */
static int option_at_instruction(const unsigned char *buf)
{
    int result = 0;

    if ((buf[0] == 'A') &&
        (buf[1] == 'T')) {

        result = 1;
    }
    else if ((buf[0] == 'a') &&
        (buf[1] == 't')) {

        result = 1;
    }
    else if ((buf[0] == '\r') &&
        (buf[1] == '\n')) {
        result = 1;
    }

    return result;
}

/* Write */
static int option_write(struct usb_serial_port *port, int from_user,
			const unsigned char *buf, int count)
{
	struct option_port_private *portdata;
	int i;
	int left, todo;
	struct urb *this_urb = NULL; /* spurious */
	int err;
    int ret;
    int normal_data = 0;
    
	portdata = usb_get_serial_port_data(port);

	dbg("%s: write (%d chars)", __FUNCTION__, count);

	i = 0;
	left = count;
	for (i=0; left > 0 && i < N_OUT_URB; i++) {
		todo = left;
		if (todo > OUT_BUFLEN)
			todo = OUT_BUFLEN;

		this_urb = portdata->out_urbs[i];
/* BEGIN: Added by aizhanlong 00169427, 2011/10/29   PN:HSPA upload speed lower than PC*/
		/*BB5D00665	chenkeyun   20081215	add  begin*/
		/*BB5D02965	l00101002   20091026	add  begin*/
		this_urb->transfer_flags = this_urb->transfer_flags &(~URB_ZERO_PACKET) ;
		/*BB5D02965	l00101002   20091026	add  end*/
		/*BB5D00665	chenkeyun   20081215	add  end*/
		if (this_urb->status == -EINPROGRESS) {
/*BB5D00777  chenkeyun	20081225   mod	begin*/
#if 0
			if (time_before(jiffies,
					portdata->tx_start_time[i] + 10 * HZ))
				continue;
			usb_unlink_urb(this_urb);
			continue;
#else
            continue;
#endif
/*BB5D00777  chenkeyun	20081225   mod	end*/
		}
		if (this_urb->status != 0)
			dbg("usb_write %p failed (err=%d)",
				this_urb, this_urb->status);

		dbg("%s: endpoint %d buf %d", __FUNCTION__,
			usb_pipeendpoint(this_urb->pipe), i);

		/* send the data */
/*BB5D00665	chenkeyun   20081215	add  begin*/
/*根据USB1.1协议，在一次传输过程中，最后的一次写的字节数为64的整数倍数，usb认为还有数据需要写，则等待写入，因此拆分数据来解决这个问题。*/
		/*BB5D02965	l00101002   20091026	add  begin*/
	  if( (left <= OUT_BUFLEN) && (!(todo%64)) && ( todo>0))
	  {
        this_urb->transfer_flags = this_urb->transfer_flags | URB_ZERO_PACKET ;
	  }
		/*BB5D02965	l00101002   20091026	add  end*/
/*BB5D00665	chenkeyun   20081215	add  end*/
/* END:   Added by aizhanlong 00169427, 2011/10/29 */
		memcpy (this_urb->transfer_buffer, buf, todo);
		this_urb->transfer_buffer_length = todo;
		this_urb->status = 0;
		this_urb->transfer_flags = URB_ASYNC_UNLINK | URB_NO_SETUP_DMA_MAP;

		this_urb->dev = port->serial->dev;
		err = usb_submit_urb(this_urb, GFP_ATOMIC);
		if (err) {

            /*
			dbg("usb_submit_urb %p (write bulk) failed "
				"(%d, has %d)", this_urb,
				err, this_urb->status);
				*/
            switch (err)
    		{
                /*ep stall*/
                case -EPIPE:
                    ret = usb_option_clear_halt(this_urb->dev, this_urb->pipe);
	                    err = usb_submit_urb(this_urb, GFP_ATOMIC);
                    printk("usb_clear_halt ret %d line %d\n", ret, __LINE__);
                    break;
                default:
#ifdef LY_OP_DEBUG
                    g_stOpstat.err[port->number] +=  todo;
#endif

                    break;
    		}
/* BEGIN: Added by aizhanlong 00169427, 2011/10/29   PN:HSPA upload speed lower than PC*/            
			continue;
/* END:   Added by aizhanlong 00169427, 2011/10/29 */
		}

#ifdef LY_OP_DEBUG
        g_stOpstat.send[port->number] += todo;
#endif

		portdata->tx_start_time[i] = jiffies;
		buf += todo;
		left -= todo;
	}

	count -= left;
	
	/*l65130 2008-09-20 start*/
/* open it if you want blink led only when there is traffic. <tanyin 2009.5.20> */
#if 0
    /* flicker led only when it gets normal data. <2009.1.5 tanyin> */
	if (port->number != 2 && count && normal_data)
	{
        /* Change Request.
         * HG556a OR 1.8(older)
         * 2G - flicker 2HZ + pause 1s + flicker 2HZ
         * 3G - flicker 4HZ + pause 1s + flicker 4HZ
         * HG556a OR 1.9
         * 2G - flicker 1HZ
         * 3G - flicker 2HZ
         * <2009.1.4 tanyin>
         */
		/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add begin:*/
		if ( MODE_2G == g_nHspaTrafficMode)
		{
		    //kerSysLedCtrl(kLedHspa, kLedStateSlowFlickerOnce);
		    kerSysLedCtrl(kLedHspa, kLedStateVerySlowBlinkOnce);
		}
		else if ( MODE_3G == g_nHspaTrafficMode )
		{
		    //kerSysLedCtrl(kLedHspa, kLedStateFastFlickerOnce);
		    kerSysLedCtrl(kLedHspa, kLedStateSlowBlinkOnce);
		}
		/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add end.*/
	}
#endif
	/*l65130 2008-09-20 end*/

	return count;
}

static void option_indat_callback(struct urb *urb)
{
    int i;
	int err;
	int endpoint;
	struct usb_serial_port *port;
	struct tty_struct *tty;
	unsigned char *data = urb->transfer_buffer;

    struct usb_serial *serial = NULL;
    int normal_data = 0;

	dbg("%s: %p", __FUNCTION__, urb);

	endpoint = usb_pipeendpoint(urb->pipe);
	port = (struct usb_serial_port *) urb->context;

    if (!port || port->serial->dev->state == USB_STATE_NOTATTACHED)
		return 0;

    serial = port->serial;

    //urb->dev = port->serial->dev;

	if (urb->status) 
    {
        ;//if (urb->status == -EPIPE)
        //    usb_option_clear_halt(urb->dev, urb->pipe);
        
        #ifdef LY_OP_DEBUG
	 	if (urb->actual_length)
        		g_stOpstat.err[port->number] += urb->actual_length;
        #endif

	} else{

        
    
		tty = port->tty;
		if (urb->actual_length) {

#ifdef LY_OP_DEBUG
        	g_stOpstat.receive[port->number] += urb->actual_length;
#endif
            /* if this is normal data. <2009.1.5 tanyin> */
            normal_data = !option_at_instruction(data);

            for (i = 0; i < urb->actual_length; i++)
            {
                if (tty->flip.count >= TTY_FLIPBUF_SIZE)
                {
                    tty_flip_buffer_push(tty);
                }
                tty_insert_flip_char(tty, data[i], 0);
            }

            tty_flip_buffer_push(tty);

			
		/*l65130 2008-09-20 start*/
#if 0
        /* flicker led only when it gets normal data. <2009.1.5 tanyin> */
		if (port->number != 2 && normal_data)
		{
                /* Change Request.
                 * HG556a OR 1.8(older)
                 * 2G - flicker 2HZ + pause 1s + flicker 2HZ
                 * 3G - flicker 4HZ + pause 1s + flicker 4HZ
                 * HG556a OR 1.9
                 * 2G - flicker 1HZ
                 * 3G - flicker 2HZ
                 * <2009.1.4 tanyin>
                 */

	            /* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add begin:*/
	            if ( MODE_2G == g_nHspaTrafficMode )
	            {
	                //kerSysLedCtrl(kLedHspa, kLedStateSlowFlickerOnce);
	                kerSysLedCtrl(kLedHspa, kLedStateVerySlowBlinkOnce);
	            }
	            else if (MODE_3G == g_nHspaTrafficMode )
	            {
	                //kerSysLedCtrl(kLedHspa, kLedStateFastFlickerOnce);
	                kerSysLedCtrl(kLedHspa, kLedStateSlowBlinkOnce);
	            }
	            /* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add end.*/
		}

#endif
		/*l65130 2008-09-20 end*/

		} else {
			dbg("%s: empty read urb received", __FUNCTION__);
		}

#if 0
        /* Continue trying to always read  */
	    usb_fill_bulk_urb (urb, serial->dev,
			   usb_rcvbulkpipe (serial->dev,
				   	    port->bulk_in_endpointAddress),
			   urb->transfer_buffer,
			   urb->transfer_buffer_length,
			   ((serial->type->read_bulk_callback) ? 
			     serial->type->read_bulk_callback : 
			     usb_serial_generic_read_bulk_callback), port);
#endif

		
	}

    /* Resubmit urb so we continue receiving */
	if (port->open_count && urb->status != -ESHUTDOWN) 
    {
	    err = usb_submit_urb(urb, GFP_ATOMIC);
	}
	return;
}

static void option_outdat_callback(struct urb *urb)
{
	struct usb_serial_port *port;

	dbg("%s", __FUNCTION__);

	port = (struct usb_serial_port *) urb->context;
/* BEGIN: Added by aizhanlong 00169427, 2011/10/29   PN:HSPA upload speed lower than PC*/
    if (!port || port->serial->dev->state == USB_STATE_NOTATTACHED)
		return 0;
 /* END:   Added by aizhanlong 00169427, 2011/10/29 */   
	usb_serial_port_softint(port);
}

static void option_instat_callback(struct urb *urb, struct pt_regs *regs)
{
	int err;
	struct usb_serial_port *port = (struct usb_serial_port *) urb->context;
	struct option_port_private *portdata = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;

    if (!port || port->serial->dev->state == USB_STATE_NOTATTACHED)
		return 0;

	dbg("%s: urb %p port %p has data %p", __FUNCTION__,urb,port,portdata);

	if (urb->status == 0) {
		struct usb_ctrlrequest *req_pkt =
				(struct usb_ctrlrequest *)urb->transfer_buffer;

		if (!req_pkt) {
			dbg("%s: NULL req_pkt\n", __FUNCTION__);
			return;
		}
		if ((req_pkt->bRequestType == 0xA1) &&
				(req_pkt->bRequest == 0x20)) {
			int old_dcd_state;
			unsigned char signals = *((unsigned char *)
					urb->transfer_buffer +
					sizeof(struct usb_ctrlrequest));

			dbg("%s: signal x%x", __FUNCTION__, signals);

			old_dcd_state = portdata->dcd_state;
			portdata->cts_state = 1;
			portdata->dcd_state = ((signals & 0x01) ? 1 : 0);
			portdata->dsr_state = ((signals & 0x02) ? 1 : 0);
			portdata->ri_state = ((signals & 0x08) ? 1 : 0);

			if (port->tty && !C_CLOCAL(port->tty) &&
					old_dcd_state && !portdata->dcd_state)
				tty_hangup(port->tty);
		} else {
			dbg("%s: type %x req %x", __FUNCTION__,
				req_pkt->bRequestType,req_pkt->bRequest);
		}
	} 
    else
	{
        if (urb->status == -EPIPE)
        {
            usb_option_clear_halt(urb->dev, urb->pipe);
        }
    }

	/* Resubmit urb so we continue receiving IRQ data */
	if (urb->status != -ESHUTDOWN) {
		urb->dev = serial->dev;
		err = usb_submit_urb(urb, GFP_ATOMIC);
		switch (err)
		{
            /*urb正确传输*/
            case 0:
                break;
            /*ep stall*/
            case -EPIPE:
                printk("ep stall %d\n", __LINE__);
                usb_option_clear_halt(urb->dev, urb->pipe);
                usb_submit_urb(urb, GFP_ATOMIC);
                break;
            default:
			    dbg("%s: submit irq_in urb failed %d",
				    __FUNCTION__, err);
                break;
		}
	}
}

static int option_write_room(struct usb_serial_port *port)
{
	struct option_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i=0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && this_urb->status != -EINPROGRESS)
			data_len += OUT_BUFLEN;
	}

	dbg("%s: %d", __FUNCTION__, data_len);
	return data_len;
}

static int option_chars_in_buffer(struct usb_serial_port *port)
{
	struct option_port_private *portdata;
	int i;
	int data_len = 0;
	struct urb *this_urb;

	portdata = usb_get_serial_port_data(port);

	for (i=0; i < N_OUT_URB; i++) {
		this_urb = portdata->out_urbs[i];
		if (this_urb && this_urb->status == -EINPROGRESS)
			data_len += this_urb->transfer_buffer_length;
	}
	dbg("%s: %d", __FUNCTION__, data_len);
	return data_len;
}

static int option_open(struct usb_serial_port *port, struct file *filp)
{
	struct option_port_private *portdata;
	struct usb_serial *serial = port->serial;
	int i, err;
	struct urb *urb;

	portdata = usb_get_serial_port_data(port);

	dbg("%s", __FUNCTION__);

	if (iUsbTtyState == 3 && port->number == 2)
	{
		iUsbModemOpened = 1;
	}

	if (iUsbTtyState == 2 && port->number == 1)
	{
		iUsbModemOpened = 1;
	}

	/* Set some sane defaults */
	portdata->rts_state = 1;
	portdata->dtr_state = 1;

	/* Reset low level data toggle and start reading from endpoints */
	for (i = 0; i < N_IN_URB; i++) 
    {
		urb = portdata->in_urbs[i];
		if (! urb)
			continue;
		if (urb->dev != serial->dev) {
		
			continue;
		}

		/*
		 * make sure endpoint data toggle is synchronized with the
		 * device
		 */
		usb_option_clear_halt(urb->dev, urb->pipe);

		usb_submit_urb(urb, GFP_KERNEL);
	}

	/* Reset low level data toggle on out endpoints */
	for (i = 0; i < N_OUT_URB; i++) {
		urb = portdata->out_urbs[i];
		if (! urb)
			continue;
		urb->dev = serial->dev;
		/* usb_settoggle(urb->dev, usb_pipeendpoint(urb->pipe),
				usb_pipeout(urb->pipe), 0); */


        usb_option_clear_halt(urb->dev, urb->pipe);
        
	}

	port->tty->low_latency = 1;

    usb_option_clear_halt(urb->dev, urb->pipe);

	option_send_setup(port);

	return (0);
}


static void option_close(struct usb_serial_port *port, struct file *filp)
{
	int i;
	struct usb_serial *serial = port->serial;
	struct option_port_private *portdata;

	dbg("%s", __FUNCTION__);
	portdata = usb_get_serial_port_data(port);

	if (iUsbTtyState == 3 && port->number == 2)
	{
		iUsbModemOpened = 0;
	}

	if (iUsbTtyState == 2 && port->number == 1)
	{
		iUsbModemOpened = 0;
	}
	

	portdata->rts_state = 0;
	portdata->dtr_state = 0;

	if (serial->dev) {
		option_send_setup(port);

		/* Stop reading/writing urbs */
		for (i = 0; i < N_IN_URB; i++)
			usb_kill_urb(portdata->in_urbs[i]);
		for (i = 0; i < N_OUT_URB; i++)
			usb_kill_urb(portdata->out_urbs[i]);
	}
	port->tty = NULL;
}

/* Helper functions used by option_setup_urbs */
static struct urb *option_setup_urb(struct usb_serial *serial, int endpoint,
		int dir, void *ctx, char *buf, int len,
		void (*callback)(struct urb *))
{
	struct urb *urb;

	if (endpoint == -1)
		return NULL;		/* endpoint not needed */

	urb = usb_alloc_urb(0, GFP_KERNEL);		/* No ISO */
	if (urb == NULL) {
		dbg("%s: alloc for endpoint %d failed.", __FUNCTION__, endpoint);
		return NULL;
	}

		/* Fill URB using supplied data. */
	usb_fill_bulk_urb(urb, serial->dev,
		      usb_sndbulkpipe(serial->dev, endpoint) | dir,
		      buf, len, callback, ctx);

	return urb;
}

/* Setup urbs */
static void option_setup_urbs(struct usb_serial *serial)
{
	int i,j;
	struct usb_serial_port *port;
	struct option_port_private *portdata;

	dbg("%s", __FUNCTION__);

	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

	/* Do indat endpoints first */
		for (j = 0; j < N_IN_URB; ++j) {
			portdata->in_urbs[j] = option_setup_urb (serial,
                  	port->bulk_in_endpointAddress, USB_DIR_IN, port,
                  	portdata->in_buffer[j], IN_BUFLEN, option_indat_callback);
		}

		/* outdat endpoints */
		for (j = 0; j < N_OUT_URB; ++j) {
			portdata->out_urbs[j] = option_setup_urb (serial,
                  	port->bulk_out_endpointAddress, USB_DIR_OUT, port,
                  	portdata->out_buffer[j], OUT_BUFLEN, option_outdat_callback);
		}
	}
}

static int option_send_setup(struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	struct option_port_private *portdata;
    int ret = -1;

	dbg("%s", __FUNCTION__);

	if (port->number != 0)
		return 0;

	portdata = usb_get_serial_port_data(port);

	if (port->tty) {
		int val = 0;
		if (portdata->dtr_state)
			val |= 0x01;
		if (portdata->rts_state)
			val |= 0x02;

		return usb_control_msg(serial->dev,
				usb_rcvctrlpipe(serial->dev, 0),
				0x22,0x21,val,0,NULL,0,USB_CTRL_SET_TIMEOUT);

	}

	return 0;
}

static int option_startup(struct usb_serial *serial)
{
	int i, err;
	struct usb_serial_port *port;
	struct option_port_private *portdata;
	int pid = 0;
    struct task_struct* ttyUsb = NULL;
    
    
	dbg("%s", __FUNCTION__);

    if (iUsbTtyState >= 4)
    {
        return -EUSERS;
    }

	/* Now setup per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		portdata = kmalloc(sizeof(*portdata), GFP_KERNEL);
		if (!portdata) {
			dbg("%s: kmalloc for option_port_private (%d) failed!.",
					__FUNCTION__, i);
			return (1);
		}

        memset(portdata, 0, sizeof(*portdata));
    
		usb_set_serial_port_data(port, portdata);

		if (! port->interrupt_in_urb)
			continue;
		err = usb_submit_urb(port->interrupt_in_urb, GFP_KERNEL);
		switch (err)
		{
            /*urb正确传输*/
            case 0:
                break;
            /*ep stall*/
            case -EPIPE:
                usb_option_clear_halt(port->interrupt_in_urb->dev, port->interrupt_in_urb->pipe);
                break;
            default:
			    dbg("%s: submit irq_in urb failed %d",
				    __FUNCTION__, err);
                break;
		}
	}

	option_setup_urbs(serial);
    
    ++iUsbTtyState;
    kerSysWakeupMonitorTask();

    if (iUsbTtyState == 2)
    {
        pid = simple_strtol(ttyUSB_pid, NULL, 10); 

        if ( pid != 0 )
        {
            ttyUsb = find_task_by_pid(pid);
            if(ttyUsb)
            {
                send_sig(SIGUSR1, ttyUsb, 0);
            }
        }
    }
    
 #if 0
	lHspaFd = sys_open("/var/HspaStatus",O_RDONLY, 0);
	if(lHspaFd>0)
	{
		sys_lseek(lHspaFd , 0,0);
		sys_read(lHspaFd, &pid, sizeof(pid));
		if(iUsbTtyState == 2)
		{
            printk("option send sig iUsbTtyState==2 \n");
			sys_kill(pid, SIGUSR1);
		}
		sys_close(lHspaFd);
	}
 #endif
 
	return (0);
}

static void option_shutdown(struct usb_serial *serial)
{
	int i, j;
	struct usb_serial_port *port;
	struct option_port_private *portdata;

	dbg("%s", __FUNCTION__);

	/* Stop reading/writing urbs */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);
		for (j = 0; j < N_IN_URB; j++)
			usb_kill_urb(portdata->in_urbs[j]);
		for (j = 0; j < N_OUT_URB; j++)
			usb_kill_urb(portdata->out_urbs[j]);
	}

	/* Now free them */
	for (i = 0; i < serial->num_ports; ++i) {
		port = serial->port[i];
		portdata = usb_get_serial_port_data(port);

		for (j = 0; j < N_IN_URB; j++) {
			if (portdata->in_urbs[j]) {
				usb_free_urb(portdata->in_urbs[j]);
				portdata->in_urbs[j] = NULL;
			}
		}
		for (j = 0; j < N_OUT_URB; j++) {
			if (portdata->out_urbs[j]) {
				usb_free_urb(portdata->out_urbs[j]);
				portdata->out_urbs[j] = NULL;
			}
		}
	}

	/* Now free per port private data */
	for (i = 0; i < serial->num_ports; i++) {
		port = serial->port[i];
		kfree(usb_get_serial_port_data(port));
	}
}

static int usb_option_clear_halt(struct usb_device *dev, int pipe)
{
    int result;
	int endp = usb_pipeendpoint(pipe);
	
	if (usb_pipein (pipe))
		endp |= USB_DIR_IN;

	/* we don't care if it wasn't halted first. in fact some devices
	 * (like some ibmcam model 1 units) seem to expect hosts to make
	 * this request for iso endpoints, which can't halt!
	 */

    //printk("ep %d line %d \n", endp, __LINE__);
    
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT,
		USB_ENDPOINT_HALT, endp, NULL, 0,
		HZ * USB_CTRL_SET_TIMEOUT);

	/* don't un-halt or force to DATA0 except on success */
	if (result < 0)
		return result;

	/* NOTE:  seems like Microsoft and Apple don't bother verifying
	 * the clear "took", so some devices could lock up if you check...
	 * such as the Hagiwara FlashGate DUAL.  So we won't bother.
	 *
	 * NOTE:  make sure the logic here doesn't diverge much from
	 * the copy in usb-storage, for as long as we need two copies.
	 */

	/* toggle was reset by the clear, then ep was reactivated */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	return 0;
}





MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

#ifdef CONFIG_USB_DEBUG
module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug messages");
#endif

