/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Sleep helper for Loongson-3A sleep mode, derived from Au1xxx.
 *
 * Copyright (C) 2011 Huacai Chen
 */

#include <asm.h>
#include <regnum.h>
#include <cpu.h>

#if 1
#define PT_R0     24
#define PT_R1     28
#define PT_R2     32
#define PT_R3     36
#define PT_R4     40
#define PT_R5     44
#define PT_R6     48
#define PT_R7     52
#define PT_R8     56
#define PT_R9     60
#define PT_R10    64
#define PT_R11    68
#define PT_R12    72
#define PT_R13    76
#define PT_R14    80
#define PT_R15    84
#define PT_R16    88
#define PT_R17    92
#define PT_R18    96
#define PT_R19    100
#define PT_R20    104
#define PT_R21    108
#define PT_R22    112
#define PT_R23    116
#define PT_R24    120
#define PT_R25    124
#define PT_R26    128
#define PT_R27    132
#define PT_R28    136
#define PT_R29    140
#define PT_R30    144
#define PT_R31    148
#define PT_SIZE   176
#else
/* MIPS pt_regs offsets. */
#define PT_R0 0 /* offsetof(struct pt_regs, regs[0])	 # */
#define PT_R1 8 /* offsetof(struct pt_regs, regs[1])	 # */
#define PT_R2 16 /* offsetof(struct pt_regs, regs[2])	 # */
#define PT_R3 24 /* offsetof(struct pt_regs, regs[3])	 # */
#define PT_R4 32 /* offsetof(struct pt_regs, regs[4])	 # */
#define PT_R5 40 /* offsetof(struct pt_regs, regs[5])	 # */
#define PT_R6 48 /* offsetof(struct pt_regs, regs[6])	 # */
#define PT_R7 56 /* offsetof(struct pt_regs, regs[7])	 # */
#define PT_R8 64 /* offsetof(struct pt_regs, regs[8])	 # */
#define PT_R9 72 /* offsetof(struct pt_regs, regs[9])	 # */
#define PT_R10 80 /* offsetof(struct pt_regs, regs[10])	 # */
#define PT_R11 88 /* offsetof(struct pt_regs, regs[11])	 # */
#define PT_R12 96 /* offsetof(struct pt_regs, regs[12])	 # */
#define PT_R13 104 /* offsetof(struct pt_regs, regs[13])	 # */
#define PT_R14 112 /* offsetof(struct pt_regs, regs[14])	 # */
#define PT_R15 120 /* offsetof(struct pt_regs, regs[15])	 # */
#define PT_R16 128 /* offsetof(struct pt_regs, regs[16])	 # */
#define PT_R17 136 /* offsetof(struct pt_regs, regs[17])	 # */
#define PT_R18 144 /* offsetof(struct pt_regs, regs[18])	 # */
#define PT_R19 152 /* offsetof(struct pt_regs, regs[19])	 # */
#define PT_R20 160 /* offsetof(struct pt_regs, regs[20])	 # */
#define PT_R21 168 /* offsetof(struct pt_regs, regs[21])	 # */
#define PT_R22 176 /* offsetof(struct pt_regs, regs[22])	 # */
#define PT_R23 184 /* offsetof(struct pt_regs, regs[23])	 # */
#define PT_R24 192 /* offsetof(struct pt_regs, regs[24])	 # */
#define PT_R25 200 /* offsetof(struct pt_regs, regs[25])	 # */
#define PT_R26 208 /* offsetof(struct pt_regs, regs[26])	 # */
#define PT_R27 216 /* offsetof(struct pt_regs, regs[27])	 # */
#define PT_R28 224 /* offsetof(struct pt_regs, regs[28])	 # */
#define PT_R29 232 /* offsetof(struct pt_regs, regs[29])	 # */
#define PT_R30 240 /* offsetof(struct pt_regs, regs[30])	 # */
#define PT_R31 248 /* offsetof(struct pt_regs, regs[31])	 # */
#define PT_LO 272 /* offsetof(struct pt_regs, lo)	 # */
#define PT_HI 264 /* offsetof(struct pt_regs, hi)	 # */
#define PT_EPC 296 /* offsetof(struct pt_regs, cp0_epc)	 # */
#define PT_BVADDR 280 /* offsetof(struct pt_regs, cp0_badvaddr)	 # */
#define PT_STATUS 256 /* offsetof(struct pt_regs, cp0_status)	 # */
#define PT_CAUSE 288 /* offsetof(struct pt_regs, cp0_cause)	 # */
#define PT_SIZE 304 /* sizeof(struct pt_regs)	 # */
#endif

