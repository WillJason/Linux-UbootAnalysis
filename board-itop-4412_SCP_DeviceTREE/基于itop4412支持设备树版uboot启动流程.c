/*
uboot版本：2015.04
先来看一下大致的启动流程。
大体上来说：
RomBoot --> SPL --> u-boot --> Linux kernel --> file system --> start application
开发板Samsung exynos4412 uboot引导程序分为BL1、BL2、U-BOOT、TZSW四个部分。
通过控制启动模式（oM），处理器上电以后，开始执行IROM中的代码，这段代码是原厂三星固化在芯片中，是无法修改的。
BL1可以放在4个引导设备上：NAND,SD/MMC,EMMC,USB
IROM启动以后，会去找BL1，BL1执行完以后去找BL2，BL2会运行uboot（这里其实中间还有个搬移到ram执行第一阶段的过
程：很多芯片厂商固化的ROM支持从nandflash、SDCARD等外部介质启动。所谓启动，就是从这些外部介质中搬移一段
固定大小（4K/8K/16K等）的代码到内部RAM中运行），uboot引导linux内核，内核启动完毕，uboot的生命周期完全结束。
IROM->BL1->BL2->uboot->linux
但是这里我们引入了SPL（Secondary programloader）方式。流程稍稍有些变化，但是总体来说没有改变，只不过是第一
阶段搬移的对象改变了(个人理解).之前搬运的是uboot前*K大小的代码，现在搬运的spl.所以可以这么说
IROM--->BL1--->BL2--->SPL--->uboot--->linux
-------------------------------------
再看一下这个uboot的配置过程：
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
------------------------主体uboot脚本选配----------------------------------------------------------------------------------
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
---------------------------------
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
在最后面endmenu 之前添加。再make 
-------------------------spl uboot脚本选配-----------------------------------------------------------------------------------
查看顶层Makefile中定义的定义的spl链接脚本的位置在哪：
	all:    $(ALL-y)
	ALL-$(CONFIG_SPL) += spl/u-boot-spl.bin
	## 当配置了CONFIG_SPL，make的时候就会执行spl/u-boot-spl.bin这个目标
	spl/u-boot-spl.bin: spl/u-boot-spl
	
	spl/u-boot-spl: tools prepare
		$(Q)$(MAKE) obj=spl -f $(srctree)/scripts/Makefile.spl all
	## obj=spl 会在out/u-boot目录下生成spl目录
	## -f $(srctree)/scripts/Makefile.spl 说明执行的Makefile文件是scripts/Makefile.spl
	## $(MAKE) all 相当于make的目标是all
综上，由CONFIG_SPL来决定是否需要编译出spl文件
spl/u-boot-spl.lds依赖关系: 
在project-x/u-boot/scripts/Makefile.spl中
这里主要是为了找到一个匹配的连接文件。
$(obj)/u-boot-spl.lds: $(LDSCRIPT) FORCE
	$(call if_changed_dep,cpp_lds)
## 依赖于$(LDSCRIPT)，$(LDSCRIPT)定义了连接脚本所在的位置，
## 然后把链接脚本经过cpp_lds处理之后复制到$(obj)/u-boot-spl.lds中，也就是spl/u-boot-spl.lds中。
## cpp_lds处理具体实现看cmd_cpu_lds定义，具体是对应连接脚本里面的宏定义进行展开。

## $(LDSCRIPT)定义如下:
# Linker Script
ifdef CONFIG_SPL_LDSCRIPT
# need to strip off double quotes
LDSCRIPT := $(addprefix $(srctree)/,$(CONFIG_SPL_LDSCRIPT:"%"=%))
endif

ifeq ($(wildcard $(LDSCRIPT)),)
	LDSCRIPT := $(srctree)/board/$(BOARDDIR)/u-boot-spl.lds
endif
ifeq ($(wildcard $(LDSCRIPT)),)
	LDSCRIPT := $(srctree)/$(CPUDIR)/u-boot-spl.lds
endif
ifeq ($(wildcard $(LDSCRIPT)),)
	LDSCRIPT := $(srctree)/arch/$(ARCH)/cpu/u-boot-spl.lds
endif
ifeq ($(wildcard $(LDSCRIPT)),)
$(error could not find linker script)
endif
## 也就是说依次从board/板级目录、cpudir目录、arch/架构/cpu/目录下去搜索u-boot-spl.lds文件。
## 例如，xxxx-4412(exynos4412 armv7)最终会在./arch/arm/cpu/下搜索到u-boot-spl.lds
我们在配置文件里定义了CONFIG_SPL_LDSCRIPT,所以前面Linker Script后一句有效
#define CONFIG_SPL_LDSCRIPT	"board/samsung/common/exynos-uboot-spl.lds"
综上，最终指定了project-X/u-boot/board/samsung/common/exynos-uboot-spl.lds作为连接脚本。

所以移植的时候要修改include/configs/itop-4412.h 里面的一个宏定义：
#define CONFIG_SPL_LDSCRIPT    "board/samsung/common/exynos-uboot-spl.lds"

SPL复用的是uboot里面的代码.那要生成我们所需要的SPL目标文件，我们又该如何下手呢？很容易想到，
通过编译选项便可以将SPL和uboot代码分离、复用。这里所说的编译选项便是CONFIG_SPL_BUILD，在
make Kconfig的时候使能。最终编译生成的SPL二进制文件有u-boot-spl，u-boot-spl.bin以及u-boot-spl.map
CONFIG_SPL_TEXT_BASE    SPL的入口地址
IN	project-x/u-boot/scripts/Makefile.spl
	LDFLAGS_$(SPL_BIN) += -T u-boot-spl.lds $(LDFLAGS_FINAL)
	ifneq ($(CONFIG_SPL_TEXT_BASE),)
	LDFLAGS_$(SPL_BIN) += -Ttext $(CONFIG_SPL_TEXT_BASE)
	endif
spl的编译是编译uboot的一部分，和uboot.bin走的是两条编译流程，这个要重点注意。 
正常来说，会先编译主体uboot，也就是uboot.bin.再编译uboot-spl，也就是uboot-spl.bin,虽然编译命令
是一起的，但是编译流程是分开的。

分析源代码：先来看SPL部分
查看u-boot-spl.lds:前面分析是主体uboot链接脚本/board/samsung/common/exynos-uboot-spl.lds:	
*/
MEMORY { .sram : ORIGIN = CONFIG_SPL_TEXT_BASE, \
		LENGTH = CONFIG_SPL_MAX_FOOTPRINT }

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS
{
	.text :
	{
		__start = .;
		*(.vectors)
		arch/arm/cpu/armv7/start.o (.text*)
		*(.text*)
	} >.sram
	. = ALIGN(4);

	.rodata : { *(SORT_BY_ALIGNMENT(.rodata*)) } >.sram
	. = ALIGN(4);

	.data : { *(SORT_BY_ALIGNMENT(.data*)) } >.sram
	. = ALIGN(4);

	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	} >.sram
	. = ALIGN(4);

	/* Align .machine_param on 256 byte boundary for easier searching */
	.machine_param ALIGN(0x100) : { *(.machine_param) } >.sram
	. = ALIGN(4);

	__image_copy_end = .;

	.end :
	{
		*(.__end)
	} >.sram

	.bss :
	{
		. = ALIGN(4);
		__bss_start = .;
		*(.bss*)
		. = ALIGN(4);
		__bss_end = .;
	} >.sram
}
/*
SPL:_start在arch/arm/cpu/armv7/start.S中，一段一段的分析，如下：
*/ 
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
/*///////////////////////  crt0.S   //////////////////////// */
ENTRY(_main)
//#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)	//CONFIG_SPL_BUILD在spl里定义了
	//首先将CONFIG_SPL_STACK定义的值加载到栈指针sp中，这个宏定义在配置头文件itop-4412.h中指定。
	ldr	sp, =(CONFIG_SPL_STACK)
