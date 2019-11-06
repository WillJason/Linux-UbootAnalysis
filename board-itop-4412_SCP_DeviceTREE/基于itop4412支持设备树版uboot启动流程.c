/*
先来说一下这个uboot的配置过程：
	这个版本的uboot由两部分配置组成，一个是uboot/configs/itop-4412_deconfig,另一个是uboot/included/configs/Itop-4412.h  **
这两个文件有什么不同？最大的不同就是"configs/boardname_defconfig"中的选项都可以在make menuconfig中进
行配置,而"include/configs/boardname.h"中的选项是与开发板相关的一些特性,在make menuconfig中是找不到
这些选项的。个人理解uboot/configs/itop-4412_deconfig也就是menuconfig中设置的CONFIG是跟各目录下的Kconfig和
Makefile以及uboot源代码中都相关的都可以使能的，而include/configs/boardname.h中设置的CONFIG仅仅是和uboot
源代码相关的。（之前S3c_2440的uboot配置是一起的，这个版本是分开的。）
从configs/itop-4412_deconfig开始看：
CONFIG_SPL=y
CONFIG_ARM=y
CONFIG_ARCH_EXYNOS=y
	In--uboot/arch/arm/cpu/armv7/exynos/Kconfig:
		if ARCH_EXYNOS........endif    定义后可以有效这个Kconfig的代码
	In--uboot/arch/arm/Kconfig:
		config ARCH_EXYNOS		使能此配置
		bool "Samsung EXYNOS"
		select CPU_V7   选中定义CPU_V7，后面有用
CONFIG_TARGET_ITOP-4412=y
	In--uboot/board/samsung/itop-4412/Kconfig:
		if TARGET_ITOP-4412.......endif   定义后可以有效这个Kconfig的代码
	In--uboot/arch/arm/cpu/armv7/exynos/Kconfig:
		config TARGET_ITOP-4412		使能此配置
		bool "Exynos4412 Ultimate board"
		select SUPPORT_SPL    选中定义SUPPORT_SPL
CONFIG_OF_CONTROL=y
	给uboot源代码用设备树相关的有效，未具体分析
CONFIG_DEFAULT_DEVICE_TREE="exynos4412-itop-4412"
	定义使用哪一个uboot设备树文件
CONFIG_OF_EMBED=y
	给uboot源代码和顶层Makefile的设备树相关，未具体分析
CONFIG_SYS_PROMT="ITOP-4412"
CONFIG_DEBUG_LL=y
CONFIG_DEBUG_UART_PHYS=0x13820000
CONFIG_DEBUG_UART_8250_FLOW_CONTROL=y

CONFIG_DM=y
CONFIG_DM_SERIAL=y
CONFIG_DM_GPIO=y
--------------------------------------------------------------------------------------
其他Kconfig中的config配置
--------------------------------------------------------------------------------------
In--uboot/arch/arm/Kconfig:
config SYS_ARCH
	default "arm"  默认为固定值，文件uboot/config.mk定义了ARCH := $(CONFIG_SYS_ARCH:"%"=%)，也就是ARCH="arm"
config SYS_CPU
        default "arm720t" if CPU_ARM720T
        default "arm920t" if CPU_ARM920T
        default "arm926ejs" if CPU_ARM926EJS
        default "arm946es" if CPU_ARM946ES
        default "arm1136" if CPU_ARM1136
        default "arm1176" if CPU_ARM1176
        default "armv7" if CPU_V7		之前使能了CPU_V7所以SYS_CPU为"armv7"
        default "pxa" if CPU_PXA
        default "sa1100" if CPU_SA1100
	default "armv8" if ARM64
In--uboot/arch/arm/cpu/armv7/exynos/Kconfig:
config SYS_SOC
	default "exynos"	默认为固定值，文件uboot/config.mk定义了SOC := $(CONFIG_SYS_SOC:"%"=%)，也就是SOC="exynos"

config DM
	default y

config DM_SERIAL
	default n

config DM_SPI
	default n

config DM_SPI_FLASH
	default n

config DM_GPIO
	default n	
In--uboot/board/samsung/itop-4412/Kconfig:
if TARGET_ITOP-4412   前面已经使能了所以有效

config SYS_BOARD
	default "itop-4412"  默认为固定值，文件uboot/config.mk定义了BOARD := $(CONFIG_SYS_BOARD:"%"=%)，也就是BOARD="itop-4412"

config SYS_VENDOR
	default "samsung"	默认为固定值，文件uboot/config.mk定义了VENDOR := $(CONFIG_SYS_VENDOR:"%"=%)，也就是VENDOR="samsung"

config SYS_CONFIG_NAME
	default "itop-4412"

endif
--------------------------------------------------------------------------------
查看顶层Makefile中定义的定义的链接脚本的位置在哪：
u-boot.lds: $(LDSCRIPT) prepare FORCE
	$(call if_changed_dep,cpp_lds)
	
	
# If board code explicitly specified LDSCRIPT or CONFIG_SYS_LDSCRIPT, use
# that (or fail if absent).  Otherwise, search for a linker script in a
# standard location.

ifndef LDSCRIPT
	#LDSCRIPT := $(srctree)/board/$(BOARDDIR)/u-boot.lds.debug
	ifdef CONFIG_SYS_LDSCRIPT				#未定义，跳过
		# need to strip off double quotes
		LDSCRIPT := $(srctree)/$(CONFIG_SYS_LDSCRIPT:"%"=%)
	endif
endif

# If there is no specified link script, we look in a number of places for it
ifndef LDSCRIPT
	ifeq ($(wildcard $(LDSCRIPT)),)
		#//$(BOARDDIR)在config.mk中BOARDDIR = $(VENDOR)/$(BOARD)，根据前面的定义BOARDDIR=samsung/itop-4412
		#//LDSCRIPT:=/board/samsung/itop-4412/u-boot.lds  无此文件
		LDSCRIPT := $(srctree)/board/$(BOARDDIR)/u-boot.lds
	endif
	ifeq ($(wildcard $(LDSCRIPT)),)
		#//$(CPUDIR)在config.mk中CPUDIR=arch/$(ARCH)/cpu$(if $(CPU),/$(CPU),)，根据前面的定义CPUDIR=arch/arm/cpu/armv7
		#//LDSCRIPT :=/arch/arm/cpu/armv7/u-boot.lds   无此文件
		LDSCRIPT := $(srctree)/$(CPUDIR)/u-boot.lds
	endif
	ifeq ($(wildcard $(LDSCRIPT)),)
		#//LDSCRIPT := /arch/arm/cpu/u-boot.lds
		LDSCRIPT := $(srctree)/arch/$(ARCH)/cpu/u-boot.lds
	endif
endif
---------------------------------
查看/arch/arm/cpu/u-boot.lds:
#include <config.h>

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text :
	{
		*(.__image_copy_start)
		*(.vectors)
		#//之前CPUDIR定义为arch/arm/cpu/armv7,所以此处为arch/arm/cpu/armv7/start.o
		CPUDIR/start.o (.text*)   
		*(.text*)
	}
	..........................so on.
--------------------------------
对于任何程序，入口函数是在链接时决定的，uboot的入口是由链接脚本决定的。根据前面的分析uboot
链接脚本目录为arch/arm/cpu/u-boot.lds（这个可以在配置文件中与CONFIG_SYS_LDSCRIPT来指定。）
或者在
入口地址也是由连接器决定的，在配置文件中可以由CONFIG_SYS_TEXT_BASE指定。这个会在编译时加在ld连接器的选项-Ttext中
board/samsung/itop-4412/Kconfig 里面的内容
if TARGET_ITOP-4412

config SYS_BOARD
	default "itop-4412"

config SYS_VENDOR
	default "samsung"

config SYS_CONFIG_NAME
	default "itop-4412"

endif

将board/samsung/itop-4412/Kconfig 添加到arch/arm/cpu/armv7/exynos/Kconfig
添加如下内容：
source "board/samsung/itop-4412/Kconfig"
在最后面endmenu 之前添加。在make 

 修改include/configs/itop-4412.h 里面的一个宏定义：
 #define CONFIG_SPL_LDSCRIPT    "board/samsung/common/exynos-uboot-spl.lds"

查看u-boot.lds
*/
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
 . = 0x00000000;
 . = ALIGN(4);
 .text :
 {
  *(.__image_copy_start)
  *(.vectors)
  arch/arm/cpu/armv7/start.o (.text*)
  *(.text*)
 }
 . = ALIGN(4);
 .rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }
 . = ALIGN(4);
 .data : {
  *(.data*)
 }
 . = ALIGN(4);
 . = .;
 . = ALIGN(4);
 .u_boot_list : {
  KEEP(*(SORT(.u_boot_list*)));
 }