//#define SUSPEND_DEBUG

#ifdef SUSPEND_DEBUG
#define	PRINT(x) \
	.rdata;98: .asciz x; .text; la a0, 98b;jal stringserial_sleep; nop
#else
#define	PRINT(x)
#endif
//use t0 ,v0
#if 1
#define SBSB(x)\
	li t0,x;\
li v0,0xbfe001e0;\
sb t0,0x0(v0);\
nop;
#else
#define SBSB(x)
#endif

.rdata
hexch:
.ascii	"0123456789abcdef"

.text
.align 2

/* use a0,a1,a2 */
LEAF(stringserial_sleep)
	.set noreorder
	move	a2, ra
#ifdef ROM_EXCEPTION
	li a1,0x3ec00000
	addu	a1, a0, a1
#else
	addu	a1, a0,zero
#endif
lbu	a0, 0(a1)
	1:
	beqz	a0, 2f
	nop
	jal	tgt_putchar
	addiu	a1, 1
	b	1b
lbu	a0, 0(a1)

	2:
	j	a2
	nop
END(stringserial_sleep)


LEAF(hexserial_sleep)
	.set noreorder
	move	a2, ra
	move	a1, a0
	li	a3, 7
	1:
	rol	a0, a1, 4
	move	a1, a0
	and	a0, 0xf
#ifdef ROM_EXCEPTION
la	v0, (hexch+0x3ec00000)
#else
	la	v0, hexch
#endif
	addu	v0, a0
	jal	tgt_putchar
lbu	a0, 0(v0)

	bnez	a3, 1b
	addu	a3, -1

	j	a2
	nop
END(hexserial_sleep)


	.text
	.set noreorder
	.set noat
	.align	5
	.set	mips64

	/* preparatory stuff */
	.macro	SETUP_SLEEP
	dsubu	sp, PT_SIZE
	sw	$1, PT_R1(sp)
	sw	$2, PT_R2(sp)
	sw	$3, PT_R3(sp)
	sw	$4, PT_R4(sp)
	sw	$5, PT_R5(sp)
	sw	$6, PT_R6(sp)
	sw	$7, PT_R7(sp)
	sw	$16, PT_R16(sp)
	sw	$17, PT_R17(sp)
	sw	$18, PT_R18(sp)
	sw	$19, PT_R19(sp)
	sw	$20, PT_R20(sp)
	sw	$21, PT_R21(sp)
	sw	$22, PT_R22(sp)
	sw	$23, PT_R23(sp)
	sw	$26, PT_R26(sp)
	sw	$27, PT_R27(sp)
	sw	$28, PT_R28(sp)
	sw	$30, PT_R30(sp)
sw	$31, PT_R31(sp)
	mfc0	k0, CP0_STATUS
sw	k0, PT_R8(sp)
	mfc0	k0, CP0_CONTEXT
sw	k0, PT_R9(sp)
	mfc0	k0, CP0_XCONTEXT
sw	k0, PT_R10(sp)
	mfc0	k0, CP0_PAGEMASK, 0  /* Loongson-3 has sub-registers */
sw	k0, PT_R11(sp)
	mfc0	k0, CP0_PAGEMASK, 1  /* Loongson-3 has sub-registers */
sw	k0, PT_R12(sp)
	mfc0	k0, CP0_CONFIG, 0
sw	k0, PT_R13(sp)

	/* Now set up the "wakeup vector" in  RTC space so the boot rom will
	 * return to this point upon wakeup.
	 * 0x40 : RA,  0x48 : SP*/

	daddi	a0, sp, 0
	li      a1, 0x48
	jal     cmos_write64
	nop
	dla	a0, wakeup_start	/* resume path */
	li      a1, 0x40
	jal     cmos_write64
	nop
#if 0
	la     a1, 0xa01ffc00
sd		a0, 0x8(a1)
	dla	a0, wakeup_start	/* resume path */
