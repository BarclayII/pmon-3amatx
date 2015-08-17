/*                               -*- Mode: C -*- 
 * 
 * File            : bootkernel.c
 * Program/Library : RAYS Packages Manager
 * Description     : 
 * Created: Thu Apr  3 15:48:18 2008
 * Author: WeiPing Zhu
 * Mail            : weiping.zhu@sw-linux.com
 * Last Modified By : WeiPing Zhu
 * Last Modified On : Mon Apr 14 15:04:01 2008 (28800 CST)
 * Update Count : 51
 * 
 *    Copyright 2007 Sun Wah Linux Limited
 *    addr: New World Center, No.88 Zhujiang Road Nanjing, Jiangsu Province, 210018, China
 *    tel : 
 *    fax : 
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/endian.h>

#include <pmon.h>
#include <exec.h>
#include <pmon/loaders/loadfn.h>

#ifdef __mips__
#include <machine/cpu.h>
#endif

#include "elf.h"

extern int errno;                       /* global error number */
extern char *heaptop;

typedef void (*entry_t)();

int findsect(int fd, unsigned int sector)
{
	unsigned int seeklimit = 0x40000000;
	unsigned int sectorlimit = seeklimit / 512;

	if (lseek(fd, 0, SEEK_SET) == -1)
		return -1;
	for (; sector > sectorlimit; sector -= sectorlimit) {
		if (lseek(fd, seeklimit, SEEK_CUR) == -1)
			return -1;
	}
	return lseek(fd, sector * 512, SEEK_CUR) == -1 ? -1 : 0;
}

void boot_custom_kernel(int fd, unsigned char *mbr)
{
	if (fd == -1) {
		printf("Open failed\n");
		return;
	}
	entry_t entry;
	int i, phnum;

	unsigned int sector_start;
	unsigned int pos = 0;

	unsigned char *buf = (unsigned char *)0x80180000;

	unsigned char *part1 = mbr + 0x1ce;
	printf("part1: %08x\n", part1);
	printf("mbr: %08x\n", mbr);
	for (i = 0; i < 512; ++i) {
		printf("%02x%c", mbr[i] & 0xff, (i + 1) % 16 ? ' ' : '\n');
	}

	printf("reading %08x\n", part1 + 0x8);
	sector_start = *(unsigned short *)(part1 + 0xa);
	sector_start = (sector_start << 16) + *(unsigned short *)(part1 + 0x8);
	printf("sector start: %x\n", sector_start);

	if (findsect(fd, sector_start) == -1) {
		printf("find sector failed\n");
		return;
	}

	if (read(fd, buf, sizeof(struct elf64hdr)) == -1) {
		printf("read failed\n");
		return;
	}
	if (*(unsigned int *)buf != 0x464c457f) {
		printf("ELF MAGIC DOES NOT MATCH\n");
		return;
	}
	pos += sizeof(struct elf64hdr);

	struct elf64hdr *eh = (struct elf64hdr *)buf;

	phnum = eh->e_phnum;
	entry = (entry_t)(eh->e_entry);
	printf("# of program headers: %d\n", phnum);
	printf("entry address: %x\n", (unsigned int)entry);

	for (i = 0; i < phnum; ++i) {
		read(fd, buf, sizeof(struct elf64_phdr));
		pos += sizeof(struct elf64_phdr);
		printf("position: %x\n", pos);
		struct elf64_phdr *ph = (struct elf64_phdr *)buf;
		if (ph->p_type == PT_LOAD) {
			printf("Loadable segment %d\n", i);
			printf("offset: %x\n", (int)(ph->p_offset));
			printf("address: %x\n", (unsigned int)(ph->p_vaddr));
			printf("file size: %x\n", (unsigned int)(ph->p_filesz));
			int off = (unsigned int)(ph->p_offset) - pos;
			if (lseek(fd, off, SEEK_CUR) == -1) {
				printf("lseek failed\n");
				return;
			}
			if (read(fd,
			    (unsigned char *)(ph->p_vaddr),
			    ph->p_filesz) == -1) {
				printf("read failed\n");
				return;
			}
			if (lseek(fd, -off, SEEK_CUR) == -1) {
				printf("lseek failed\n");
				return;
			}
		}
	}

	(*entry)();
}

unsigned char mbr[512];