/*
链接脚本中这些宏的定义在linkage.h中，看字面意思也明白，程序的入口是在_start.，后面是text段，data段等。
_start在arch/arm/cpu/armv7/start.S中，一段一段的分析，如下：
*/ 
/*************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * setup Memory and board specific bits prior to relocation.
 * relocate armboot to ram
 * setup stack
 *
 *************************************************************************/
/*.global相当于C语言中的Extern，声明此变量，并告诉链接器此变量是全局，外部可以访问*/
	.globl	reset
	.globl	save_boot_params_ret
	.globl	led_on
#define MEM_DLLl_ON

led_on:
	push {r0, r1}
	ldr r0, =0x11000100
	mov r1, #1
	str r1, [r0]
	ldr r0, =0x11000060
	mov r1, #0
	str r1, [r0]
	ldr r0, =0x11000104
	mov r1, #1
	str r1, [r0]
	ldr r0, =0x11000064
	mov r0, #0
	//str r0, [r0]	
	pop {r0, r1}
	bx lr

reset:
	/* Allow the board to save important registers */
	/*注意到 save_boot_params 函数中没有进行任何操作，直接返回了，注释告诉我们栈指针
	还没有初始化，不能向栈中保存任何数据。
	值得注意的是 .weak 伪操作： .weak save_boot_params

可以理解为如果定义了 save_boot_params 这个函数，那么久调用它，如果没有定义，就定义它，*/
	b	save_boot_params
