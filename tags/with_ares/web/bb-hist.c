/*----------------------------------------------------------------------------*/
/* Big Brother webpage generator tool.                                        */
/*                                                                            */
/* This is a replacement for the "bb-hist.sh" script                          */
/* It includes additional features like summary graphs and statistics for     */
/* the past week, month and year; in addition to being loads faster than      */
/* the original script.                                                       */
/*                                                                            */
/* Copyright (C) 2003 Henrik Storner <henrik@storner.dk>                      */
/*                                                                            */
/* This program is released under the GNU General Public License (GPL),       */
/* version 2. See the file "COPYING" for details.                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/

static char rcsid[] = "$Id: bb-hist.c,v 1.33 2003-09-12 11:43:29 henrik Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bbgen.h"
#include "util.h"
#include "reportdata.h"
#include "debug.h"

static char selfurl[MAX_PATH];
static time_t req_endtime = 0;

static int len1d = 24;
static char *bartitle1d = "1 day summary";
static int len1w = 7;
static char *bartitle1w = "1 week summary";
static int len4w = 28;
static char *bartitle4w = "4 week summary";
static int len1y = 12;
static char *bartitle1y = "1 year summary";

/* DEFPIXELS is defined by the configure script */
#ifndef DEFPIXELS
static int usepct = 1;
#define DEFPIXELS 0
#else
static int usepct = 0;
#endif
static int pixels = DEFPIXELS;

/* What colorbars and summaries to show by default */
#define BARSUM_1D 0x0001	/* 1-day bar */
#define BARSUM_1W 0x0002	/* 1-week bar */
#define BARSUM_4W 0x0004	/* 4-week bar */
#define BARSUM_1Y 0x0008	/* 1-year bar */

/* DEFBARSUMS is defined by the configure script */
#ifndef DEFBARSUMS
#define DEFBARSUMS (BARSUM_1D|BARSUM_1W)
#endif

static unsigned int barsums = DEFBARSUMS;

static char *barbkgcolor = "\"#000033\"";
static char *tagcolors[COL_COUNT] = {
	"#3AF03A",	/* A bright green */
	"white",
	"blue",
	"purple",
	"yellow",
	"red"
};

#define ALIGN_HOUR  0
#define ALIGN_DAY   1
#define ALIGN_MONTH 2

#define DAY_BAR 0
#define WEEK_BAR 1
#define MONTH_BAR 2
#define YEAR_BAR 3

#define END_START 0
#define END_END 1
#define END_UNCHANGED 2

