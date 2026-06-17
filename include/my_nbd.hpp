#ifndef __MY_NBD_HPP__
#define __MY_NBD_HPP__

/* SPDX-License-Identifier: GPL-1.0+ WITH Linux-syscall-note */
/*
 * 1999 Copyright (C) Pavel Machek, pavel@ucw.cz. This code is GPL.
 * 1999/11/04 Copyright (C) 1999 VMware, Inc. (Regis "HPReg" Duchesne)
 *            Made nbd_end_request() use the io_request_lock
 * 2001 Copyright (C) Steven Whitehouse
 *            New nbd_end_request() for compatibility with new linux block
 *            layer code.
 * 2003/06/24 Louis D. Langholtz <ldl@aros.net>
 *            Removed unneeded blksize_bits field from nbd_device struct.
 *            Cleanup PARANOIA usage & code.
 * 2004/02/19 Paul Clements
 *            Removed PARANOIA, plus various cleanup and comments
 * 2023 Copyright Red Hat
 *            Link to userspace extensions, favor cookie over handle.
 */

#ifndef LINUX_NBD_H
#define LINUX_NBD_H

#include <linux/types.h>

#define NBD_SET_SOCK	_IO( 0xab, 0 )
#define NBD_SET_BLKSIZE	_IO( 0xab, 1 )
#define NBD_SET_SIZE	_IO( 0xab, 2 )
#define NBD_DO_IT	_IO( 0xab, 3 )
#define NBD_CLEAR_SOCK	_IO( 0xab, 4 )
#define NBD_CLEAR_QUE	_IO( 0xab, 5 )
#define NBD_PRINT_DEBUG	_IO( 0xab, 6 )
#define NBD_SET_SIZE_BLOCKS	_IO( 0xab, 7 )
#define NBD_DISCONNECT  _IO( 0xab, 8 )
#define NBD_SET_TIMEOUT _IO( 0xab, 9 )
#define NBD_SET_FLAGS   _IO( 0xab, 10)

/*
 * See also https://github.com/NetworkBlockDevice/nbd/blob/master/doc/proto.md
 * for additional userspace extensions not yet utilized in the kernel module.
 */

enum {
	NBD_CMD_READ = 0,
	NBD_CMD_WRITE = 1,
	NBD_CMD_DISC = 2,
	NBD_CMD_FLUSH = 3,
	NBD_CMD_TRIM = 4
	/* userspace defines additional extension commands */
};

/* values for flags field, these are server interaction specific. */
#define NBD_FLAG_HAS_FLAGS	(1 << 0) /* nbd-server supports flags */
#define NBD_FLAG_READ_ONLY	(1 << 1) /* device is read-only */
#define NBD_FLAG_SEND_FLUSH	(1 << 2) /* can flush writeback cache */
#define NBD_FLAG_SEND_FUA	(1 << 3) /* send FUA (forced unit access) */
/* there is a gap here to match userspace */
#define NBD_FLAG_SEND_TRIM	(1 << 5) /* send trim/discard */
#define NBD_FLAG_CAN_MULTI_CONN	(1 << 8)	/* Server supports multiple connections per export. */

/* values for cmd flags in the upper 16 bits of request type */
#define NBD_CMD_FLAG_FUA	(1 << 16) /* FUA (forced unit access) op */

/* These are client behavior specific flags. */
#define NBD_CFLAG_DESTROY_ON_DISCONNECT	(1 << 0) /* delete the nbd device on
						    disconnect. */
#define NBD_CFLAG_DISCONNECT_ON_CLOSE (1 << 1) /* disconnect the nbd device on
						*  close by last opener.
						*/

/* userspace doesn't need the nbd_device structure */

/* These are sent over the network in the request/reply magic fields */

#define NBD_REQUEST_MAGIC 0x25609513
#define NBD_REPLY_MAGIC 0x67446698
/* Do *not* use magics: 0x12560953 0x96744668. */
/* magic 0x668e33ef for structured reply not supported by kernel yet */

/*
 * This is the packet used for communication between client and
 * server. All data are in network byte order.
 */