save_boot_params_ret:
	/*
	 * disable interrupts (FIQ and IRQ), also set the cpu to SVC32 mode,
	 * except if in HYP mode already
	 */
	mrs	r0, cpsr
	and	r1, r0, #0x1f		@ mask mode bits
	teq	r1, #0x1a		@ test for HYP mode
	bicne	r0, r0, #0x1f		@ clear all mode bits
	orrne	r0, r0, #0x13		@ set SVC mode
	orr	r0, r0, #0xc0		@ disable FIQ and IRQ
	msr	cpsr,r0

/*
 * Setup vector:
 * (OMAP4 spl TEXT_BASE is not 32 byte aligned.
 * Continue to use ROM code vector only in OMAP4 spl)
 */
#if !(defined(CONFIG_OMAP44XX) && defined(CONFIG_SPL_BUILD))
	/* Set V=0 in CP15 SCTLR register - for VBAR to point to vector */
	mrc	p15, 0, r0, c1, c0, 0	@ Read CP15 SCTLR Register
	bic	r0, #CR_V		@ V = 0
	mcr	p15, 0, r0, c1, c0, 0	@ Write CP15 SCTLR Register

	/* Set vector address in CP15 VBAR register */
	ldr	r0, =_start
	mcr	p15, 0, r0, c12, c0, 0	@Set VBAR