static void generate_pct_summary(
			FILE *htmlrep,			/* output file */
			char *hostname,
			char *service,
			char *caption,
			reportinfo_t *repinfo, 		/* Percent summaries for period */
			time_t secsperpixel)
{
	fprintf(htmlrep, "<TABLE BORDER=0 BGCOLOR=%s CELLPADDING=3>\n", barbkgcolor);

	fprintf(htmlrep, "<TR BGCOLOR=\"#333333\"><TD COLSPAN=6 ALIGN=CENTER><FONT SIZE=\"+1\">%s</FONT></TD></TR>\n", caption);
	fprintf(htmlrep, "<TR BGCOLOR=\"#333333\"><TD COLSPAN=6 ALIGN=CENTER><FONT SIZE=\"-1\">Min. duration shown: %s</FONT></TD></TR>\n", 
		durationstr(secsperpixel / 2));

	fprintf(htmlrep, "<TR BGCOLOR=\"#000000\">\n");

	fprintf(htmlrep, "<TD ALIGN=CENTER><IMG SRC=\"%s/%s\" ALT=\"%s\" HEIGHT=%s WIDTH=%s BORDER=0></TD>\n", 
		getenv("BBSKIN"), dotgiffilename(COL_GREEN, 0, 1), colorname(COL_GREEN), getenv("DOTHEIGHT"), getenv("DOTWIDTH"));
	fprintf(htmlrep, "<TD ALIGN=CENTER><IMG SRC=\"%s/%s\" ALT=\"%s\" HEIGHT=%s WIDTH=%s BORDER=0></TD>\n", 
		getenv("BBSKIN"), dotgiffilename(COL_YELLOW, 0, 1), colorname(COL_YELLOW), getenv("DOTHEIGHT"), getenv("DOTWIDTH"));
	fprintf(htmlrep, "<TD ALIGN=CENTER><IMG SRC=\"%s/%s\" ALT=\"%s\" HEIGHT=%s WIDTH=%s BORDER=0></TD>\n", 
		getenv("BBSKIN"), dotgiffilename(COL_RED, 0, 1), colorname(COL_RED), getenv("DOTHEIGHT"), getenv("DOTWIDTH"));
	fprintf(htmlrep, "<TD ALIGN=CENTER><IMG SRC=\"%s/%s\" ALT=\"%s\" HEIGHT=%s WIDTH=%s BORDER=0></TD>\n", 
		getenv("BBSKIN"), dotgiffilename(COL_PURPLE, 0, 1), colorname(COL_PURPLE), getenv("DOTHEIGHT"), getenv("DOTWIDTH"));
	fprintf(htmlrep, "<TD ALIGN=CENTER><IMG SRC=\"%s/%s\" ALT=\"%s\" HEIGHT=%s WIDTH=%s BORDER=0></TD>\n", 
		getenv("BBSKIN"), dotgiffilename(COL_CLEAR, 0, 1), colorname(COL_CLEAR), getenv("DOTHEIGHT"), getenv("DOTWIDTH"));
	fprintf(htmlrep, "<TD ALIGN=CENTER><IMG SRC=\"%s/%s\" ALT=\"%s\" HEIGHT=%s WIDTH=%s BORDER=0></TD>\n", 
		getenv("BBSKIN"), dotgiffilename(COL_BLUE, 0, 1), colorname(COL_BLUE), getenv("DOTHEIGHT"), getenv("DOTWIDTH"));
	fprintf(htmlrep, "</TR>\n");
	fprintf(htmlrep, "<TR BGCOLOR=\"#000033\">\n");
	fprintf(htmlrep, "<TD ALIGN=CENTER><B>%.2f%%</B></TD>\n", repinfo->fullpct[COL_GREEN]);
	fprintf(htmlrep, "<TD ALIGN=CENTER><B>%.2f%%</B></TD>\n", repinfo->fullpct[COL_YELLOW]);
	fprintf(htmlrep, "<TD ALIGN=CENTER><B>%.2f%%</B></TD>\n", repinfo->fullpct[COL_RED]);
	fprintf(htmlrep, "<TD ALIGN=CENTER><B>%.2f%%</B></TD>\n", repinfo->fullpct[COL_PURPLE]);
	fprintf(htmlrep, "<TD ALIGN=CENTER><B>%.2f%%</B></TD>\n", repinfo->fullpct[COL_CLEAR]);
	fprintf(htmlrep, "<TD ALIGN=CENTER><B>%.2f%%</B></TD>\n", repinfo->fullpct[COL_BLUE]);
	fprintf(htmlrep, "</TR>\n");

	fprintf(htmlrep, "</TABLE>\n");

}

static unsigned int calc_time(time_t endtime, int change, int alignment, int endofperiod)
{
	int daysinmonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	struct tm *tmbuf;
	time_t result, now;
	int dstsetting = -1;

again:
	tmbuf = localtime(&endtime);
	switch (alignment) {
		case ALIGN_HOUR: 
			tmbuf->tm_hour += change;
			if (endofperiod == END_END) {
				tmbuf->tm_min = tmbuf->tm_sec = 59;
			}
			else if (endofperiod == END_START) {
				tmbuf->tm_min = tmbuf->tm_sec = 0;
			}
			break;

		case ALIGN_DAY:
			tmbuf->tm_mday += change;
			if (endofperiod == END_END) {
				tmbuf->tm_hour = 23;
				tmbuf->tm_min = 59;
				tmbuf->tm_sec = 59;
			}
			else if (endofperiod == END_START) {
				tmbuf->tm_hour = tmbuf->tm_min = tmbuf->tm_sec = 0;
			}
			break;

		case ALIGN_MONTH:
			tmbuf->tm_mon += change;

			if (endofperiod == END_END) {
				/* Need to find the last day of the month */
				tmbuf->tm_mday = daysinmonth[tmbuf->tm_mon];
				if (tmbuf->tm_mon == 1) {
					if (((tmbuf->tm_year + 1900) % 4) == 0) {
						tmbuf->tm_mday = 29;
						if (((tmbuf->tm_year + 1900) % 100) == 0) tmbuf->tm_mday = 28;
						if (((tmbuf->tm_year + 1900) % 400) == 0) tmbuf->tm_mday = 29;
					}
				}

				tmbuf->tm_hour = 23;
				tmbuf->tm_min = 59;
				tmbuf->tm_sec = 59;
			}
			else if (endofperiod == END_START) {
				tmbuf->tm_mday = 1;
				tmbuf->tm_hour = tmbuf->tm_min = tmbuf->tm_sec = 0;
			}
			break;
	}
	tmbuf->tm_isdst = dstsetting;
	result = mktime(tmbuf);
	if ((dstsetting == -1) && (endofperiod == END_END) && (result < endtime)) {
		/* DST->normaltime switchover - redo with forced DST setting */
		dprintf("DST rollover with endtime/change/alignment/endodperiod = %u/%d/%d/%d\n",
			(unsigned int)endtime, change, alignment, endofperiod);
		dstsetting = 0;
		goto again;
	}

	/* Dont try to foresee the future */
	now = time(NULL);
	if (result > now) result = now;

	return (unsigned int)result;
}

