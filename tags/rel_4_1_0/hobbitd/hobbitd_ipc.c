/*----------------------------------------------------------------------------*/
/* Hobbit message daemon.                                                     */
/*                                                                            */
/* This module implements the setup/teardown of the hobbitd communications    */
/* channel, using standard System V IPC mechanisms: Shared memory and         */
/* semaphores.                                                                */
/*                                                                            */
/* The concept is to use a shared memory segment for each "channel" that      */
/* hobbitd supports. This memory segment is used to pass a single hobbitd     */
/* message between the hobbit master daemon, and the hobbitd_channel workers. */
/* Two semaphores are used to synchronize between the master daemon and the   */
/* workers, i.e. the workers wait for a semaphore to go up indicating that a  */
/* new message has arrived, and the master daemon then waits for the other    */
/* semaphore to go 0 indicating that the workers have read the message. A     */
/* third semaphore is used as a simple counter to tell how many workers have  */
/* attached to a channel.                                                     */
/*                                                                            */
/* Copyright (C) 2004-2005 Henrik Storner <henrik@hswn.dk>                    */
/*                                                                            */
/* This program is released under the GNU General Public License (GPL),       */
/* version 2. See the file "COPYING" for details.                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/

static char rcsid[] = "$Id: hobbitd_ipc.c,v 1.24 2005-07-17 12:49:42 henrik Exp $";

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "libbbgen.h"

#include "hobbitd_ipc.h"

char *channelnames[C_LAST+1] = {
	"",		/* First one is index 0 - not used */
	"status", 
	"stachg",
	"page",
	"data",
	"notes",
	"enadis",
	"client",
	NULL
};

hobbitd_channel_t *setup_channel(enum msgchannels_t chnid, int role)
{
	key_t key;
	struct stat st;
	struct sembuf s;
	hobbitd_channel_t *newch;
	int flags = ((role == CHAN_MASTER) ? (IPC_CREAT | 0600) : 0);
	char *bbh = xgetenv("BBHOME");

	if ( (bbh == NULL) || (stat(bbh, &st) == -1) ) {
		errprintf("BBHOME not defined, or points to invalid directory - cannot continue.\n");
		return NULL;
	}

	dprintf("Setting up %s channel (id=%d)\n", channelnames[chnid], chnid);

	dprintf("calling ftok('%s',%d)\n", bbh, chnid);
	key = ftok(bbh, chnid);
	if (key == -1) {
		errprintf("Could not generate shmem key based on %s: %s\n", bbh, strerror(errno));
		return NULL;
	}
	dprintf("ftok() returns: 0x%X\n", key);

	newch = (hobbitd_channel_t *)malloc(sizeof(hobbitd_channel_t));
	newch->seq = 0;
	newch->channelid = chnid;
	newch->msgcount = 0;
	newch->shmid = shmget(key, SHAREDBUFSZ, flags);
	if (newch->shmid == -1) {
		errprintf("Could not get shm of size %d: %s\n", SHAREDBUFSZ, strerror(errno));
		xfree(newch);
		return NULL;
	}
	dprintf("shmget() returns: 0x%X\n", newch->shmid);

	newch->channelbuf = (char *) shmat(newch->shmid, NULL, 0);
	if (newch->channelbuf == (char *)-1) {
		errprintf("Could not attach shm %s\n", strerror(errno));
		if (role == CHAN_MASTER) shmctl(newch->shmid, IPC_RMID, NULL);
		xfree(newch);
		return NULL;
	}

	newch->semid = semget(key, 3, flags);
	if (newch->semid == -1) {
		errprintf("Could not get sem: %s\n", strerror(errno));
		shmdt(newch->channelbuf);
		if (role == CHAN_MASTER) shmctl(newch->shmid, IPC_RMID, NULL);
		xfree(newch);
		return NULL;
	}

	if (role == CHAN_CLIENT) {
		/*
		 * Clients must register their presence.
		 * We use SEM_UNDO; so if the client crashes, it wont leave a stale count.
		 */
		s.sem_num = CLIENTCOUNT; s.sem_op = +1; s.sem_flg = SEM_UNDO;
		if (semop(newch->semid, &s, 1) == -1) {
			errprintf("Could not register presence: %s\n", strerror(errno));
			shmdt(newch->channelbuf);
			xfree(newch);
			return NULL;
		}
	}
	else if (role == CHAN_MASTER) {
		int n;

		n = semctl(newch->semid, CLIENTCOUNT, GETVAL);
		if (n > 0) {
			errprintf("FATAL: hobbitd sees clientcount %d, should be 0\nCheck for hanging hobbitd_channel processes or stale semaphores\n", n);
			shmdt(newch->channelbuf);
			shmctl(newch->shmid, IPC_RMID, NULL);
			semctl(newch->semid, 0, IPC_RMID);
			xfree(newch);
			return NULL;
		}
	}

#ifdef MEMORY_DEBUG
	add_to_memlist(newch->channelbuf, SHAREDBUFSZ);
#endif
	return newch;
}

void close_channel(hobbitd_channel_t *chn, int role)
{
	if (chn == NULL) return;

	/* No need to de-register, this happens automatically because we registered with SEM_UNDO */

	if (role == CHAN_MASTER) semctl(chn->semid, 0, IPC_RMID);

	MEMUNDEFINE(chn->channelbuf);
	shmdt(chn->channelbuf);
	if (role == CHAN_MASTER) shmctl(chn->shmid, IPC_RMID, NULL);
}

