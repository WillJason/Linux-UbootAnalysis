/*
 *  crt0 - C-runtime startup Code for ARM U-Boot
 *
 *  Copyright (c) 2012  Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <asm-offsets.h>
#include <linux/linkage.h>

/*
 * This file handles the target-independent stages of the U-Boot
 * start-up where a C runtime environment is needed. Its entry point
 * is _main and is branched into from the target's start.S file.
 *
 * _main execution sequence is:
 *
 * 1. Set up initial environment for calling board_init_f().
 *    This environment only provides a stack and a place to store
 *    the GD ('global data') structure, both located in some readily
 *    available RAM (SRAM, locked cache...). In this context, VARIABLE
 *    global data, initialized or not (BSS), are UNAVAILABLE; only
 *    CONSTANT initialized data are available.
 *
 * 2. Call board_init_f(). This function prepares the hardware for
 *    execution from system RAM (DRAM, DDR...) As system RAM may not
 *    be available yet, , board_init_f() must use the current GD to
 *    store any data which must be passed on to later stages. These
 *    data include the relocation destination, the future stack, and
 *    the future GD location.
 *
 * (the following applies only to non-SPL builds)
 *
 * 3. Set up intermediate environment where the stack and GD are the
 *    ones allocated by board_init_f() in system RAM, but BSS and
 *    initialized non-const data are still not available.
 *
 * 4. Call relocate_code(). This function relocates U-Boot from its
 *    current location into the relocation destination computed by
 *    board_init_f().
 *
 * 5. Set up final environment for calling board_init_r(). This
 *    environment has BSS (initialized to 0), initialized non-const
 *    data (initialized to their intended value), and stack in system
 *    RAM. GD has retained values set by board_init_f(). Some CPUs
 *    have some work left to do at this point regarding memory, so
 *    call c_runtime_cpu_setup.
 *
 * 6. Branch to board_init_r().
 */

/*
 * entry point of crt0 sequence
 */

ENTRY(_main)

/*
 * Set up initial C runtime environment and call board_init_f(0).
 */

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)
	ldr	sp, =(CONFIG_SPL_STACK)
#else
/*首先将CONFIG_SYS_INIT_SP_ADDR定义的值加载到栈指针sp中，这个宏定义在配置头文件itop-4412.h中指定。*/
	ldr	sp, =(CONFIG_SYS_INIT_SP_ADDR)
#endif
/*接下来为board_init_f C函数调用提供环境，也就是栈指针sp初始化*/
	bic	sp, sp, #7	/* 8-byte alignment for ABI compliance */
	mov	r2, sp
	sub	sp, sp, #GD_SIZE	/* allocate one GD above SP */
	bic	sp, sp, #7	/* 8-byte alignment for ABI compliance */
	mov	r9, sp		/* GD is above SP */
	mov	r1, sp
	mov	r0, #0
/*最后将sp保存在r9寄存器中。因此r9寄存器中的地址就是gd结构体的首地址*/
	/*清除GD空间，以待重新赋值*/
clr_gd:
	cmp	r1, r2			/* while not at end of GD */
	strlo	r0, [r1]		/* clear 32-bit GD word */
	addlo	r1, r1, #4		/* move to next */
	blo	clr_gd