static int maxcolor(replog_t *periodlog, time_t begintime, time_t endtime)
{
	int result = COL_GREEN;
	replog_t *walk = periodlog;

	while (walk) {
		if (walk->color > result) {
			/*
			 * We want this event, IF:
			 * - it starts sometime during begintime -> endtime, or
			 * - it starts before begintime, but lasts into after begintime.
			 */
			if ( ((walk->starttime >= begintime) && (walk->starttime < endtime))  ||
			     ((walk->starttime <  begintime) && ((walk->starttime + walk->duration) >= begintime)) ) {
				result = walk->color;
			}
		}

		walk = walk->next;
	}

	return result;
}


static void generate_colorbar(
			FILE *htmlrep,		/* Output file */
			time_t begintime,
			time_t endtime,
			int alignment,		/* Align by hour/day/month */
			int bartype,            /* Day/Week/Month/Year bar */
			char *hostname,
			char *service,
			char *caption,		/* Title */
			replog_t *periodlog,	/* Log entries for period */
			reportinfo_t *repinfo) 	/* Info for the percent summary */
{
	int secsperpixel;
	char *pctstr = "";
	replog_t *colorlog, *walk;
	int changeval = 0;
	int changealign = 0;

	/*
	 * Pixel-based charts are better, but for backwards
	 * compatibility allow for a graph that has 100 "pixels"
	 * and adds a "%" to the width specs.
	 */
	if (usepct) {
		pixels = 100;
		pctstr = "%";
	}

	/* How many seconds required for 1 pixel */
	secsperpixel = ((endtime - begintime) / pixels);

	/* Need to re-sort the period-log to chronological order */
	colorlog = NULL;
	{
		replog_t *tmp;
		for (walk = periodlog; (walk); walk = tmp) {
			tmp = walk->next;
			walk->next = colorlog;
			colorlog = walk;
			walk = tmp;
		}
	}

	/* Determine the back/forward link times */
	switch (bartype) {
		case DAY_BAR   : changeval = len1d; changealign = ALIGN_HOUR; break;
		case WEEK_BAR  : changeval = len1w; changealign = ALIGN_DAY; break;
		case MONTH_BAR : changeval = len4w; changealign = ALIGN_DAY; break;
		case YEAR_BAR  : changeval = len1y; changealign = ALIGN_MONTH; break;
	}

	/* Beginning of page */
	fprintf(htmlrep, "<TABLE SUMMARY=\"Bounding rectangle\" WIDTH=\"%d%s\" BORDER=0 BGCOLOR=\"#666666\">\n", pixels, pctstr);
	fprintf(htmlrep, "<TR><TD>\n");


	/* The date stamps, percent summaries and zoom/reset links */
	fprintf(htmlrep, "<TABLE SUMMARY=\"%s\" WIDTH=\"100%%\" BORDER=0 FRAME=VOID CELLSPACING=0 CELLPADDING=1 BGCOLOR=\"#000033\">\n", caption);
	fprintf(htmlrep, "<TR BGCOLOR=%s><TD>\n", barbkgcolor);

	fprintf(htmlrep, "  <TABLE SUMMARY=\"Adjustment, Past navigation\" WIDTH=\"100%%\" BORDER=0 CELLSPACING=0 CELLPADDING=0>\n");
	if (usepct) {
		fprintf(htmlrep, "  <TR><TD ALIGN=RIGHT VALIGN=TOP><A HREF=\"%s&amp&amp;PIXELS=%d\">Time reset</A></TD></TR>\n", 
			selfurl, (usepct ? 0 : pixels));
	}
	else {
		fprintf(htmlrep, "  <TR><TD ALIGN=RIGHT VALIGN=TOP><A HREF=\"%s&amp;ENDTIME=%u&amp;PIXELS=%d\">Zoom +</A></TD></TR>\n", 
			selfurl, (unsigned int)endtime, pixels+200);
		if (pixels > 200) {
			fprintf(htmlrep, "  <TR><TD ALIGN=RIGHT VALIGN=TOP><A HREF=\"%s&amp;ENDTIME=%u&amp;PIXELS=%d\">Zoom -</A></TD></TR>\n", 
				selfurl, (unsigned int)endtime, pixels-200);
		}
	}
	fprintf(htmlrep, "  <TR><TD ALIGN=LEFT VALIGN=BOTTOM><BR>\n");
	
	if (colorlog && colorlog->starttime <= begintime) {
		fprintf(htmlrep, "<A HREF=\"%s&amp;ENDTIME=%u&amp;PIXELS=%d\">", 
			selfurl, calc_time(endtime, -changeval, changealign, END_UNCHANGED), (usepct ? 0 : pixels));
	}
	fprintf(htmlrep, "<B>%s</B>", ctime(&begintime));
	if (colorlog && colorlog->starttime <= begintime) fprintf(htmlrep, "</A>");
	fprintf(htmlrep, "\n  </TD></TR>\n");
	fprintf(htmlrep, "  </TABLE>\n");
	fprintf(htmlrep, "</TD>\n");
	
	fprintf(htmlrep, "<TD ALIGN=CENTER>\n");
	generate_pct_summary(htmlrep, hostname, service, caption, repinfo, secsperpixel);
	fprintf(htmlrep, "</TD>\n");

	fprintf(htmlrep, "<TD>\n");
	fprintf(htmlrep, "  <TABLE SUMMARY=\"Adjustment, Future navigation\" WIDTH=\"100%%\" BORDER=0 CELLSPACING=0 CELLPADDING=0>\n");
	fprintf(htmlrep, "  <TR><TD ALIGN=LEFT VALIGN=TOP><A HREF=\"%s&amp&amp;PIXELS=%d\">Time reset</A></TD></TR>\n", 
		selfurl, (usepct ? 0 : pixels));
	if (!usepct) {
		fprintf(htmlrep, "  <TR><TD ALIGN=LEFT VALIGN=TOP><A HREF=\"%s&amp;ENDTIME=%u&amp;PIXELS=%d\">Zoom reset</A></TD></TR>\n", 
			selfurl, (unsigned int)endtime, DEFPIXELS);
	}

	fprintf(htmlrep, "  <TR><TD ALIGN=RIGHT VALIGN=BOTTOM><BR>\n");
	fprintf(htmlrep, "  <A HREF=\"%s&amp;ENDTIME=%d&amp;PIXELS=%d\">", selfurl, 
		calc_time(endtime, +changeval, changealign, END_UNCHANGED), (usepct ? 0 : pixels));
	fprintf(htmlrep, "<B>%s</B>", ctime(&endtime));
	fprintf(htmlrep, "</A>\n");
	fprintf(htmlrep, "  </TD></TR>\n");
	fprintf(htmlrep, "  </TABLE>\n");
	fprintf(htmlrep, "</TD>\n");

	fprintf(htmlrep, "</TR>\n");
	fprintf(htmlrep, "<TR BGCOLOR=%s><TD COLSPAN=5><HR></TD></TR>\n", barbkgcolor);
	fprintf(htmlrep, "</TABLE>\n");


	/* The period marker line */
	fprintf(htmlrep, "<TABLE SUMMARY=\"Periods\" WIDTH=\"100%%\" BORDER=0 FRAME=VOID CELLSPACING=0 CELLPADDING=0 BGCOLOR=\"#000033\">\n");
	fprintf(htmlrep, "<TR>\n");

	{
		time_t begininterval = begintime;
		time_t endofinterval;
		char tag[20];
		char *bgcols[2] = { "\"#000000\"", "\"#555555\"" };
		int curbg = 0;
		int intervalpixels, tagcolor;
		time_t minduration = 1800;
		struct tm *tmbuf;

		do {
			endofinterval = calc_time(begininterval, 0, alignment, END_END);
			dprintf("Period starts %u ends %u - %s", 
				(unsigned int)begininterval, (unsigned int)endofinterval, 
				ctime(&endofinterval));

			tmbuf = localtime(&begininterval);
			switch (bartype) {
				case DAY_BAR   : 
					minduration = 1800;
					strftime(tag, sizeof(tag), "%H", tmbuf);
					break;
				case WEEK_BAR  : 
					minduration = 14400;
					strftime(tag, sizeof(tag), "%a", tmbuf);
					break;
				case MONTH_BAR : 
					minduration = 43200;
					strftime(tag, sizeof(tag), "%d", tmbuf);
					break;
				case YEAR_BAR  : 
					minduration = 10*86400;
					strftime(tag, sizeof(tag), "%b", tmbuf);
					break;
			}

			intervalpixels = ((endofinterval - begininterval) / secsperpixel);
			tagcolor = maxcolor(colorlog, begininterval, endofinterval);

			fprintf(htmlrep, "<TD WIDTH=\"%d%s\" ALIGN=CENTER BGCOLOR=%s>", intervalpixels, pctstr, bgcols[curbg]);
			if ((endofinterval - begininterval) > minduration) {
				int dolink = (colorlog && endofinterval >= colorlog->starttime);

				if (dolink) fprintf(htmlrep, "<A HREF=\"%s&amp;ENDTIME=%u&amp;PIXELS=%d\">",
						    selfurl, (unsigned int)endofinterval, 
						    (usepct ? 0 : pixels));
				fprintf(htmlrep, "<FONT COLOR=\"%s\"><B>%s</B></FONT>", 
					tagcolors[tagcolor], tag);
				if (dolink) fprintf(htmlrep, "</A>");
			}
			fprintf(htmlrep, "</TD>\n");

			curbg = (1 - curbg);

			if ((endofinterval + 1) <= begininterval) {
				/*
				 * This should not happen!
				 */
				fprintf(htmlrep, "Time moves backwards! begintime=%u, alignment=%d, begininterval=%u\n",
					  (unsigned int)begintime, alignment, (unsigned int)begininterval);
				begininterval = endtime;
			}

			begininterval = endofinterval + 1;
		} while (begininterval < endtime);
	}
	fprintf(htmlrep, "</TR>\n");
	fprintf(htmlrep, "</TABLE>\n");


	/* The actual color bar */
	fprintf(htmlrep, "<TABLE SUMMARY=\"Color status graph\" WIDTH=\"100%%\" BORDER=0 FRAME=VOID CELLSPACING=0 CELLPADDING=0 BGCOLOR=\"#000033\">\n");
	fprintf(htmlrep, "<TR>\n");

	/* First entry may not start at our report-start time */
	if (colorlog == NULL) {
		/* No data for period - all white */
		fprintf(htmlrep, "<TD WIDTH=\"100%%\" BGCOLOR=white NOWRAP>&nbsp</TD>\n");
	}
	else if (colorlog->starttime > begintime) {
		/* Data starts after the bar does - so a white period in front */
		int pixels = ((colorlog->starttime - begintime) / secsperpixel);

		if (((colorlog->starttime - begintime) >= (secsperpixel/2)) && (pixels == 0)) pixels = 1;
		if (pixels > 0) {
			fprintf(htmlrep, "<TD WIDTH=\"%d%s\" BGCOLOR=%s NOWRAP>&nbsp</TD>\n", pixels, pctstr, "white");
		}
	}

	for (walk = colorlog; (walk); walk = walk->next) {
		/* Show each interval we have data for */

		int pixels = (walk->duration / secsperpixel);

		/* Intervals that give between 0.5 and 1 pixel are enlarged */
		if ((walk->duration >= (secsperpixel/2)) && (pixels == 0)) pixels = 1;

		if (pixels > 0) {
			fprintf(htmlrep, "<TD WIDTH=\"%d%s\" BGCOLOR=%s NOWRAP>&nbsp</TD>\n", 
				pixels, pctstr, ((walk->color == COL_CLEAR) ? "white" : colorname(walk->color)));
		}
	}

	fprintf(htmlrep, "</TR>\n");
	fprintf(htmlrep, "</TABLE>\n");

	fprintf(htmlrep, "</TD>\n");
	fprintf(htmlrep, "</TR>\n");
	fprintf(htmlrep, "</TABLE>\n");
	fprintf(htmlrep, "<BR><BR>\n");

}


