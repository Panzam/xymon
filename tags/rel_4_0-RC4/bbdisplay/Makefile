# Makefile for bbgen - bbdisplay programs
#


PROGRAMS = bbgen bb-rep.cgi bb-replog.cgi bb-hist.cgi bb-snapshot.cgi bb-eventlog.cgi bb-findhost.cgi bb-csvinfo.cgi bb-hostsvc.cgi bb-ack.cgi bb-webpage bb-larrdcolumn bb-infocolumn

EXTENSIONS = bbcombotest

CGISCRIPTS = bb-rep.sh bb-replog.sh bb-hist.sh bb-snapshot.sh bb-eventlog.sh bb-findhost.sh bb-csvinfo.sh bbmessage.cgi bb-hostsvc.sh bb-histlog.sh hobbitcolumn.sh

TEMPLATES = acknowledge_header acknowledge_footer acknowledge_form findhost_header findhost_footer findhost_form report_header report_footer report_form snapshot_header snapshot_footer snapshot_form event_header event_footer event_form

GENOBJS = bbgen.o loadbbhosts.o loaddata.o reportdata.o bbconvert.o pagegen.o eventlog.o acklog.o process.o wmlgen.o rssgen.o bb-replog.o util.o debug.o
COMBOTESTOBJS = bbcombotest.o
REPOBJS = bb-rep.o
REPLOGOBJS = bb-replog-cgi.o reportdata.o
HISTOBJS = bb-hist.o reportdata.o
SNAPOBJS = bb-snapshot.o
EVENTLOGOBJS = bb-eventlog.o util.o loadbbhosts.o		# util.o for histlogurl, find_or_create_column
FINDHOSTOBJS = bb-findhost.o
CSVINFOOBJS = bb-csvinfo.o
MESSAGECGIOBJS = sendmsg-cgi.o
HOSTSVCCGIOBJS = bb-hostsvc.o
ACKCGIOBJS = bb-ack.o
WEBPAGEOBJS = bb-webpage.o
LARRDCOLUMNOBJS = bb-larrdcolumn.o
INFOCOLUMNOBJS = bb-infocolumn.o
ifdef HOBBITD
INFOCOLUMNOBJS += hobbit-alertconfig.o
endif

HTMLPAGES = bb-findhost.html
GIFFILES = bbfindhost1.gif bbfindhost2.gif


all: $(PROGRAMS) $(EXTENSIONS) $(CGISCRIPTS)


bbgen: $(GENOBJS) ../lib/libbbgen.a
	$(CC) $(CFLAGS) -o $@ $(GENOBJS) ../lib/libbbgen.a $(NETLIBS)

bbcombotest: $(COMBOTESTOBJS) ../lib/libbbgen.a
	$(CC) $(CFLAGS) -o $@ $(COMBOTESTOBJS) ../lib/libbbgen.a $(NETLIBS)

bb-rep.cgi: $(REPOBJS) ../lib/libbbgen.a
	$(CC) $(CFLAGS) -o $@ $(REPOBJS) ../lib/libbbgen.a $(NETLIBS)

bb-replog.cgi: $(REPLOGOBJS) ../lib/libbbgen.a
	$(CC) $(CFLAGS) -o $@ $(REPLOGOBJS) ../lib/libbbgen.a $(NETLIBS)

bb-hist.cgi: $(HISTOBJS) ../lib/libbbgen.a
	$(CC) $(CFLAGS) -o $@ $(HISTOBJS) ../lib/libbbgen.a $(NETLIBS)

bb-snapshot.cgi: $(SNAPOBJS) ../lib/libbbgen.a
	$(CC) $(CFLAGS) -o $@ $(SNAPOBJS) ../lib/libbbgen.a $(NETLIBS)

bb-eventlog.cgi: $(EVENTLOGOBJS) ../lib/libbbgen.a
	$(CC) $(CFLAGS) -o $@ $(EVENTLOGOBJS) ../lib/libbbgen.a $(NETLIBS)

bb-findhost.cgi: $(FINDHOSTOBJS) ../lib/libbbgen.a
	$(CC) $(CFLAGS) -o $@ $(FINDHOSTOBJS) ../lib/libbbgen.a $(NETLIBS)

bb-csvinfo.cgi: $(CSVINFOOBJS) ../lib/libbbgen.a
	$(CC) $(CFLAGS) -o $@ $(CSVINFOOBJS) ../lib/libbbgen.a $(NETLIBS)

bbmessage.cgi: $(MESSAGECGIOBJS) ../lib/libbbgen.a
	$(CC) $(CFLAGS) -o $@ $(MESSAGECGIOBJS) ../lib/libbbgen.a $(NETLIBS)

bb-hostsvc.cgi: $(HOSTSVCCGIOBJS) ../lib/libbbgen.a
	$(CC) $(CFLAGS) -o $@ $(HOSTSVCCGIOBJS) ../lib/libbbgen.a $(NETLIBS)