#if defined(CONFIG_SYS_MALLOC_F_LEN)/*这个未定义，跳过*/
	sub	sp, sp, #CONFIG_SYS_MALLOC_F_LEN
	str	sp, [r9, #GD_MALLOC_BASE]
#endif
	/* mov r0, #0 not needed due to above code */
	bl led_on
	bl board_init_f
	/*看打印信息发现这里貌似调用了Spl_boot.c (arch\arm\cpu\armv7\exynos):和Board_f.c (common)这两
	个文件，其中一个是weak定义*/

#if ! defined(CONFIG_SPL_BUILD)

/*
 * Set up intermediate environment (new sp and gd) and call
 * relocate_code(addr_moni). Trick here is that we'll return
 * 'here' but relocated.
 */
	/*前4条汇编实现了新gd结构体的更新。
	首先更新sp，并且将sp 8字节对齐，方便后面函数开辟栈能对齐，

	然后获取gd->bd地址到r9中，需要注意，在board_init_f中gd->bd已经
	更新为新分配的bd了，下一条汇编将r9减掉bd的size，这样就获取到了board_init_f中新分配的gd了！

	后面汇编则是为relocate_code做准备，首先加载here地址，然后加上新地址
	偏移量给lr，则是code relocate后的新here了，relocate_code返回条转到lr，则是新位置的here！

	最后在r0中保存code的新地址，跳转到relocate_code*/
	ldr	sp, [r9, #GD_START_ADDR_SP]	/* sp = gd->start_addr_sp */
	bic	sp, sp, #7	/* 8-byte alignment for ABI compliance */
	ldr	r9, [r9, #GD_BD]		/* r9 = gd->bd */
	sub	r9, r9, #GD_SIZE		/* new GD is below bd */

	adr	lr, here
	ldr	r0, [r9, #GD_RELOC_OFF]		/* r0 = gd->reloc_off */
	add	lr, lr, r0
	ldr	r0, [r9, #GD_RELOCADDR]		/* r0 = gd->relocaddr */
	/*将uboot code拷贝到relocaddr*/
	b	relocate_code
	//可以参考这位大牛的博客http://blog.csdn.net/skyflying2012/article/details/37660265
here:
/*
 * now relocate vectors
 */

	bl	relocate_vectors

/* Set up final (full) environment */

	bl	c_runtime_cpu_setup	/* we still call old routine here */
#endif
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_FRAMEWORK)
# ifdef CONFIG_SPL_BUILD
	/* Use a DRAM stack for the rest of SPL, if requested */
	bl	spl_relocate_stack_gd
	cmp	r0, #0
	movne	sp, r0
# endif
	ldr	r0, =__bss_start	/* this is auto-relocated! */

#ifdef CONFIG_USE_ARCH_MEMSET
	ldr	r3, =__bss_end		/* this is auto-relocated! */
	mov	r1, #0x00000000		/* prepare zero to clear BSS */

	subs	r2, r3, r0		/* r2 = memset len */
	bl	memset
#else
	ldr	r1, =__bss_end		/* this is auto-relocated! */
	mov	r2, #0x00000000		/* prepare zero to clear BSS */

clbss_l:cmp	r0, r1			/* while not at end of BSS */
	strlo	r2, [r0]		/* clear 32-bit BSS word */
	addlo	r0, r0, #4		/* move to next */
	blo	clbss_l
#endif

#if ! defined(CONFIG_SPL_BUILD)
	bl coloured_LED_init
	bl red_led_on
#endif
	/* call board_init_r(gd_t *id, ulong dest_addr) */
	mov     r0, r9                  /* gd_t */
	ldr	r1, [r9, #GD_RELOCADDR]	/* dest_addr */
	/* call board_init_r */
	/*貌似调用Spl_boot.c (arch\arm\cpu\armv7\exynos)和 Board_r.c (common)这两个文件，其中一个也是weak定义*/
	ldr	pc, =board_init_r	/* this is auto-relocated! */

	/*如果icache是enable，则无效掉icache，保证从sdram中更新指令到cache中。
	接着更新异常向量表首地址，因为code被relocate，所以异常向量表也被relocate。

	从c_runtime_cpu_setup返回，下面一段汇编是将bss段清空。

	接下来分别调用了coloured_LED_init以及red_led_on,很多开发板都会有led指示灯，
	这里可以实现上电指示灯亮，有调试作用。

	最后r0赋值gd指针，r1赋值relocaddr，进入最后的board_init_r !*/
	/* we should not return here. */
#endif

ENDPROC(_main)


/*参考资料：https://blog.csdn.net/dddd0216/article/details/51142333*/