#endif

	/* the mask ROM code should have PLL and others stable */
	/*配置cp15，设置异常向量入口 */
	/*注意：这里由于已经定义CONFIG_SKIP_LOWLEVEL_INIT（itop-4412.h中#include <configs/exynos4-common.h>，
	exynos4-common.h中#include "exynos-common.h"，exynos-common.h中定义了CONFIG_SKIP_LOWLEVEL_INIT），所以跳过直接执行main*/
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
/*cpu_init_cp15函数是配置cp15协处理器相关寄存器来设置处理器的MMU，cache以及tlb。如果没有定义CONFIG_SYS_ICACHE_OFF则会打开icache。关掉mmu以及tlb。
具体配置过程可以对照cp15寄存器来看，这里不详细说了*/
	bl	cpu_init_cp15
	/*cpu_init_crit调用的level_init函数与特定开发板相关的初始化函数，在这个函数里会做一些pll
	初始化，如果不是从mem启动，则会做memory初始化，方便后续拷贝到mem中运行（注意：这个开发板
	没有在这里做内存初始化）*/
	bl	cpu_init_crit
#endif

	bl	_main
/*-------------------------------------
不运行仅为解释。
-------------------------------------*/
ENTRY(cpu_init_crit)
/*看注释可以明白，cpu_init_crit调用的lowlevel_init函数是与特定开发板相关的初始化函数，在这个函数
里会做一些pll初始化，如果不是从mem启动，则会做memory初始化，方便后续拷贝到mem中运行。
lowlevel_init函数则是需要移植来实现，做clk初始化以及ddr初始化

从cpu_init_crit返回后，_start的工作就完成了，接下来就要调用_main，总结一下_start工作：

1 前面总结过的部分，初始化异常向量表，设置svc模式，关中断

2 配置cp15，初始化mmu cache tlbq

3 板级初始化，pll memory初始化*/
	/*
	 * Jump to board specific initialization...
	 * The Mask ROM will have already initialized
	 * basic memory. Go here to bump up clock rate and handle
	 * wake up conditions.
	 */
	b	lowlevel_init		@ go setup pll,mux,memory
ENDPROC(cpu_init_crit)
#endif



//_main函数在arch/arm/lib/crt0.S中，mian函数的作用在注释中有详细的说明 
/*///////////////////////  crt0.S   //////////////////////// */
ENTRY(_main)

/*
 * Set up initial C runtime environment and call board_init_f(0).
 */

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)
	ldr	sp, =(CONFIG_SPL_STACK)