bb-ack.cgi: $(ACKCGIOBJS) ../lib/libbbgen.a
	$(CC) $(CFLAGS) -o $@ $(ACKCGIOBJS) ../lib/libbbgen.a $(NETLIBS)

bb-webpage: $(WEBPAGEOBJS) ../lib/libbbgen.a
	$(CC) $(CFLAGS) -o $@ $(WEBPAGEOBJS) ../lib/libbbgen.a

bb-larrdcolumn: $(LARRDCOLUMNOBJS) ../lib/libbbgen.a
	$(CC) $(CFLAGS) -o $@ $(LARRDCOLUMNOBJS) ../lib/libbbgen.a $(NETLIBS)

bb-infocolumn: $(INFOCOLUMNOBJS) ../lib/libbbgen.a
ifdef HOBBITD
	$(CC) $(CFLAGS) -o $@ $(RPATHOPT) $(INFOCOLUMNOBJS) ../lib/libbbgen.a $(PCRELIBS) $(NETLIBS)
else
	$(CC) $(CFLAGS) -o $@ $(INFOCOLUMNOBJS) ../lib/libbbgen.a $(NETLIBS)
endif


################################################
# Objects requiring special compile flags 
################################################
digest.o: digest.c
	$(CC) $(CFLAGS) $(SSLFLAGS) $(SSLINCDIR) -c -o $@ digest.c

bb-hist.o: bb-hist.c
	$(CC) $(CFLAGS) $(HISTGRAPHDEF) -c -o $@ bb-hist.c


################################################
# Objects compiled differently for use in 
# bbgen/bbtest-net, and in standalone programs
################################################
bb-eventlog.o: eventlog.o
	$(CC) $(CFLAGS) -DCGI -c -o $@ eventlog.c

bb-replog-cgi.o: bb-replog.c
	$(CC) $(CFLAGS) -DCGI -c -o $@ bb-replog.c

sendmsg-cgi.o: ../lib/sendmsg.c
	$(CC) $(CFLAGS) -DCGI -c -o $@ ../lib/sendmsg.c

bb-hostsvc.o: bb-hostsvc.c
	$(CC) $(CFLAGS) -DCGI -c -o $@ bb-hostsvc.c

ifdef HOBBITD
bb-infocolumn.o: bb-infocolumn.c
	$(CC) $(CFLAGS) -DHOBBIT -I../hobbitd -c -o $@ bb-infocolumn.c
else
bb-infocolumn.o: bb-infocolumn.c
	$(CC) $(CFLAGS) -c -o $@ bb-infocolumn.c
endif

hobbit-alertconfig.o: ../hobbitd/do_alert.c
	$(CC) $(CFLAGS) -DINFOCOL -c -o $@ $(PCREINCDIR) -I../hobbitd ../hobbitd/do_alert.c

################################################
# Default compilation rules
################################################
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

ifdef HOBBITD
%.sh: %.sh-hobbitd.DIST
	cat $< | sed -e 's!@BBHOME@!$(BBHOME)!g' | sed -e 's!@RUNTIMEDEFS@!$(RUNTIMEDEFS)!g' >$@
	chmod 755 $@
else
%.sh: %.sh-bbd.DIST
	cat $< | sed -e 's!@BBHOME@!$(BBHOME)!g' | sed -e 's!@RUNTIMEDEFS@!$(RUNTIMEDEFS)!g' >$@
	chmod 755 $@
endif

clean:
	rm -f *.o *.a *~ $(PROGRAMS) $(EXTENSIONS) $(CGISCRIPTS)


install: install-nocgi install-cgi

ifdef HOBBITD
install-nocgi: install-bin install-exts install-man 
else
install-nocgi: install-bin install-exts install-man install-html install-gif install-templates
endif

install-bin: $(PROGRAMS)
	cp -fp $(PROGRAMS) $(BBHOME)/bin/

install-exts: $(EXTENSIONS)
	cp -fp $(EXTENSIONS) $(BBHOME)/ext/

install-cgi: $(CGISCRIPTS)
ifndef HOBBITD
	@echo ""
	@echo "  WARNING : About to overwrite any existing Big Brother CGI scripts"
	@echo "            Hit ctrl-C NOW if you want to save them first!"
	@echo ""
	@sleep 5
endif
	mkdir -p $(CGIDIR)
	cp -fp $(CGISCRIPTS) $(CGIDIR)/

install-html: $(HTMLPAGES)
	cp -fp $(HTMLPAGES) $(BBHOME)/www/help/

install-gif: $(GIFFILES)
	BBHOME=$(BBHOME) ../build/install-nooverwrite.sh www/gifs $(GIFFILES)

install-templates: $(TEMPLATES)
	BBHOME=$(BBHOME) ../build/install-nooverwrite.sh web $(TEMPLATES)

install-man:
	mkdir -p $(MANROOT)/man1 $(MANROOT)/man5
	cp -fp *.1 $(MANROOT)/man1/
	cp -fp *.5 $(MANROOT)/man5/

