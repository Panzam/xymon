#!/bin/sh
#----------------------------------------------------------------------------#
# AIX client for Hobbit                                                      #
#                                                                            #
# Copyright (C) 2005-2006 Henrik Storner <henrik@hswn.dk>                    #
#                                                                            #
# This program is released under the GNU General Public License (GPL),       #
# version 2. See the file "COPYING" for details.                             #
#                                                                            #
#----------------------------------------------------------------------------#
#
# $Id: hobbitclient-aix.sh,v 1.15 2006-09-27 08:12:33 henrik Exp $

echo "[date]"
date
echo "[uname]"
uname -a
echo "[uptime]"
uptime
echo "[who]"
who
echo "[df]"
# The sed stuff is to make sure lines are not split into two.
df -Ik | sed -e '/^[^ 	][^ 	]*$/{
N
s/[ 	]*\n[ 	]*/ /
}'
echo "[mount]"
mount
echo "[realmem]"
lsattr -El sys0 -a realmem
echo "[freemem]"
vmstat 1 2 | tail -1
echo "[swap]"
lsps -s
echo "[ifconfig]"
ifconfig -a
echo "[route]"
netstat -rn
echo "[netstat]"
netstat -s
echo "[ports]"
netstat -an | grep "^tcp"
echo "[ifstat]"
netstat -v
echo "[ps]"
# I think the -f and -l options are ignored with -o, but this works...
ps -A -k -f -l -o pid,ppid,user,stat,pri,pcpu,time,etime,pmem,vsz,args

# $TOP must be set, the install utility should do that for us if it exists.
if test "$TOP" != ""
then
    if test -x "$TOP"
    then
        echo "[top]"
        $TOP -b 20
    fi
fi

# vmstat
nohup sh -c "vmstat 300 2 1>$BBTMP/hobbit_vmstat.$MACHINEDOTS.$$ 2>&1; mv $BBTMP/hobbit_vmstat.$MACHINEDOTS.$$ $BBTMP/hobbit_vmstat.$MACHINEDOTS" </dev/null >/dev/null 2>&1 &
sleep 5
if test -f $BBTMP/hobbit_vmstat.$MACHINEDOTS; then echo "[vmstat]"; cat $BBTMP/hobbit_vmstat.$MACHINEDOTS; rm -f $BBTMP/hobbit_vmstat.$MACHINEDOTS; fi

exit