#else
/*首先将CONFIG_SYS_INIT_SP_ADDR定义的值加载到栈指针sp中，这个宏定义在配置头文件itop-4412.h中指定。
CONFIG_SYS_TEXT_BASE+UBOOT_SIZE-0x1000*/
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
		/*
		->spl_boot.c中board_init_f
			void board_init_f(unsigned long bootflag)
			{
				__aligned(8) gd_t local_gd;
				__attribute__((noreturn)) void (*uboot)(void);
			
				设置全局指针变量的值
				setup_global_data(&local_gd);
				//如果是唤醒则退出唤醒（这段没有分析太明白，如有错误望指正）
				if (do_lowlevel_init()) {
					power_exit_wakeup();
				}
				这里函数
				do_lowlevel_init()
					->arch_cpu_init()；
					reset_status = get_reset_status();获取复位状态并赋相应的值给变量，
					switch (reset_status)然后做出相应的动作：actions = DO_CLOCKS | DO_MEM_RESET | DO_POWER;
					set_ps_hold_ctrl();关于供电锁存类PS_HOLD;
					system_clock_init();系统时钟初始化；
					test_uart();测试串口；
					mem_ctrl_init(actions & DO_MEM_RESET);ddr内存初始化；
					//tzpc_init();tzpc这个安全源代码去掉了，原因未知
				/*
				
				//Copy U-boot from mmc to RAM 将uboot code拷贝到relocaddr
				copy_uboot_to_ram();
			
				/*使用printascii函数打印prink缓存信息*//*
				printascii("copy BL2 ends.\n");
				/* Jump to U-Boot image *//*
				uboot = (void *)CONFIG_SYS_TEXT_BASE;
				(*uboot)();
				/* Never returns Here *//*
			}

		*/
	

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
	/*将uboot code拷贝到relocaddr，注意这里没有用到！*/
	b	relocate_code
	 /*	 * void relocate_code(addr_moni)
			 *
			 * This function relocates the monitor code.
			 *
			 * NOTE:
			 * To prevent the code below from containing references with an R_ARM_ABS32
			 * relocation record type, we never refer to linker-defined symbols directly.
			 * Instead, we declare literals which contain their relative location with
			 * respect to relocate_code, and at run time, add relocate_code back to them.
			 *
			
			ENTRY(relocate_code)
				ldr	r1, =__image_copy_start	//* r1 <- SRC &__image_copy_start *
				subs	r4, r0, r1		//* r4 <- relocation offset *
				注意：这里判断是否运行在sdram上，也就是是否重定位完，如果重定位完就直接跳到后面relocate_done函数
				这里我们之前已经copy_uboot_to_ram();在CONFIG_SYS_TEXT_BASE上运行，所以跳转
				beq	relocate_done		//* skip relocation *       
				ldr	r2, =__image_copy_end	//* r2 <- SRC &__image_copy_end *
			
			copy_loop:
				ldmia	r1!, {r10-r11}		//* copy from source address [r1]    *
				stmia	r0!, {r10-r11}		//* copy to   target address [r0]    *
				cmp	r1, r2			//* until source end address [r2]    
				blo	copy_loop
			
				
				 //* fix .rel.dyn relocations
				 
				ldr	r2, =__rel_dyn_start	//* r2 <- SRC &__rel_dyn_start 
				ldr	r3, =__rel_dyn_end	//* r3 <- SRC &__rel_dyn_end 
			fixloop:
				ldmia	r2!, {r0-r1}		//* (r0,r1) <- (SRC location,fixup) 
				and	r1, r1, #0xff
				cmp	r1, #23			//* relative fixup
				bne	fixnext
			
				/* relative fix: increase location by offset *
				add	r0, r0, r4
				ldr	r1, [r0]
				add	r1, r1, r4
				str	r1, [r0]
			fixnext:
				cmp	r2, r3
				blo	fixloop
			
			relocate_done:    跳转到这里
			
			#ifdef __XSCALE__        未定义，跳过
				
				//* On xscale, icache must be invalidated and write buffers drained,
				// * even with cache disabled - 4.2.7 of xscale core developer's manual
				//*
				mcr	p15, 0, r0, c7, c7, 0	//* invalidate icache *
				mcr	p15, 0, r0, c7, c10, 4	//* drain write buffer *
			#endif
			
				// ARMv4- don't know bx lr but the assembler fails to see that 
			
			#ifdef __ARM_ARCH_4__     未定义，执行else
				mov	pc, lr
			#else
				bx	lr    BX指令是ARM指令系统中的带状态切换跳转指令，跳转到lr地址，lr之前被赋值到here函数的地方，所以跳转到here
			#endif
			
			ENDPROC(relocate_code)
	*/
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