//#else
//	ldr	sp, =(CONFIG_SYS_INIT_SP_ADDR)
//#endif
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
		/*现在在SPL里，由于定义了CONFIG_SPL，也就定义了CONFIG_SPL_BUILD,故调用的是Spl_boot.c*/
		/*
		->spl_boot.c中board_init_f
			void board_init_f(unsigned long bootflag)
			{
				__aligned(8) gd_t local_gd;
				__attribute__((noreturn)) void (*uboot)(void);
			
				//设置全局指针变量的值
				setup_global_data(&local_gd);
				//如果是唤醒则退出唤醒（这段没有分析太明白，如有错误望指正）
				if (do_lowlevel_init()) {
					power_exit_wakeup();
				}
				//这里函数
				do_lowlevel_init()
					->arch_cpu_init()；
					reset_status = get_reset_status();获取复位状态并赋相应的值给变量，
					switch (reset_status)然后做出相应的动作：actions = DO_CLOCKS | DO_MEM_RESET | DO_POWER;
					set_ps_hold_ctrl();关于供电锁存类PS_HOLD;
					system_clock_init();系统时钟初始化；
					test_uart();测试串口；
					mem_ctrl_init(actions & DO_MEM_RESET);ddr内存初始化；
					//tzpc_init();tzpc这个安全源代码去掉了，必须关掉，否则从SPL跳转到UBOOT执行时死机。
				/*				
				//Copy U-boot from mmc to RAM 将uboot code拷贝到relocaddr
				copy_uboot_to_ram();
			
				/*使用printascii函数打印prink缓存信息*//*
				printascii("copy BL2 ends.\n");
				/* Jump to U-Boot image *//*
				uboot = (void *)CONFIG_SYS_TEXT_BASE;
				(*uboot)();
				/*从SPL到U-Boot第二阶段主体uboot代码的跳转执行！！！
				 *翻译成我们所熟悉的汇编跳转的话应该是：ldr pc, =CONFIG_SYS_TEXT_BASE
				 *//*
			}
		*/