static void generate_histlog_table(FILE *htmlrep,
		char *hostname, char *service, int entrycount, replog_t *loghead)
{
	char *bgcols[2] = { "\"#000000\"", "\"#000033\"" };
	int curbg = 0;
	replog_t *walk;

	fprintf(htmlrep, "<TABLE BORDER=0 BGCOLOR=\"#333333\" CELLSPACING=3>\n");
	fprintf(htmlrep, "<TR>\n");
	if (entrycount) {
		fprintf(htmlrep, "<TD COLSPAN=3 ALIGN=CENTER><B>Last %d log entries</B> ", entrycount);
		fprintf(htmlrep, "<A HREF=\"%s&amp;ENDTIME=%u&amp;PIXELS=%d&amp;ENTRIES=all\">(Full HTML log)</A></TD>\n", 
			selfurl, (unsigned int)req_endtime, (usepct ? 0 : pixels));
	}
	else {
		fprintf(htmlrep, "<TD COLSPAN=3 ALIGN=CENTER><B>All log entries</B></TD>\n");
	}
	fprintf(htmlrep, "</TR>\n");
	fprintf(htmlrep, "<TR BGCOLOR=\"#333333\">\n");
	fprintf(htmlrep, "<TD ALIGN=CENTER><FONT %s><B>Date</B></FONT></TD>\n", getenv("MKBBCOLFONT"));
	fprintf(htmlrep, "<TD ALIGN=CENTER><FONT %s><B>Status</B></FONT></TD>\n", getenv("MKBBCOLFONT"));
	fprintf(htmlrep, "<TD ALIGN=CENTER><FONT %s><B>Duration</B></FONT></TD>\n", getenv("MKBBCOLFONT"));
	fprintf(htmlrep, "</TR>\n");

	for (walk = loghead; (walk); walk = walk->next) {
		char start[30];

		strftime(start, sizeof(start), "%a %b %d %H:%M:%S %Y", localtime(&walk->starttime));

		fprintf(htmlrep, "<TR BGCOLOR=%s>\n", bgcols[curbg]); curbg = (1-curbg);
		fprintf(htmlrep, "<TD ALIGN=LEFT NOWRAP>%s</TD>\n", start);
		fprintf(htmlrep, "<TD ALIGN=CENTER BGCOLOR=\"#000000\">");
		fprintf(htmlrep, "<A HREF=\"%s/bb-histlog.sh?HOST=%s&amp;SERVICE=%s&amp;TIMEBUF=%s\">", 
			getenv("CGIBINURL"), hostname, service, walk->timespec);
		fprintf(htmlrep, "<IMG SRC=\"%s/%s\" ALT=\"%s\" HEIGHT=%s WIDTH=%s BORDER=0>", 
			getenv("BBSKIN"), dotgiffilename(walk->color, 0, 1), colorname(walk->color),
			getenv("DOTHEIGHT"), getenv("DOTWIDTH"));
		fprintf(htmlrep, "</A></TD>\n");

		fprintf(htmlrep, "<TD ALIGN=CENTER>%s</TD>\n", durationstr(walk->duration));
		fprintf(htmlrep, "</TR>\n\n");
	}

	fprintf(htmlrep, "</TABLE>\n");
}