/*///////////////////////  board_r.c   //////////////////////// */
__weak void board_init_r(gd_t *new_gd, ulong dest_addr)
{
#ifdef CONFIG_NEEDS_MANUAL_RELOC
	int i;
#endif

#ifdef CONFIG_AVR32
	mmu_init_r(dest_addr);
#endif

#if !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#endif

#ifdef CONFIG_NEEDS_MANUAL_RELOC

	for (i = 0; i < ARRAY_SIZE(init_sequence_r); i++)
		init_sequence_r[i] += gd->reloc_off;
#endif
	/*调用initcall_run_list(init_sequence_r)函数执行一系列初始化函数以实现后半部
	分板级初始化
	board_init,	// Setup chipselects 设置机器ID
	initr_env,	->
		set_default_env(NULL);->
			himport_r(&env_htab, (char *)default_environment,
			sizeof(default_environment), '\0', flags, 0,
			0, NULL) == 0)
			//使用默认的配置
			const uchar default_environment[] = {
			#ifdef	CONFIG_BOOTARGS
				"bootargs="	CONFIG_BOOTARGS			"\0"
			#endif
			////在itop-4412.h配置CONFIG_BOOTCOMMAND参数
			#ifdef	CONFIG_BOOTCOMMAND
				"bootcmd="	CONFIG_BOOTCOMMAND		"\0"
			#endif
			....................
			//在itop-4412.h配置CONFIG_EXTRA_ENV_SETTINGS了很多参数
			//CONFIG_EXTRA_ENV_SETTINGS和CONFIG_BOOTCOMMAND这两个宏的意思就是把你手工在console上敲的代码集合在一起。
			//比如 CONFIG_BOOTCOMMAND  的效果就是在uboot的console上一行一行把uboot命令敲进去（去掉“”和\），一直到
			//run netboot这条命令。这些都是uboot的命令或uboot环境设置。
			#ifdef	CONFIG_EXTRA_ENV_SETTINGS
				CONFIG_EXTRA_ENV_SETTINGS
			#endif
			}
	最后进入run_main_loop不再返回*/
	if (initcall_run_list(init_sequence_r))
		hang();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}

/*///////////////////////  board_r.c   //////////////////////// */

static int run_main_loop(void)
{
#ifdef CONFIG_SANDBOX
	sandbox_main_loop_init();
#endif
	/* main_loop() can return to retry autoboot, if so just run it again */
	for (;;)
		main_loop();
	return 0;
}

/*///////////////////////  main.c   //////////////////////// */
/* We come here after U-Boot is initialised and ready to process commands */
void main_loop(void)
{
	const char *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

#ifndef CONFIG_SYS_GENERIC_BOARD
	puts("Warning: Your board does not use generic board. Please read\n");
	puts("doc/README.generic-board and take action. Boards not\n");
	puts("upgraded by the late 2014 may break or be removed.\n");
#endif

	modem_init();
#ifdef CONFIG_VERSION_VARIABLE
	setenv("ver", version_string);  /* set version variable */
#endif /* CONFIG_VERSION_VARIABLE */

	cli_init();

	run_preboot_environment_command();

#if defined(CONFIG_UPDATE_TFTP)
	update_tftp(0UL);
#endif /* CONFIG_UPDATE_TFTP */
	/*main_loop中调用bootdelay_process函数判断在delay time中有没有键按下，并给出
	prompt"Hit any key to stop autoboot"，同时return的s="bootcmd"*/
	s = bootdelay_process();
	if (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);
/*若没有键按下则autoboot_command(s)(s="bootcmd").这里的bootcmd需要根据内核的位置设置，
若bootcmd未赋值，U-boot会一直等待。如果敲入bootm，也会启动内核，并传入参数。*/
	autoboot_command(s);

	cli_loop();
}

/*///////////////////////  autoboot.c   //////////////////////// */