sd		a0, 0x0(a1)
#endif
	.endm

	/* sleep code for Loongson-3A memory controller type */
LEAF(loongson_suspend_lowlevel)
	SETUP_SLEEP

#ifdef SUSPEND_DEBUG
	PRINT("All Regs in stack are:\r\n")
	SBSB('\r')
	SBSB('\n')
	dli		t1, 32
	move	t5,sp

	PRINT("SP=")
	move	a0,sp
	jal     hexserial_sleep
	nop
	PRINT("\r\n")

	101:
	PRINT("0x")
ld		t6,	0x0(t5)
	dsrl    a0,t6,32
	jal		hexserial_sleep
	nop
	move    a0, t6
	jal     hexserial_sleep
	nop
	SBSB('\r')
	SBSB('\n')
	daddiu  t1, t1, -1
	daddiu  t5, t5, 0x8
	bnez    t1, 101b
	nop
#endif


#ifndef RESUME_AFTER_COPY
	//set enter s3 flag
	li v0,0xa01ffffc//The Addr Store S3 Flag
	li t0,0x12345678//S3 Flag
sw t0,0(v0)
#endif

	li t0, 0x80000000
	li t1, 512
	flushL1:
	cache 0, 0(t0)
	cache 0, 1(t0)
	cache 0, 2(t0)
	cache 0, 3(t0)
	cache 1, 0(t0)
	cache 1, 1(t0)
	cache 1, 2(t0)
cache 1, 3(t0)
	addiu t0, t0, 0x20
	bnez  t1, flushL1
	addiu t1, t1, -1

	li t0, 0x80000000
	li t1, 32768
	flushL2:
	cache 3, 0(t0)
	cache 3, 1(t0)
	cache 3, 2(t0)
cache 3, 3(t0)
	addiu t0, t0, 0x20
	bnez  t1, flushL2
	addiu t1, t1, -1


	li v0, 0xbfc00500 /* Call PMON's STR sleep routine */
	jr  v0
	nop
END(loongson_suspend_lowlevel)

	/* This is where we return upon wakeup.
	 * Reload all of the registers and return.
	 */
LEAF(wakeup_start)
	//or sp, 0xa0000000
lw	k0, PT_R8(sp)
	mtc0	k0, CP0_STATUS
	SBSB('A')
lw	k0, PT_R9(sp)
	mtc0	k0, CP0_CONTEXT
	SBSB('B')
lw	k0, PT_R10(sp)
	mtc0	k0, CP0_XCONTEXT
	SBSB('C')
lw	k0, PT_R11(sp)
	mtc0	k0, CP0_PAGEMASK, 0
	SBSB('D')
lw	k0, PT_R12(sp)
	mtc0	k0, CP0_PAGEMASK, 1
	SBSB('E')
lw	k0, PT_R13(sp)
	mtc0	k0, CP0_CONFIG, 0
	nop
	SBSB('F')

	SBSB('\r')
	SBSB('\n')
lw	$1, PT_R1(sp)
	SBSB('1')
	lw	$2, PT_R2(sp)
	lw	$3, PT_R3(sp)
	lw	$4, PT_R4(sp)
	lw	$5, PT_R5(sp)
	lw	$6, PT_R6(sp)
	lw	$7, PT_R7(sp)
	lw	$16, PT_R16(sp)
	lw	$17, PT_R17(sp)
	lw	$18, PT_R18(sp)
	lw	$19, PT_R19(sp)
	lw	$20, PT_R20(sp)
	lw	$21, PT_R21(sp)
	lw	$22, PT_R22(sp)
	lw	$23, PT_R23(sp)
	lw	$26, PT_R26(sp)
	lw	$27, PT_R27(sp)
	lw	$28, PT_R28(sp)
	lw	$30, PT_R30(sp)
lw	$31, PT_R31(sp)
	SBSB('G')
	SBSB('\r')
	SBSB('\n')
lw	$2, PT_R2(sp)//because micro SBSB use v0($2)

#ifndef RESUME_AFTER_COPY
	//clear s3 flag
	li t0,0xa01ffffc
	li t1,0x0
sw t1,0(t0)
#endif

	jr	ra
	daddiu	sp, PT_SIZE
END(wakeup_start)
	.set mips0