void generate_history(FILE *htmlrep, 			/* output file */
		      char *hostname, char *service, 	/* Host and service we report on */
		      char *ip, 			/* IP - for the header only */
		      time_t endtime,			/* End time of color-bar graphs */

                      time_t start1d,			/* Starttime of 1-day period */
		      reportinfo_t *repinfo1d, 		/* Percent summaries for 1-day period */
		      replog_t *log1d, 			/* Events during past 1 day */

                      time_t start1w,			/* Starttime of 1-week period */
		      reportinfo_t *repinfo1w, 		/* Percent summaries for 1-week period */
		      replog_t *log1w, 			/* Events during past 1 week */

                      time_t start4w,			/* Starttime of 4-week period */
		      reportinfo_t *repinfo4w, 		/* Percent summaries for 4-week period */
		      replog_t *log4w, 			/* Events during past 4 weeks */

                      time_t start1y,			/* Starttime of 1-year period */
		      reportinfo_t *repinfo1y, 		/* Percent summaries for 1-year period */
		      replog_t *log1y, 			/* Events during past 1 yeary */

		      int entrycount,			/* Log entry maxcount */
		      replog_t *loghead)		/* Eventlog for entrycount events back */
{
	sethostenv(hostname, ip, service, colorname(COL_GREEN));
	headfoot(htmlrep, "hist", "", "header", COL_GREEN);

	fprintf(htmlrep, "\n");
	fprintf(htmlrep, "<CENTER>\n");
	fprintf(htmlrep, "<BR><FONT %s><B>%s - %s</B></FONT>\n", getenv("MKBBROWFONT"), hostname, service);
	fprintf(htmlrep, "<BR><BR>\n");

	/* Create the color-bars */
	if (log1d) {
		/* 1-day bar */
		generate_colorbar(htmlrep, start1d, endtime, ALIGN_HOUR, DAY_BAR,
				  hostname, service, bartitle1d, log1d, repinfo1d);
	}

	if (log1w) {
		/* 1-week bar */
		generate_colorbar(htmlrep, start1w, endtime, ALIGN_DAY, WEEK_BAR,
				  hostname, service, bartitle1w, log1w, repinfo1w);
	}

	if (log4w) {
		/* 4-week bar */
		generate_colorbar(htmlrep, start4w, endtime, ALIGN_DAY, MONTH_BAR,
				  hostname, service, bartitle4w, log4w, repinfo4w);
	}

	if (log1y) {
		/* 1-year bar */
		generate_colorbar(htmlrep, start1y, endtime, ALIGN_MONTH, YEAR_BAR,
				  hostname, service, bartitle1y, log1y, repinfo1y);
	}

	/* Last N histlog entries */
	fprintf(htmlrep, "<CENTER>\n");
	generate_histlog_table(htmlrep, hostname, service, entrycount, loghead);
	fprintf(htmlrep, "</CENTER>\n");

	fprintf(htmlrep, "<BR><BR>\n");

	/* BBHISTEXT extensions */
	do_bbext(htmlrep, "BBHISTEXT", "hist");

	fprintf(htmlrep, "</CENTER>\n");

	headfoot(htmlrep, "histlog", "", "footer", COL_GREEN);
}


