/*----------------------------------------------------------------------------*/
/* Hobbit monitor library.                                                    */
/*                                                                            */
/* Copyright (C) 2002-2006 Henrik Storner <henrik@storner.dk>                 */
/*                                                                            */
/* This program is released under the GNU General Public License (GPL),       */
/* version 2. See the file "COPYING" for details.                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#ifndef __LIBBBGEN_H__
#define __LIBBBGEN_H__

#include <stdio.h>
#include <time.h>

typedef struct htnames_t {
	char *name;
	struct htnames_t *next;
} htnames_t;

typedef struct strbuffer_t {
	char *s;
	int used, sz;
} strbuffer_t;

#define STRBUF(buf) (buf->s)
#define STRBUFLEN(buf) (buf->used)

#define IP_ADDR_STRLEN 16

#include "version.h"
#include "config.h"
#include "../lib/osdefs.h"

/* Defines CGI URL's */
#include "../lib/cgiurls.h"
#include "../lib/links.h"

/* Generates HTML */
#include "../lib/acklog.h"
#include "../lib/eventlog.h"
#include "../lib/headfoot.h"
#include "../lib/htmllog.h"
#include "../lib/reportlog.h"

#include "../lib/availability.h"
#include "../lib/calc.h"
#include "../lib/cgi.h"
#include "../lib/color.h"
#include "../lib/digest.h"
#include "../lib/encoding.h"
#include "../lib/environ.h"
#include "../lib/errormsg.h"
#include "../lib/files.h"
#include "../lib/hobbitrrd.h"
#include "../lib/loadalerts.h"
#include "../lib/loadhosts.h"
#include "../lib/loadnkconf.h"
#include "../lib/matching.h"
#include "../lib/md5.h"
#include "../lib/memory.h"
#include "../lib/misc.h"
#include "../lib/netservices.h"
#include "../lib/rbtr.h"
#include "../lib/rmd160c.h"
#include "../lib/sendmsg.h"
#include "../lib/sha1.h"
#include "../lib/sig.h"
#include "../lib/stackio.h"
#include "../lib/strfunc.h"
#include "../lib/timefunc.h"
#include "../lib/timing.h"
#include "../lib/url.h"

#endif