#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_FRAMEWORK)
# ifdef CONFIG_SPL_BUILD
	/* Use a DRAM stack for the rest of SPL, if requested */
	bl	spl_relocate_stack_gd
	cmp	r0, #0
	movne	sp, r0
# endif
	ldr	r0, =__bss_start	/* this is auto-relocated! */

	ldr	r1, =__bss_end		/* this is auto-relocated! */
	mov	r2, #0x00000000		/* prepare zero to clear BSS */

clbss_l:cmp	r0, r1			/* while not at end of BSS */
	strlo	r2, [r0]		/* clear 32-bit BSS word */
	addlo	r0, r0, #4		/* move to next */
	blo	clbss_l
	/* call board_init_r(gd_t *id, ulong dest_addr) */
	mov     r0, r9                  /* gd_t */
	ldr	r1, [r9, #GD_RELOCADDR]	/* dest_addr */
	/* call board_init_r */
	ldr	pc, =board_init_r	/* this is auto-relocated! */
	//在SPL中是spl_boot.c中的board_init_r函数，是个空函数死循环，当然永远不会运行到这里，因为之前已经跳到第二阶段了
	/* we should not return here. */
#endif
ENDPROC(_main)						//结束

/*
分析源代码：再来看主体U-boot部分（第二阶段）
查看u-boot.lds:前面分析是主体uboot链接脚本/arch/arm/cpu/u-boot.lds:
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
//#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)	//CONFIG_SPL_BUILD在主体uboot未定义
//	ldr	sp, =(CONFIG_SPL_STACK)
//#else
/*首先将CONFIG_SYS_INIT_SP_ADDR定义的值加载到栈指针sp中，这个宏定义在配置头文件itop-4412.h中指定。
CONFIG_SYS_TEXT_BASE+UBOOT_SIZE-0x1000*/
	ldr	sp, =(CONFIG_SYS_INIT_SP_ADDR)
//#endif
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
		/*现在在主体uboot中，未使能CONFIG_SPL_BUILD,调用的是Board_f.c (common)*/
		/*
		->board_f.c中board_init_f
			__weak void board_init_f(ulong boot_flags)
			{
				gd->flags = boot_flags;
				gd->have_console = 0;
				printascii("board_init_f.\n");
			
				if (initcall_run_list(init_sequence_f))
					hang();
			}
		*/
		//这里调用个函数集init_sequence_f,下面来分析这个非常重要的部分
		/*
		static init_fnc_t init_sequence_f[] = {
			........
			/ *
			initf_dm
			这里涉及到了DM驱动模型相关的东西.
			1、DM驱动模型的一般流程bind->ofdata_to_platdata(可选)->probe
			2、启动，bind操作时单独完成的，主要完成设备和驱动的绑定，以及node 注：node是匹配到驱动的设备节点
			之间的连接。ofdata_to_platdata(可选)->probe 则是在device_probe函数中完成的。
			initf_dm主要是初始化驱动树和uclass树，并绑定含有DM_FLAG_PRE_RELOC标志位和特殊设备树特性的节点
			* /
			initf_dm,->       								//执行bind操作，初始化一个dm模型的树形结构
				dm_init_and_scan(true)->				//初始化根节点设备，并bind根节点的带有u-boot,dm-pre-reloc属性的一级子节点。对DM进行初始化和设备的解析
					//DM的初始化
					.创建根设备root的udevice，存放在gd->dm_root中。
					.根设备其实是一个虚拟设备，主要是为uboot的其他设备提供一个挂载点。
					.初始化uclass链表gd->uclass_root
					dm_init	->											//将根节点绑定到gd->dm_root上，初始化根节点设备
						//DM_ROOT_NON_CONST是指根设备udevice，root_info是表示根设备的设备信息
        		//device_bind_by_name会查找和设备信息匹配的driver，然后创建对应的udevice和uclass并进行绑定，最后放在DM_ROOT_NON_CONST中。
        		//device_bind_by_name后续我们会进行说明，这里我们暂时只需要了解root根设备的udevice以及对应的uclass都已经创建完成。
						device_bind_by_name(NULL, false, &root_info, &DM_ROOT_NON_CONST);
							drv = lists_driver_lookup_name(info->name);
							device_bind			//在device_bind中实现了udevice和uclass的创建和绑定以及一些初始化操作
						// 对根设备执行probe操作，device_probe后续再进行说明
						device_probe(DM_ROOT_NON_CONST);     //device_probe(dev)进行设备的激活和驱动的注册，这里是根设备
					dm_scan_platdata ->						//从平台设备中解析udevice和uclass,搜索使用宏U_BOOT_DEVICE定义的设备进行驱动匹配，也就是bind子节点
					dm_scan_fdt	->								//在设备树种搜索设备并进行驱动匹配，然后bind
						dm_scan_fdt_node//具体绑定设备的入口，在该函数中会确定设备是否具有boot,dm-pre-reloc属性，如果没有则不会绑定
		          lists_bind_fdt//搜索可以匹配到该设备的驱动
		            device_bind				//有关device_bind()的DM实现函数在后面附录有详解
                  uclass_bind_device//将该设备挂在对应的U_CLASS链表上
                  drv->bind(dev)//设备驱动的bind接口函数
                  parent->driver->child_post_bind(dev)//父节点驱动的child_post_bind接口函数
                  uc->uc_drv->post_bind//设备所属类的驱动的post_bind接口函数（具体的设备节点就是在这个接口下在soc下进行展开的）
					dm_scan_other	->

			env_init,//仅仅把环境参数的地址赋值
			setup_machine,//设置机器ID
			init_baud_rate,	//设置波特率
			........
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
	/*现在是主体uboot，所以调用的是Board_r.c (common)这个文件中的函数*/
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
	
	initr_dm,	->				////执行bind操作，初始化一个dm模型的树形结构
		/ *
			dm初始化的接口在dm_init_and_scan中。 可以发现在uboot relocate之前的initf_dm和之后
		的initr_dm都调用了这个函数。主要区别在于参数。首先说明一下dts节点中的“u-boot,dm-pre-reloc”
		属性，当设置了这个属性时，则表示这个设备在relocate之前就需要使用。当dm_init_and_scan的参
		数为true时，只会对带有“u-boot,dm-pre-reloc”属性的节点进行解析。而当参数为false的时候，
		则会对所有节点都进行解析。“u-boot,dm-pre-reloc”的情况比较少			* /
		dm_init_and_scan(false)	->
			.......................(这里和之前调用的是同一个函数，不再重复解析)
			/ * 
				在init_sequence_r中的initr_dm中，完成了FDT的解析，解析了所有的外设node,并针对各个
			节点进行了 udevice和uclass的创建，以及各个组成部分的绑定关系。注意，这里只是绑定，
			即调用了driver的bind函数，但是设备还没有真正激活，也就是还没有执行设备的probe函数。
			将在具体xxx设备初始化阶段进行相关设备的激活和驱动的注册
				假设后面XXX设备初始化，其中最主要的工作是调用uclass_first_device(UCLASS_XXX, &dev)
			或者XXX_probe等等函数，从uclass的设备链表中获取第一个udevice，并且进行probe。最终，是通过调
			用device_probe(dev)进行XXX设备的激活和驱动的注册
				下面分析device_probe(dev)的实现的部分过程：
				.分配设备的私有数据
				.对父设备进行probe				
				.执行probe device之前uclass需要调用的一些函数				
				.调用driver的ofdata_to_platdata，将dts信息转化为设备的平台数据（重要）		
				.调用driver的probe函数（重要）		
				.执行probe device之后uclass需要调用的一些函数
			* /
	
	initr_mmc,	//初始化emmc和SD卡。比较多，放在board_init_r函数后面解析
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
/*-------------------------------------------------------------------------------------------------
/common/board_r.c:init_sequence_r[]中initr_mmc函数
--------------------------------------------------------------------------------------------------*/
initr_mmc,->						///初始化eMMC，分析这个函数 
	mmc_initialize(gd->bd);->
		board_mmc_init(bis)		//board/samsung/common
			init_mmc();															//SD相关		
				exynos_mmc_init()
					fdtdec_find_aliases_for_id()				//查找EXYNOS_MMC类型
					process_nodes()											//配置和初始化MMC设备
						sdhci_get_config()
							pinmux_decode_periph_id(blob, node);//获取dev_id
							fdtdec_get_int(blob, node, "samsung,bus-width", 0);
							fdtdec_get_addr(blob, node, "reg");
							gpio_request_by_name_nodev(blob, node, "cd-gpios", 0, &host->cd_gpio,GPIOD_IS_IN);
						do_sdhci_init(host)
							exynos_pinmux_config(dev_id, flag);	//配置端口
							s5p_sdhci_core_init(host);
								host->version = sdhci_readw(host, SDHCI_HOST_VERSION);//设置调用的函数
								host->set_control_reg = &s5p_sdhci_set_control_reg;
								host->set_clock = set_mmc_clk;
								add_sdhci();			//注册
							
			init_dwmmc();														//eMMC相关
				exynos_dwmmc_init()
					fdtdec_find_aliases_for_id()				//查找EXYNOS_DWMMC类型，找到设备树中emmc的节点
					exynos_dwmci_process_node()					//配置和初始化DWMMC设备
						exynos_dwmci_get_config()					//获取设备树信息
							priv = malloc(sizeof(struct dwmci_exynos_priv_data));
							fdtdec_get_int(blob, node, "index", host->dev_id);	//读取设备树中对应的信息
							fdtdec_get_int(blob, node, "samsung,bus-width", 0);
							fdtdec_get_int_array(blob, node, "samsung,timing", timing, 3);
							fdtdec_get_int(blob, node, "fifoth_val", 0);
							fdtdec_get_int(blob, node, "bus_hz", 0);
							fdtdec_get_int(blob, node, "div", 0);
						do_dwmci_init(host)
							exynos_pinmux_config(host->dev_id, flag);//配置端口
							exynos_dwmci_core_init(host, index);
								get_mmc_clk(index);
								set_mmc_clk(index, div);
								host->board_init = exynos_dwmci_board_init;//设置调用的函数
								host->caps = MMC_MODE_DDR_52MHz;
								host->clksel = exynos_dwmci_clksel;
								host->dev_index = index;
								host->get_mmc_clk = exynos_dwmci_get_clk;		 
								add_dwmci();		//注册
		do_preinit();
			mmc_start_init()		//初始化MMC，以及发送CMD0,CMD2...设置mmc
/*
	从这里我们看出，这个启动使用了设备树，通过获取设备树信息来配置启动的设备。
	还有就是在前面虽然有DM驱动的初始化工作，但是后面基本都没有用到DM驱动，也许是刚刚引入这个概念的关系，
很多方面还不是很成熟。
*/
/*------------------------------------------------------------------------------------------------
initr_mmc函数====>END.
-------------------------------------------------------------------------------------------------*/

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











//附录：
/*
device_bind函数
在device_bind中实现了udevice和uclass的创建和绑定以及一些初始化操作，这里专门学习一下device_bind。
device_bind的实现如下(去除部分代码)
driver/core/device.c
*/
int device_bind(struct udevice *parent, const struct driver *drv,
        const char *name, void *platdata, int of_offset,
        struct udevice **devp)
// parent:父设备
// drv：设备对应的driver
// name：设备名称
// platdata：设备的平台数据指针
// of_offset：在dtb中的偏移，即代表了其dts节点
// devp：所创建的udevice的指针，用于返回
{
    struct udevice *dev;
    struct uclass *uc;
    int size, ret = 0;

    ret = uclass_get(drv->id, &uc);
        // 获取driver id对应的uclass，如果uclass原先并不存在，那么会在这里创建uclass并其uclass_driver进行绑定
    dev = calloc(1, sizeof(struct udevice));
        // 分配一个udevice

    dev->platdata = platdata; // 设置udevice的平台数据指针
    dev->name = name; // 设置udevice的name
    dev->of_offset = of_offset; // 设置udevice的dts节点偏移
    dev->parent = parent; // 设置udevice的父设备
    dev->driver = drv;    // 设置udevice的对应的driver，相当于driver和udevice的绑定
    dev->uclass = uc;    // 设置udevice的所属uclass

    dev->seq = -1;
    dev->req_seq = -1;
    if (CONFIG_IS_ENABLED(OF_CONTROL) && CONFIG_IS_ENABLED(DM_SEQ_ALIAS)) {
        /*
         * Some devices, such as a SPI bus, I2C bus and serial ports
         * are numbered using aliases.
         *
         * This is just a 'requested' sequence, and will be
         * resolved (and ->seq updated) when the device is probed.
         */
        if (uc->uc_drv->flags & DM_UC_FLAG_SEQ_ALIAS) {
            if (uc->uc_drv->name && of_offset != -1) {
                fdtdec_get_alias_seq(gd->fdt_blob,
                        uc->uc_drv->name, of_offset,
                        &dev->req_seq);
            }
                    // 设置udevice的alias请求序号
        }
    }
    if (!dev->platdata && drv->platdata_auto_alloc_size) {
        dev->flags |= DM_FLAG_ALLOC_PDATA;
        dev->platdata = calloc(1, drv->platdata_auto_alloc_size);
                // 为udevice分配平台数据的空间，由driver中的platdata_auto_alloc_size决定
    }

    size = uc->uc_drv->per_device_platdata_auto_alloc_size;
    if (size) {
        dev->flags |= DM_FLAG_ALLOC_UCLASS_PDATA;
        dev->uclass_platdata = calloc(1, size);
                // 为udevice分配给其所属uclass使用的平台数据的空间，由所属uclass的driver中的per_device_platdata_auto_alloc_size决定
    }
    /* put dev into parent's successor list */
    if (parent)
        list_add_tail(&dev->sibling_node, &parent->child_head);
        // 添加到父设备的子设备链表中
    ret = uclass_bind_device(dev);
        // uclass和udevice进行绑定，主要是实现了将udevice链接到uclass的设备链表中
    /* if we fail to bind we remove device from successors and free it */
    if (drv->bind) {
        ret = drv->bind(dev);
        // 执行udevice对应driver的bind函数
    }

    if (parent && parent->driver->child_post_bind) {
        ret = parent->driver->child_post_bind(dev);
        // 执行父设备的driver的child_post_bind函数
    }

    if (uc->uc_drv->post_bind) {
        ret = uc->uc_drv->post_bind(dev);
        if (ret)
            goto fail_uclass_post_bind;
        // 执行所属uclass的post_bind函数
    }

    if (devp)
        *devp = dev;
        // 将udevice进行返回

    dev->flags |= DM_FLAG_BOUND;
        // 设置已经绑定的标志
        // 后续可以通过dev->flags & DM_FLAG_ACTIVATED或者device_active宏来判断设备是否已经被激活
    return 0;
}










 
 
 
 
 
 