struct nbd_request {
	__be32 magic;	/* NBD_REQUEST_MAGIC	*/
	__be32 type;	/* See NBD_CMD_*	*/
	union {
		__be64 cookie;	/* Opaque identifier for request	*/
		char handle[8];	/* older spelling of cookie		*/
	};
	__be64 from;
	__be32 len;
} __attribute__((packed));

/*
 * This is the reply packet that nbd-server sends back to the client after
 * it has completed an I/O request (or an error occurs).
 */
struct nbd_reply {
	__be32 magic;		/* NBD_REPLY_MAGIC	*/
	__be32 error;		/* 0 = ok, else error	*/
	union {
		__be64 cookie;	/* Opaque identifier from request	*/
		char handle[8];	/* older spelling of cookie		*/
	};
};
#endif /* LINUX_NBD_H */


/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (C) 2017 Facebook.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License v2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 */
#ifndef LINUX_NBD_NETLINK_H
#define LINUX_NBD_NETLINK_H

#define NBD_GENL_FAMILY_NAME		"nbd"
#define NBD_GENL_VERSION		0x1
#define NBD_GENL_MCAST_GROUP_NAME	"nbd_mc_group"

/* Configuration policy attributes, used for CONNECT */
enum {
	NBD_ATTR_UNSPEC,
	NBD_ATTR_INDEX,
	NBD_ATTR_SIZE_BYTES,
	NBD_ATTR_BLOCK_SIZE_BYTES,
	NBD_ATTR_TIMEOUT,
	NBD_ATTR_SERVER_FLAGS,
	NBD_ATTR_CLIENT_FLAGS,
	NBD_ATTR_SOCKETS,
	NBD_ATTR_DEAD_CONN_TIMEOUT,
	NBD_ATTR_DEVICE_LIST,
	NBD_ATTR_BACKEND_IDENTIFIER,
	__NBD_ATTR_MAX,
};
#define NBD_ATTR_MAX (__NBD_ATTR_MAX - 1)

/*
 * This is the format for multiple devices with NBD_ATTR_DEVICE_LIST
 *
 * [NBD_ATTR_DEVICE_LIST]
 *   [NBD_DEVICE_ITEM]
 *     [NBD_DEVICE_INDEX]
 *     [NBD_DEVICE_CONNECTED]
 */
enum {
	NBD_DEVICE_ITEM_UNSPEC,
	NBD_DEVICE_ITEM,
	__NBD_DEVICE_ITEM_MAX,
};
#define NBD_DEVICE_ITEM_MAX (__NBD_DEVICE_ITEM_MAX - 1)

enum {
	NBD_DEVICE_UNSPEC,
	NBD_DEVICE_INDEX,
	NBD_DEVICE_CONNECTED,
	__NBD_DEVICE_MAX,
};
#define NBD_DEVICE_ATTR_MAX (__NBD_DEVICE_MAX - 1)

/*
 * This is the format for multiple sockets with NBD_ATTR_SOCKETS
 *
 * [NBD_ATTR_SOCKETS]
 *   [NBD_SOCK_ITEM]
 *     [NBD_SOCK_FD]
 *   [NBD_SOCK_ITEM]
 *     [NBD_SOCK_FD]
 */
enum {
	NBD_SOCK_ITEM_UNSPEC,
	NBD_SOCK_ITEM,
	__NBD_SOCK_ITEM_MAX,
};
#define NBD_SOCK_ITEM_MAX (__NBD_SOCK_ITEM_MAX - 1)

enum {
	NBD_SOCK_UNSPEC,
	NBD_SOCK_FD,
	__NBD_SOCK_MAX,
};
#define NBD_SOCK_MAX (__NBD_SOCK_MAX - 1)

enum {
	NBD_CMD_UNSPEC,
	NBD_CMD_CONNECT,
	NBD_CMD_DISCONNECT,
	NBD_CMD_RECONFIGURE,
	NBD_CMD_LINK_DEAD,
	NBD_CMD_STATUS,
	__NBD_CMD_MAX,
};
#define NBD_CMD_MAX	(__NBD_CMD_MAX - 1)

#endif /* LINUX_NBD_NETLINK_H */

#endif //__MY_NBD_HPP__