int boot_kernel(const char* path, int flags, void* flashaddr, unsigned int offset)
{
	int	bootfd;
	int	bootbigend;

	char buf[DLREC+1];
	long ep;
	int n;
#ifdef HAVE_FLASH
	size_t	    flashsize;
#endif

	if ((bootfd = open (path, O_RDONLY | O_NONBLOCK)) < 0) {
		perror (path);
		return -1;
	}

#ifdef HAVE_FLASH
	if (flags & FFLAG) {
		tgt_flashinfo (flashaddr, &flashsize);
		if (flashsize == 0) {
			printf ("No FLASH at given address\n");
			return 0;
		}
		/* any loaded program will be trashed... */
		flags &= ~(SFLAG | BFLAG | EFLAG);
		flags |= NFLAG;		/* don't bother with symbols */
		/*
		 * Recalculate any offset given on command line.
		 * Addresses should be 0 based, so a given offset should be
		 * the actual load address of the file.
		 */
		offset = (unsigned long)heaptop - offset;
#if BYTE_ORDER == LITTLE_ENDIAN
		bootbigend = 0;
#else
		bootbigend = 1;
#endif
	}
#endif
	int i;
	int fd = open("/dev/disk/wd0", 0);
	entry_t entry;
	if (fd == -1) {
		printf("Error while opening /dev/disk/wd0");
		goto loongson_original;
	}

	if (read(fd, mbr, 512) == -1) {
		printf("Error while reading /dev/disk/wd0");
		close(fd);
		goto loongson_original;
	}

	for (i = 0; i < 512; ++i) {
		printf("%02x%c", mbr[i] & 0xff, (i + 1) % 16 ? ' ' : '\n');
	}
	close(fd);

	printf("open: %08x\n", open);
	printf("close: %08x\n", close);
	printf("read: %08x\n", read);
	printf("puts: %08x\n", puts);
	unsigned char *pos = (unsigned char *)0x80100000;
	memcpy(pos, mbr, 512);
	fd = open("/dev/disk/wd0", 0);
#if 1
	entry = (entry_t)pos;
	entry(fd, findsect, read, lseek);
#else
	boot_custom_kernel(fd, pos);
#endif
loongson_original:
	dl_initialise (offset, flags);

	fprintf (stderr, "Loading file: %s ", path);
	errno = 0;
	n = 0;

	if (flags & RFLAG) {
	   ExecId id;

	   id = getExec("bin");
	   if (id != NULL) {
		   ep = exec (id, bootfd, buf, &n, flags);
	   }
	} else {
		ep = exec (NULL, bootfd, buf, &n, flags);
	}

	close (bootfd);
	putc ('\n', stderr);

	if (ep == -1) {
		fprintf (stderr, "%s: boot failed\n", path);
		return -3;
	}

	if (ep == -2) {
		fprintf (stderr, "%s: invalid file format\n", path);
		return -4;
	}

	if (!(flags & (FFLAG|YFLAG))) {
		printf ("Entry address is %08x\n", ep);
		/* Flush caches if they are enabled */
		if (md_cachestat())
			flush_cache (DCACHE | ICACHE, NULL);
		md_setpc(NULL, ep);
		if (!(flags & SFLAG)) {
		    dl_setloadsyms ();
		}
	}
#ifdef HAVE_FLASH
	if (flags & FFLAG) {
		extern long dl_minaddr;
		extern long dl_maxaddr;
		if (flags & WFLAG)
			bootbigend = !bootbigend;
		tgt_flashprogram ((void *)flashaddr, 	   	/* address */
				dl_maxaddr - dl_minaddr, 	/* size */
				(void *)heaptop,		/* load */
				bootbigend);
	}
#endif
   	return 0;
}

static	unsigned int rd_start;
static	unsigned int rd_size;
static	int execed;

int boot_initrd(const char* path, int rdstart,int flags)
{
	char buf[DLREC+1] = {0};
	int bootfd;
	int n = 0;
	ExecId id;
	
	rd_start = rdstart;
	rd_size = 0;
	
	printf("Loading initrd image %s", path);
	
	if ((bootfd = open (path, O_RDONLY | O_NONBLOCK)) < 0) {
		perror (path);
		return -1;
	}

	dl_initialise (rd_start, flags);
	
	id = getExec("bin");
	if (id != NULL) {
		exec (id, bootfd, buf, &n, flags);
		rd_size = (dl_maxaddr - dl_minaddr);
		execed = 1;
	}else{
		printf("[error] this pmon can't load bin file!");
		return -2;
	}
	close(bootfd);
#ifdef INITRD_DEBUG
	printf("rd_start %x, rd_size %x\n", rd_start, rd_size);
#endif
	return 0;
}

int initrd_execed(void)
{
	return execed;
}

unsigned int get_initrd_start(void)
{
	return 	rd_start;
}

unsigned int get_initrd_size(void)
{
	return rd_size;
}