/*main函数调用到这里

bootcmd是否有定义？
./include/env_default.h定义bootcmd为CONFIG_BOOTCOMMAND
./inlcude/itop-4412.h中定义CONFIG_BOOTCOMMAND的值，其中bootm是我们需要关心的重点。
"mmc read ${loadaddr} 0x1000 0x4000; mmc read ${dtb_addr} 0x800 0xa0; bootm ${loadaddr} - ${dtb_addr}" \
mmc read 0x40007000 0x1000 0x4000即将存储设备上从块号0x1000开始的0x4000个存储块的东西
拷贝到内存0x40007000开始的空间内。

*这里顺便提一下itop-4412.h中的参数CONFIG_EXTRA_ENV_SETTINGS，其中定义了很多的参数
包括：
"loadaddr=0x40007000\0" \
	"rdaddr=0x48000000\0" \
	"kerneladdr=0x40007000\0" \
	"ramdiskaddr=0x48000000\0" \
	"console=ttySAC2,115200n8\0" \
	"mmcdev=0\0" \
	"bootenv=uEnv.txt\0" \
	"dtb_addr=0x41000000\0" \
	"dtb_name=exynos4412-itop-4412.dtb\0" \
	"loadbootenv=load mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"bootargs=console=ttySAC2,115200n8 earlyprintk\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
	"env import -t $loadaddr $filesize\0" \
    "loadbootscript=load mmc ${mmcdev} ${loadaddr} boot.scr\0" \
    "bootscript=echo Running bootscript from mmc${mmcdev} ...; " \
    "source ${loadaddr}\0"
加载地址、内核在内存存放的地址、终端波特率、bootargs等等
猜测eMMC分区也会用到这些参数。****************************************************

继续回来看：
bootm:./common/cmd_bootm.c
U_BOOT_CMD(
	bootm,	CONFIG_SYS_MAXARGS,	1,	do_bootm,
	"boot application image from memory", bootm_help_text
);
uboot命令中，bootm对应的函数为do_bootm(),在./common/cmd_bootm.c;
bootm - boot application image from image in memory 
do_bootm()调用do_bootm_states()在./common/bootm.c;

do_bootm_states()
调用bootm_find_os->boot_get_kernel
printf("## Booting kernel from Legacy Image at %08lx ...\n",
		       img_addr);
调用bootm_find_other->bootm_find_ramdisk_fdt->bootm_find_fdt->boot_get_fdt->
printf("   Booting using the fdt blob at %#08lx\n", fdt_addr);

打印do_bootm_states: reserve fdt done.
并调用bootm_os_get_boot_func()
bootm_os_get_func()中boot_os[os]数组中对应到linux相关的函数入口，
boot_os[os]数组中找到linux的入口do_bootm_linux()

do_bootm_linux()在arch/arm/lib.
boot_prep_linux(images);实现uboot到kernel的参数传递（tag）
调用image_setup_linux(images)然后调用boot_relocate_fdt().
boot_relocate_fdt会在bootmap区
域中申请一块未被使用的内存，接着将DTB镜像内容复制到这块区域（即归lmb所管理的区域）


boot_jump_linux(images, flag);启动kernel，打印：## Transferring control to 
Linux (at address 40007000)...
*/
void autoboot_command(const char *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

/*//abortboot函数如果延时大于等于零，并且没有在延时过程中接收到按键，则引导内核。
abortboot函数里会打印Hit any key to stop autoboot:。*/
	if (stored_bootdelay != -1 && s && !abortboot(stored_bootdelay)) {
#if defined(CONFIG_AUTOBOOT_KEYED) && !defined(CONFIG_AUTOBOOT_KEYED_CTRLC)
		int prev = disable_ctrlc(1);	/* disable Control C checking */
#endif

/*run_command_list中调用了hush shell的命令解释器(parse_stream_outer函数)，解释bootcmd中的启动命令。*/
		run_command_list(s, -1, 0);

#if defined(CONFIG_AUTOBOOT_KEYED) && !defined(CONFIG_AUTOBOOT_KEYED_CTRLC)
		disable_ctrlc(prev);	/* restore Control C checking */
#endif
	}

#ifdef CONFIG_MENUKEY
	if (menukey == CONFIG_MENUKEY) {
		s = getenv("menucmd");
		if (s)
			run_command_list(s, -1, 0);
	}
#endif /* CONFIG_MENUKEY */
}






















 
 
 
 
 
 