/*
 * This program is invoked via CGI with QUERY_STRING containing:
 *
 *	HISTFILE=www,sample,com.conn
 *	ENTRIES=50
 */

/* These are dummy vars needed by stuff in util.c */
hostlist_t      *hosthead = NULL;
link_t          *linkhead = NULL;
link_t  null_link = { "", "", "", NULL };

double reportgreenlevel = 99.995;
double reportwarnlevel = 98.0;

char *hostname = "";
char *service = "";
char *ip = "";
int entrycount = 50;

char *reqenv[] = {
"BBHIST",
"BBHISTLOGS",
"BBREP",
"BBREPURL",
"BBSKIN",
"CGIBINURL",
"DOTWIDTH",
"DOTHEIGHT",
"MKBBCOLFONT",
"MKBBROWFONT",
NULL };

static void errormsg(char *msg)
{
	printf("Content-type: text/html\n\n");
	printf("<html><head><title>Invalid request</title></head>\n");
	printf("<body>%s</body></html>\n", msg);
	exit(1);
}

static void parse_query(void)
{
	char *query, *token;

	if (getenv("QUERY_STRING") == NULL) {
		errormsg("Invalid request");
		return;
	}
	else query = urldecode("QUERY_STRING");

	if (!urlvalidate(query, NULL)) {
		errormsg("Invalid request");
		return;
	}

	token = strtok(query, "&");
	while (token) {
		char *val;
		
		val = strchr(token, '='); if (val) { *val = '\0'; val++; }

		if (argnmatch(token, "HISTFILE")) {
			char *p = strrchr(val, '.');

			if (p) { *p = '\0'; service = malcop(p+1); }
			hostname = malcop(val);
			while ((p = strchr(hostname, ','))) *p = '.';
		}
		else if (argnmatch(token, "IP")) {
			ip = malcop(val);
		}
		else if (argnmatch(token, "ENTRIES")) {
			if (strcmp(val, "all") == 0) entrycount = 0;
			else entrycount = atoi(val);
			if (entrycount < 0) errormsg("Invalid parameter");
		}
		else if (argnmatch(token, "PIXELS")) {
			pixels = atoi(val);
			if (pixels > 0) usepct = 0; else usepct = 1;
		}
		else if (argnmatch(token, "ENDTIME")) {
			req_endtime = atol(val);
			if (req_endtime < 0) errormsg("Invalid parameter");
		}
		else if (argnmatch(token, "BARSUMS")) {
			barsums = atoi(val);
		}

		token = strtok(NULL, "&");
	}

	free(query);
}


int main(int argc, char *argv[])
{
	char histlogfn[MAX_PATH];
	char tailcmd[MAX_PATH];
	FILE *fd;
	time_t start1d, start1w, start4w, start1y;
	reportinfo_t repinfo1d, repinfo1w, repinfo4w, repinfo1y, dummyrep;
	replog_t *log1d, *log1w, *log4w, *log1y;
	char *p;

	envcheck(reqenv);
	parse_query();


	/* Build our own URL */
	sprintf(selfurl, "%s/bb-hist.sh?HISTFILE=%s.%s", getenv("CGIBINURL"), commafy(hostname), service);

	p = selfurl + strlen(selfurl);
	sprintf(p, "&amp;BARSUMS=%d", barsums);

	if (strlen(ip)) {
		p = selfurl + strlen(selfurl);
		sprintf(p, "&amp;IP=%s", ip);
	}

	if (entrycount) {
		p = selfurl + strlen(selfurl);
		sprintf(p, "&amp;ENTRIES=%d", entrycount);
	}
	else strcat(selfurl, "&amp;ENTRIES=ALL");

	if (usepct) {
		/* Must modify 4-week charts to be 5-weeks, or the last day is 19% of the bar */
		/*
		 * Percent-based charts look awful with 24 hours / 7 days / 28 days / 12 months as basis
		 * because these numbers dont divide into 100 neatly. So the last item becomes
		 * too large (worst with the 28-day char: 100/28 = 3, last becomes (100-27*3) = 19% wide).
		 * So adjust the periods to something that matches percent-based calculations better.
		 */
		len1d = 25; bartitle1d = "25 hour summary";
		len1w = 10; bartitle1w = "10 day summary";
		len4w = 33; bartitle4w = "33 day summary";
		len1y = 10; bartitle1y = "10 month summary";
	}

	sprintf(histlogfn, "%s/%s.%s", getenv("BBHIST"), commafy(hostname), service);
	fd = fopen(histlogfn, "r");
	if (fd == NULL) {
		errormsg("Cannot open history file");
	}

	log1d = log1w = log4w = log1y = NULL;
	if (req_endtime == 0) req_endtime = time(NULL);
	/*
	 * Calculate the beginning time of each colorbar. We go back the specified length
	 * of time, except 1 second - so days are from midnight -> 23:59:59 etc.
	 */
	start1d = calc_time(req_endtime, -len1d, ALIGN_HOUR,  END_UNCHANGED) + 1;
	start1w = calc_time(req_endtime, -len1w, ALIGN_DAY,   END_UNCHANGED) + 1;
	start4w = calc_time(req_endtime, -len4w, ALIGN_DAY,   END_UNCHANGED) + 1;
	start1y = calc_time(req_endtime, -len1y, ALIGN_MONTH, END_UNCHANGED) + 1;

	/*
	 * Collect data for the color-bars and summaries. Multiple scans over the history file,
	 * but doing it all in one go would be hideously complex.
	 */
	if (barsums & BARSUM_1D) {
		parse_historyfile(fd, &repinfo1d, NULL, NULL, start1d, req_endtime, 1, reportwarnlevel, reportgreenlevel, NULL);
		log1d = save_replogs();
	}

	if (barsums & BARSUM_1W) {
		parse_historyfile(fd, &repinfo1w, NULL, NULL, start1w, req_endtime, 1, reportwarnlevel, reportgreenlevel, NULL);
		log1w = save_replogs();
	}

	if (barsums & BARSUM_4W) {
		parse_historyfile(fd, &repinfo4w, NULL, NULL, start4w, req_endtime, 1, reportwarnlevel, reportgreenlevel, NULL);
		log4w = save_replogs();
	}

	if (barsums & BARSUM_1Y) {
		parse_historyfile(fd, &repinfo1y, NULL, NULL, start1y, req_endtime, 1, reportwarnlevel, reportgreenlevel, NULL);
		log1y = save_replogs();
	}

	if (entrycount == 0) {
		/* All entries - just rewind the history file and do all of them */
		rewind(fd);
		parse_historyfile(fd, &dummyrep, NULL, NULL, 0, time(NULL), 1, reportwarnlevel, reportgreenlevel, NULL);
		fclose(fd);
	}
	else {
		/* Last 50 entries - we cheat and use "tail" in a pipe to pick the entries */
		fclose(fd);
		sprintf(tailcmd, "tail -%d %s", entrycount, histlogfn);
		fd = popen(tailcmd, "r");
		if (fd == NULL) errormsg("Cannot run tail on the histfile");
		parse_historyfile(fd, &dummyrep, NULL, NULL, 0, time(NULL), 1, reportwarnlevel, reportgreenlevel, NULL);
		pclose(fd);
	}


	/* Now generate the webpage */
	printf("Content-Type: text/html\n\n");

	generate_history(stdout, 
			 hostname, service, ip, req_endtime, 
			 start1d, &repinfo1d, log1d, 
			 start1w, &repinfo1w, log1w, 
			 start4w, &repinfo4w, log4w, 
			 start1y, &repinfo1y, log1y, 
			 entrycount, reploghead);

	return 0;
}

