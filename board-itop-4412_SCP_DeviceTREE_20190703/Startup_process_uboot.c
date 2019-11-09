/*
uboot版本：2017.11
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
------------------------------------
执行u-boot下的build.sh即可编译uboot。我们先来看看这个文件
cd ../u-boot-2017.11/

if [ ! -f .config ]
then
	make itop4412_defconfig
fi
.......
先来说一下这个uboot的配置过程：
	这个版本的uboot由两部分配置组成，一个是uboot/configs/itop4412_deconfig,另一个是uboot/included/configs/itop4412.h  **
这两个文件有什么不同？最大的不同就是"configs/boardname_defconfig"中的选项都可以在make menuconfig中进
行配置,而"include/configs/boardname.h"中的选项是与开发板相关的一些特性,在make menuconfig中是找不到
这些选项的。个人理解uboot/configs/itop-4412_deconfig也就是menuconfig中设置的CONFIG是跟各目录下的Kconfig和
Makefile以及uboot源代码中都相关的都可以使能的，而include/configs/boardname.h中设置的CONFIG仅仅是和uboot
源代码相关的。（之前S3c_2440的uboot配置是一起的，这个版本是分开的。）

从configs/itop4412_deconfig开始看：
CONFIG_ARM=y
CONFIG_ARCH_EXYNOS=y
	In--uboot/arch/arm/mach-exynos/Kconfig:
		if ARCH_EXYNOS........endif    定义后可以有效这个Kconfig的代码
	In--uboot/arch/arm/Kconfig:
		config ARCH_EXYNOS		使能此配置
		bool "Samsung EXYNOS"
		select DM							选中定义了DM，也就是DM驱动模式
		select DM_I2C					选中定义DM_I2C
		select DM_SPI_FLASH			
		select DM_SERIAL
		select DM_SPI
		select DM_GPIO
		select DM_KEYBOARD
		imply FAT_WRITE
CONFIG_ARCH_EXYNOS4=y
	In--uboot/arch/arm/mach-exynos/Kconfig:
		config ARCH_EXYNOS4		使能此配置
		bool "Exynos4 SoC family"
		select CPU_V7					选中定义CPU_V7，后面有用
		select BOARD_EARLY_INIT_F
		help
	  Samsung Exynos4 SoC family are based on ARM Cortex-A9 CPU. There
	  are multiple SoCs in this family including Exynos4210, Exynos4412,
	  and Exynos4212.
CONFIG_TARGET_ITOP4412=y
	In--uboot/arch/arm/mach-exynos/Kconfig:
		config TARGET_ITOP4412		使能此配置
		bool "Exynos4412 iTop-4412 board"
		select SUPPORT_SPL				选中定义SUPPORT_SPL
	In--uboot/board/samsung/itop4412/Kconfig:
		if TARGET_ITOP4412.......endif   定义后可以有效这个Kconfig的代码
CONFIG_SPL_GPIO_SUPPORT=y
CONFIG_SPL_SERIAL_SUPPORT=y
CONFIG_IDENT_STRING=" for itop-4412"
CONFIG_DEFAULT_DEVICE_TREE="exynos4412-itop4412"
CONFIG_DEBUG_UART=y
CONFIG_SD_BOOT=y
CONFIG_BOOTDELAY=5
CONFIG_SYS_CONSOLE_IS_IN_ENV=y
CONFIG_SYS_CONSOLE_INFO_QUIET=y
CONFIG_SPL=y
CONFIG_HUSH_PARSER=y
CONFIG_SYS_PROMPT="u-boot # "
CONFIG_FASTBOOT=y
CONFIG_FASTBOOT_BUF_ADDR=0x70000000
CONFIG_FASTBOOT_BUF_SIZE=0x100000
CONFIG_FASTBOOT_FLASH=y
CONFIG_FASTBOOT_FLASH_MMC_DEV=0
...............
--------------------------------------------------------------------------------------
其他Kconfig中的config配置
--------------------------------------------------------------------------------------
In--uboot/arch/arm/Kconfig:
config SYS_ARCH
	default "arm"
config SYS_CPU
	default "arm720t" if CPU_ARM720T
	default "arm920t" if CPU_ARM920T
	default "arm926ejs" if CPU_ARM926EJS
	default "arm946es" if CPU_ARM946ES
	default "arm1136" if CPU_ARM1136
	default "arm1176" if CPU_ARM1176
	default "armv7" if CPU_V7		之前使能了CPU_V7所以SYS_CPU为"armv7"
	default "armv7m" if CPU_V7M
	default "pxa" if CPU_PXA
	default "sa1100" if CPU_SA1100
	default "armv8" if ARM64
config SYS_ARM_ARCH
	int
	default 4 if CPU_ARM720T
	default 4 if CPU_ARM920T
	default 5 if CPU_ARM926EJS
	default 5 if CPU_ARM946ES
	default 6 if CPU_ARM1136
	default 6 if CPU_ARM1176
	default 7 if CPU_V7					之前使能了CPU_V7所以SYS_ARM_ARCH为"7"
	default 7 if CPU_V7M
	default 5 if CPU_PXA
	default 4 if CPU_SA1100
	default 8 if ARM64
	
In--uboot/board/samsung/itop4412/Kconfig:
if TARGET_ITOP4412

config SYS_BOARD
	default "itop4412"

config SYS_VENDOR
	default "samsung"

config SYS_CONFIG_NAME
	default "itop4412"

endif
-----------------------------主体uboot脚本选配----------------------------------------------------------------------
查看顶层Makefile中定义的定义的链接脚本的位置在哪：
u-boot.lds: $(LDSCRIPT) prepare FORCE
	$(call if_changed_dep,cpp_lds)
	
# If board code explicitly specified LDSCRIPT or CONFIG_SYS_LDSCRIPT, use
# that (or fail if absent).  Otherwise, search for a linker script in a
# standard location.

ifndef LDSCRIPT
	#LDSCRIPT := $(srctree)/board/$(BOARDDIR)/u-boot.lds.debug
	ifdef CONFIG_SYS_LDSCRIPT						#未定义，跳过
		# need to strip off double quotes
		LDSCRIPT := $(srctree)/$(CONFIG_SYS_LDSCRIPT:"%"=%)
	endif
endif

# If there is no specified link script, we look in a number of places for it
ifndef LDSCRIPT
	ifeq ($(wildcard $(LDSCRIPT)),)
		#//$(BOARDDIR)在config.mk中BOARDDIR = $(VENDOR)/$(BOARD)，根据前面的定义BOARDDIR=samsung/itop4412
		#//LDSCRIPT:=/board/samsung/itop4412/u-boot.lds  无此文件
		LDSCRIPT := $(srctree)/board/$(BOARDDIR)/u-boot.lds
	endif
	ifeq ($(wildcard $(LDSCRIPT)),)
		#//$(CPUDIR)在config.mk中CPUDIR=arch/$(ARCH)/cpu$(if $(CPU),/$(CPU),)，根据前面的定义CPUDIR=arch/arm/cpu/armv7
		#//LDSCRIPT :=/arch/arm/cpu/armv7/u-boot.lds   无此文件
		LDSCRIPT := $(srctree)/$(CPUDIR)/u-boot.lds
	endif
	ifeq ($(wildcard $(LDSCRIPT)),)
		#//LDSCRIPT := /arch/arm/cpu/u-boot.lds						这个就是主uboot的链接脚本
		LDSCRIPT := $(srctree)/arch/$(ARCH)/cpu/u-boot.lds
	endif
endif
---------------------------------
查看/arch/arm/cpu/u-boot.lds:* /
#include <config.h>
#include <asm/psci.h>

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
#ifndef CONFIG_CMDLINE
	/DISCARD/ : { *(.u_boot_list_2_cmd_*) }
#endif
#if defined(CONFIG_ARMV7_SECURE_BASE) && defined(CONFIG_ARMV7_NONSEC)
	/ *
	 * If CONFIG_ARMV7_SECURE_BASE is true, secure code will not
	 * bundle with u-boot, and code offsets are fixed. Secure zone
	 * only needs to be copied from the loading address to
	 * CONFIG_ARMV7_SECURE_BASE, which is the linking and running
	 * address for secure code.
	 *
	 * If CONFIG_ARMV7_SECURE_BASE is undefined, the secure zone will
	 * be included in u-boot address space, and some absolute address
	 * were used in secure code. The absolute addresses of the secure
	 * code also needs to be relocated along with the accompanying u-boot
	 * code.
	 *
	 * So DISCARD is only for CONFIG_ARMV7_SECURE_BASE.
	* / 
	/DISCARD/ : { *(.rel._secure*) }
#endif
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
/ *--------------------------------
对于任何程序，入口函数是在链接时决定的，uboot的入口是由链接脚本决定的。根据前面的分析uboot
链接脚本目录为arch/arm/cpu/u-boot.lds（这个可以在配置文件中与CONFIG_SYS_LDSCRIPT来指定。）
或者在
入口地址也是由连接器决定的，在配置文件中可以由CONFIG_SYS_TEXT_BASE指定。这个会在编译时加在ld连接器的选项-Ttext中
board/samsung/itop4412/Kconfig 里面的内容
if TARGET_ITOP4412

config SYS_BOARD
	default "itop4412"

config SYS_VENDOR
	default "samsung"

config SYS_CONFIG_NAME
	default "itop4412"

endif

将board/samsung/itop4412/Kconfig 添加到arch/arm/mach-exynos/Kconfig
添加如下内容：
source "board/samsung/itop4412/Kconfig"

在最后面endmenu 之前添加。
-------------------------spl uboot脚本选配-----------------------------------------------------------------------------------
查看顶层Makefile中定义的定义的spl链接脚本的位置在哪：
	all:		$(ALL-y) cfg
	ALL-$(CONFIG_SPL) += spl/u-boot-spl.bin
	## 当配置了CONFIG_SPL，make的时候就会执行spl/u-boot-spl.bin这个目标
	
	spl/u-boot-spl.bin: spl/u-boot-spl
	@:
	spl/u-boot-spl: tools prepare \
		$(if $(CONFIG_OF_SEPARATE)$(CONFIG_SPL_OF_PLATDATA),dts/dt.dtb) \
		$(if $(CONFIG_OF_SEPARATE)$(CONFIG_TPL_OF_PLATDATA),dts/dt.dtb)
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
# First test whether there's a linker-script for the specific stage defined...
ifneq ($(CONFIG_$(SPL_TPL_)LDSCRIPT),)
# need to strip off double quotes
LDSCRIPT := $(addprefix $(srctree)/,$(CONFIG_$(SPL_TPL_)LDSCRIPT:"%"=%))
else
# ...then fall back to the generic SPL linker-script
ifneq ($(CONFIG_SPL_LDSCRIPT),)
# need to strip off double quotes
LDSCRIPT := $(addprefix $(srctree)/,$(CONFIG_SPL_LDSCRIPT:"%"=%))
endif
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
我们在配置文件arch/arm/mach-exynos/Kconfig里,在最后一句又添加了这个：
config SPL_LDSCRIPT
	default "board/samsung/common/exynos-uboot-spl.lds" if ARCH_EXYNOS5 || ARCH_EXYNOS4
	设定默认值，如果ARCH_EXYNOS5或ARCH_EXYNOS4一个定义了，那么就默认为default值
	如果在前面定义了CONFIG_ARCH_EXYNOS4=y，所以CONFIG_SPL_LDSCRIPT="board/samsung/common/exynos-uboot-spl.lds"
所以前面Linker Script后一句有效
综上，最终指定了project-X/u-boot/board/samsung/common/exynos-uboot-spl.lds作为连接脚本。

SPL复用的是uboot里面的代码.那要生成我们所需要的SPL目标文件，我们又该如何下手呢？很容易想到，
通过编译选项便可以将SPL和uboot代码分离、复用。这里所说的编译选项便是CONFIG_SPL_BUILD，在
make Kconfig的时候使能。最终编译生成的SPL二进制文件有u-boot-spl，u-boot-spl.bin以及u-boot-spl.map
CONFIG_SPL_TEXT_BASE    SPL的入口地址
在配置文件itop4412.h中
#define CONFIG_SPL_TEXT_BASE	0x02023400 / * 0x02021410 * /
IN	project-x/u-boot/scripts/Makefile.spl
	# First try the best-match (i.e. SPL_TEXT_BASE for SPL, TPL_TEXT_BASE for TPL)
	ifneq ($(CONFIG_$(SPL_TPL_)TEXT_BASE),)
	LDFLAGS_$(SPL_BIN) += -Ttext $(CONFIG_$(SPL_TPL_)TEXT_BASE)
	else
	# And then fall back to just testing for SPL_TEXT_BASE, even if in TPL mode
	ifneq ($(CONFIG_SPL_TEXT_BASE),)
	LDFLAGS_$(SPL_BIN) += -Ttext $(CONFIG_SPL_TEXT_BASE)
	endif
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

	.machine_param : { *(.machine_param) } >.sram
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
#ifdef CONFIG_ARMV7_LPAE
	.global	switch_to_hypervisor_ret
#endif

reset:
	/* Allow the board to save important registers */
	/*注意到 save_boot_params 函数中没有进行任何操作，直接返回了，注释告诉我们栈指针
	还没有初始化，不能向栈中保存任何数据。
	值得注意的是 .weak 伪操作： .weak save_boot_params
	可以理解为如果定义了 save_boot_params 这个函数，那么久调用它，如果没有定义，就定义它，*/
	b	save_boot_params
save_boot_params_ret:
#ifdef CONFIG_ARMV7_LPAE
/*
 * check for Hypervisor support
 */
	mrc	p15, 0, r0, c0, c1, 1		@ read ID_PFR1
	and	r0, r0, #CPUID_ARM_VIRT_MASK	@ mask virtualization bits
	cmp	r0, #(1 << CPUID_ARM_VIRT_SHIFT)
	beq	switch_to_hypervisor
switch_to_hypervisor_ret:
#endif
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
	//uboot/include/autoconf.mk中已经定义了CONFIG_SKIP_LOWLEVEL_INIT=y
	//所以这段代码跳过，直接执行_main函数
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	/*cpu_init_cp15函数是配置cp15协处理器相关寄存器来设置处理器的MMU，cache以及tlb。如果没有
	定义CONFIG_SYS_ICACHE_OFF则会打开icache。关掉mmu以及tlb。具体配置过程可以对照cp15寄存器
	来看，这里不详细说了*/
	bl	cpu_init_cp15
#ifndef CONFIG_SKIP_LOWLEVEL_INIT_ONLY
	/*cpu_init_crit调用的level_init函数与特定开发板相关的初始化函数，在这个函数里会做一些pll
	初始化，如果不是从mem启动，则会做memory初始化，方便后续拷贝到mem中运行（注意：这个开发板
	没有在这里做内存初始化）*/
	bl	cpu_init_crit
#endif
#endif

	bl	_main
/*///////////////////////  crt0.S   //////////////////////// */
ENTRY(_main)
//#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)	//CONFIG_SPL_BUILD在spl里定义了
	//首先将CONFIG_SPL_STACK定义的值加载到栈指针sp中，这个宏定义在配置头文件itop-4412.h中指定。
	ldr	r0, =(CONFIG_SPL_STACK)
//#else
//	ldr	r0, =(CONFIG_SYS_INIT_SP_ADDR)
//#endif
/*接下来为board_init_f C函数调用提供环境，也就是栈指针sp初始化*/
	bic	r0, r0, #7	/* 8-byte alignment for ABI compliance */
	mov	sp, r0
	//堆栈、GD、early malloc空间的分配 
	bl	board_init_f_alloc_reserve
	mov	sp, r0
	/* set up gd here, outside any C code */
	mov	r9, r0
	//堆栈、GD、early malloc空间的分配 
	bl	board_init_f_init_reserve

	mov	r0, #0
	bl	board_init_f
		/*现在在SPL里，由于定义了CONFIG_SPL，也就定义了CONFIG_SPL_BUILD,故调用的是Spl_boot.c*/
		/*
		->spl_boot.c中board_init_f 		
			void board_init_f(unsigned long bootflag)
			{
				__aligned(8) gd_t local_gd;
				__attribute__((noreturn)) void (*uboot)(void);
			
				设置全局指针变量的值
				setup_global_data(&local_gd);
				//如果是唤醒则退出唤醒（这段没有分析太明白，如有错误望指正）
				//do_lowlevel_init()后面分析，很重要！
				if (do_lowlevel_init()) {
					power_exit_wakeup();
				}
				/*
				//Copy U-boot from mmc to RAM 将uboot code拷贝到relocaddr
				copy_uboot_to_ram();->
					//进入函数分析
					unsigned int bootmode = BOOT_MODE_OM;
					u32 (*copy_bl2)(u32 offset, u32 nblock, u32 dst) = NULL;
					u32 offset = 0, size = 0;
					#ifdef CONFIG_SUPPORT_EMMC_BOOT
						u32 (*copy_bl2_from_emmc)(u32 nblock, u32 dst);
						void (*end_bootop_from_emmc)(void);
					#endif
					if (bootmode == BOOT_MODE_OM)
					bootmode = get_boot_mode();
					switch (bootmode) {
					................//省略代码
					#ifndef CONFIG_ITOP4412
					//* just for exynos5 can be call *
							emmc_boot_clk_div_set();
					#endif
							copy_bl2_from_emmc = get_irom_func(EMMC44_INDEX);
							end_bootop_from_emmc = get_irom_func(EMMC44_END_INDEX);
					
							copy_bl2_from_emmc(BL2_SIZE_BLOC_COUNT, CONFIG_SYS_TEXT_BASE);
							end_bootop_from_emmc();
							break;
					}
					if (copy_bl2)
					copy_bl2(offset, size, CONFIG_SYS_TEXT_BASE);
					//copy_uboot_to_ram结束，返回上一层函数			
				/* Jump to U-Boot image 这里就跳到RAM上运行*//*
				uboot = (void *)CONFIG_SYS_TEXT_BASE;
				(*uboot)();
				/*从SPL到U-Boot第二阶段主体uboot代码的跳转执行！！！
				 *翻译成我们所熟悉的汇编跳转的话应该是：ldr pc, =CONFIG_SYS_TEXT_BASE
				 *//*
			}*/
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_FRAMEWORK)
# ifdef CONFIG_SPL_BUILD
	/* Use a DRAM stack for the rest of SPL, if requested */
	bl	spl_relocate_stack_gd
	cmp	r0, #0
	movne	sp, r0
	movne	r9, r0
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
#if defined(CONFIG_CPU_V7M)
	itt	lo
#endif
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
#if CONFIG_IS_ENABLED(SYS_THUMB_BUILD)
	ldr	lr, =board_init_r	/* this is auto-relocated! */
	bx	lr
#else
	ldr	pc, =board_init_r	/* this is auto-relocated! */
	//在SPL中是spl_boot.c中的board_init_r函数，是个空函数死循环
	//当然永远不会运行到这里，因为之前已经跳到第二阶段了
#endif
	/* we should not return here. */
#endif
ENDPROC(_main)				
/*-------------------------------------------------------------------------------------------------
uboot涉及底层初始化的函数分析：arch/arm/mach-exynos/spl_boot.c:do_lowlevel_init()函数
--------------------------------------------------------------------------------------------------*/
int do_lowlevel_init(void)
{
		arch_cpu_init();
		get_reset_status()			//获取复位状态并赋相应的值给变量，
		switch (reset_status)	//然后做出相应的动作：actions = DO_CLOCKS | DO_MEM_RESET | DO_POWER;
		set_ps_hold_ctrl();			//关于供电锁存类PS_HOLD;
		system_clock_init();->		//系统时钟初始化；
			/*
			struct exynos4_clock *clk =
			(struct exynos4_clock *)samsung_get_base_clock();

			writel(CLK_SRC_CPU_VAL, &clk->src_cpu);配置4412 CLK_SRC_CPU寄存器，CMU_CPU有4个MUX，设置这些MUX
		
			sdelay(0x10000);
		
			writel(CLK_SRC_TOP0_VAL, &clk->src_top0);
			writel(CLK_SRC_TOP1_VAL, &clk->src_top1);
			writel(CLK_SRC_DMC_VAL, &clk->src_dmc);
			writel(CLK_SRC_LEFTBUS_VAL, &clk->src_leftbus);
			writel(CLK_SRC_RIGHTBUS_VAL, &clk->src_rightbus);
			writel(CLK_SRC_FSYS_VAL, &clk->src_fsys);
			writel(CLK_SRC_PERIL0_VAL, &clk->src_peril0);
			writel(CLK_SRC_CAM_VAL, &clk->src_cam);
			writel(CLK_SRC_MFC_VAL, &clk->src_mfc);
			writel(CLK_SRC_G3D_VAL, &clk->src_g3d);
			writel(CLK_SRC_LCD0_VAL, &clk->src_lcd0);
		
			sdelay(0x10000);
		
			writel(CLK_DIV_CPU0_VAL, &clk->div_cpu0);
			writel(CLK_DIV_CPU1_VAL, &clk->div_cpu1);
			writel(CLK_DIV_DMC0_VAL, &clk->div_dmc0);
			writel(CLK_DIV_DMC1_VAL, &clk->div_dmc1);
			writel(CLK_DIV_LEFTBUS_VAL, &clk->div_leftbus);
			writel(CLK_DIV_RIGHTBUS_VAL, &clk->div_rightbus);
			writel(CLK_DIV_TOP_VAL, &clk->div_top);
			writel(CLK_DIV_FSYS1_VAL, &clk->div_fsys1);
			writel(CLK_DIV_FSYS2_VAL, &clk->div_fsys2);
			writel(CLK_DIV_FSYS3_VAL, &clk->div_fsys3);
			writel(CLK_DIV_PERIL0_VAL, &clk->div_peril0);
			writel(CLK_DIV_CAM_VAL, &clk->div_cam);
			writel(CLK_DIV_MFC_VAL, &clk->div_mfc);
			writel(CLK_DIV_G3D_VAL, &clk->div_g3d);
			writel(CLK_DIV_LCD0_VAL, &clk->div_lcd0);
		
			// Set PLL locktime 
			writel(PLL_LOCKTIME, &clk->apll_lock);
			writel(PLL_LOCKTIME, &clk->mpll_lock);
			writel(PLL_LOCKTIME, &clk->epll_lock);
			writel(PLL_LOCKTIME, &clk->vpll_lock);
		
			writel(APLL_CON1_VAL, &clk->apll_con1);
			writel(APLL_CON0_VAL, &clk->apll_con0);
			writel(MPLL_CON1_VAL, &clk->mpll_con1);
			writel(MPLL_CON0_VAL, &clk->mpll_con0);
			writel(EPLL_CON1_VAL, &clk->epll_con1);
			writel(EPLL_CON0_VAL, &clk->epll_con0);
			writel(VPLL_CON1_VAL, &clk->vpll_con1);
			writel(VPLL_CON0_VAL, &clk->vpll_con0);
		
			sdelay(0x30000);
			*/
		exynos_pinmux_config(PERIPH_ID_UART2, PINMUX_FLAG_NONE);//端口配置	
		debug_uart_init();->						//测试串口；
		mem_ctrl_init(actions & DO_MEM_RESET);->			//ddr内存初始化；
			/*struct exynos4_dmc *dmc;
				/* Async bridge configuration at CPU_core:
				 * 1: half_sync
				 * 0: full_sync
				 * /
				writel(1, ASYNC_CONFIG);
		
				/* Interleave: 2Bit, Interleave_bit1: 0x15, Interleave_bit0: 0x7 * /
				writel(APB_SFR_INTERLEAVE_CONF_VAL, EXYNOS4_MIU_BASE +
					APB_SFR_INTERLEAVE_CONF_OFFSET);
				/* Update MIU Configuration * /
				writel(APB_SFR_ARBRITATION_CONF_VAL, EXYNOS4_MIU_BASE +
					APB_SFR_ARBRITATION_CONF_OFFSET);
					
			#ifdef CONFIG_MIU_LINEAR
				writel(SLAVE0_SINGLE_ADDR_MAP_START_ADDR, EXYNOS4_MIU_BASE +
					ABP_SFR_SLV0_SINGLE_ADDRMAP_START_OFFSET);
				writel(SLAVE0_SINGLE_ADDR_MAP_END_ADDR, EXYNOS4_MIU_BASE +
					ABP_SFR_SLV0_SINGLE_ADDRMAP_END_OFFSET);
				writel(SLAVE1_SINGLE_ADDR_MAP_START_ADDR, EXYNOS4_MIU_BASE +
					ABP_SFR_SLV1_SINGLE_ADDRMAP_START_OFFSET);
				writel(SLAVE1_SINGLE_ADDR_MAP_END_ADDR, EXYNOS4_MIU_BASE +
					ABP_SFR_SLV1_SINGLE_ADDRMAP_END_OFFSET);
				writel(APB_SFR_SLV_ADDR_MAP_CONF_VAL, EXYNOS4_MIU_BASE +
					ABP_SFR_SLV_ADDRMAP_CONF_OFFSET);
			#endif
			#endif
				// DREX0 *
				dmc = (struct exynos4_dmc *)samsung_get_base_dmc_ctrl();
				dmc_init(dmc);->					
					writel(mem.control1, &dmc->phycontrol1);				
					writel(mem.zqcontrol, &dmc->phyzqcontrol);
					sdelay(0x100000);				
					phy_control_reset(1, dmc);
					phy_control_reset(0, dmc);				
					/* Set DLL Parameters * /
					writel(mem.control1, &dmc->phycontrol1);				
					/* DLL Start * /
					writel((mem.control0 | CTRL_START | CTRL_DLL_ON), &dmc->phycontrol0);				
					writel(mem.control2, &dmc->phycontrol2);				
					/* Set Clock Ratio of Bus clock to Memory Clock * /
					writel(mem.concontrol, &dmc->concontrol);				
					writel(mem.memcontrol, &dmc->memcontrol);
				
					//.memconfig0 = MEMCONFIG0_VAL=(CHIP_BANK_8 | CHIP_ROW_14 | CHIP_COL_10| CHIP_MAP_INTERLEAVED | CHIP_MASK | CHIP0_BASE)=0x40C01333
					//8 banks，row 15bits，col 10bits，chip_base=40表示从40000000开始
					writel(mem.memconfig0, &dmc->memconfig0);
					writel(mem.memconfig1, &dmc->memconfig1);
				
					/* Config Precharge Policy * /
					writel(mem.prechconfig, &dmc->prechconfig);
					
					writel(mem.timingref, &dmc->timingref);
					writel(mem.timingrow, &dmc->timingrow);
					writel(mem.timingdata, &dmc->timingdata);
					writel(mem.timingpower, &dmc->timingpower);
				
					//* Chip0: NOP Command: Assert and Hold CKE to high level *
					writel(DIRECT_CMD_NOP, &dmc->directcmd);
					sdelay(0x100000);
				
					//* Chip0: EMRS2, EMRS3, EMRS, MRS Commands Using Direct Command *
					dmc_config_mrs(dmc, 0);
					sdelay(0x100000);
				
					//* Chip0: ZQINIT *
					writel(DIRECT_CMD_ZQ, &dmc->directcmd);
					sdelay(0x100000);
				
					writel((DIRECT_CMD_NOP | DIRECT_CMD_CHIP1_SHIFT), &dmc->directcmd);
					sdelay(0x100000);
				
					//* Chip1: EMRS2, EMRS3, EMRS, MRS Commands Using Direct Command *
					dmc_config_mrs(dmc, 1);
					sdelay(0x100000);
				
					//* Chip1: ZQINIT *
					writel((DIRECT_CMD_ZQ | DIRECT_CMD_CHIP1_SHIFT), &dmc->directcmd);
					sdelay(0x100000);
				
					phy_control_reset(1, dmc);
					sdelay(0x100000);
				
					// turn on DREX0, DREX1 *
					writel((mem.concontrol | AREF_EN), &dmc->concontrol);
				// DREX1 *
				dmc = (struct exynos4_dmc *)(samsung_get_base_dmc_ctrl()
								+ DMC_OFFSET);
				dmc_init(dmc);					*/ //dmc_init(dmc)END.
		//tzpc_init();					//tzpc这个安全源代码去掉了，原因未知				
}
			
/*------------------------------------------------------------------------------------------------
do_lowlevel_init()函数====>END.
-------------------------------------------------------------------------------------------------*/		
//SPL阶段结束！！！！！！！！！！
/*
分析源代码：再来看主体U-boot部分（第二阶段）
查看u-boot.lds:前面分析是主体uboot链接脚本/arch/arm/cpu/u-boot.lds:
*/
#include <config.h>
#include <asm/psci.h>
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
		//之前CPUDIR定义为arch/arm/cpu/armv7,所以此处为arch/arm/cpu/armv7/start.o
		CPUDIR/start.o (.text*)
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
	. = ALIGN(4);
	.__efi_runtime_start : {
		*(.__efi_runtime_start)
	}
	.efi_runtime : {
		*(efi_runtime_text)
		*(efi_runtime_data)
	}
	.__efi_runtime_stop : {
		*(.__efi_runtime_stop)
	}
	.efi_runtime_rel_start :
	{
		*(.__efi_runtime_rel_start)
	}
	.efi_runtime_rel : {
		*(.relefi_runtime_text)
		*(.relefi_runtime_data)
	}
	.efi_runtime_rel_stop :
	{
		*(.__efi_runtime_rel_stop)
	}
	. = ALIGN(4);
	.image_copy_end :
	{
		*(.__image_copy_end)
	}
	.rel_dyn_start :
	{
		*(.__rel_dyn_start)
	}
	.rel.dyn : {
		*(.rel*)
	}
	.rel_dyn_end :
	{
		*(.__rel_dyn_end)
	}
	.end :
	{
		*(.__end)
	}
	_image_binary_end = .;
	. = ALIGN(4096);
	.mmutable : {
		*(.mmutable)
	}
	.bss_start __rel_dyn_start (OVERLAY) : {
		KEEP(*(.__bss_start));
		__bss_base = .;
	}
	.bss __bss_base (OVERLAY) : {
		*(.bss*)
		 . = ALIGN(4);
		 __bss_limit = .;
	}
	.bss_end __bss_limit (OVERLAY) : {
		KEEP(*(.__bss_end));
	}
	.dynsym _image_binary_end : { *(.dynsym) }
	.dynbss : { *(.dynbss) }
	.dynstr : { *(.dynstr*) }
	.dynamic : { *(.dynamic*) }
	.plt : { *(.plt*) }
	.interp : { *(.interp*) }
	.gnu.hash : { *(.gnu.hash) }
	.gnu : { *(.gnu*) }
	.ARM.exidx : { *(.ARM.exidx*) }
	.gnu.linkonce.armexidx : { *(.gnu.linkonce.armexidx.*) }
}
/*
链接脚本中这些宏的定义在linkage.h中，看字面意思也明白，程序的入口是在_start.，后面是text段，data段等。
_start在arch/arm/cpu/armv7/start.S中，一段一段的分析，如下：
*/
/*************************************************************************
 *
 * Startup Code (reset vector)
 *
 * Do important init only if we don't start from memory!
 * Setup memory and board specific bits prior to relocation.
 * Relocate armboot to ram. Setup stack.
 *
 *************************************************************************/
/*.global相当于C语言中的Extern，声明此变量，并告诉链接器此变量是全局，外部可以访问*/
	.globl	reset
	.globl	save_boot_params_ret
#ifdef CONFIG_ARMV7_LPAE
	.global	switch_to_hypervisor_ret
#endif

reset:
	/* Allow the board to save important registers */
	/*注意到 save_boot_params 函数中没有进行任何操作，直接返回了，注释告诉我们栈指针
	还没有初始化，不能向栈中保存任何数据。
	值得注意的是 .weak 伪操作： .weak save_boot_params
	可以理解为如果定义了 save_boot_params 这个函数，那么久调用它，如果没有定义，就定义它，*/
	b	save_boot_params
save_boot_params_ret:
#ifdef CONFIG_ARMV7_LPAE
/*
 * check for Hypervisor support
 */
	mrc	p15, 0, r0, c0, c1, 1		@ read ID_PFR1
	and	r0, r0, #CPUID_ARM_VIRT_MASK	@ mask virtualization bits
	cmp	r0, #(1 << CPUID_ARM_VIRT_SHIFT)
	beq	switch_to_hypervisor
switch_to_hypervisor_ret:
#endif
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
	//uboot/include/autoconf.mk中已经定义了CONFIG_SKIP_LOWLEVEL_INIT=y
	//所以这段代码跳过，直接执行_main函数
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	/*cpu_init_cp15函数是配置cp15协处理器相关寄存器来设置处理器的MMU，cache以及tlb。如果没有
	定义CONFIG_SYS_ICACHE_OFF则会打开icache。关掉mmu以及tlb。具体配置过程可以对照cp15寄存器
	来看，这里不详细说了*/
	bl	cpu_init_cp15
#ifndef CONFIG_SKIP_LOWLEVEL_INIT_ONLY
	/*cpu_init_crit调用的level_init函数与特定开发板相关的初始化函数，在这个函数里会做一些pll
	初始化，如果不是从mem启动，则会做memory初始化，方便后续拷贝到mem中运行（注意：这个开发板
	没有在这里做内存初始化）*/
	bl	cpu_init_crit
#endif
#endif
	bl	_main
----------------------------------------------------------------------
//_main函数在arch/arm/lib/crt0.S中，mian函数的作用在注释中有详细的说明 
/*///////////////////////  crt0.S   //////////////////////// */
ENTRY(_main)
/*
 * Set up initial C runtime environment and call board_init_f(0).
 */

//#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)
//	ldr	r0, =(CONFIG_SPL_STACK)
//#else
	/*首先将CONFIG_SYS_INIT_SP_ADDR定义的值加载到栈指针sp中，这个宏定义在配置头文件itop-4412.h中指定。
	CONFIG_SYS_TEXT_BASE+UBOOT_SIZE-0x1000*/
	ldr	r0, =(CONFIG_SYS_INIT_SP_ADDR)
//#endif
	/*接下来为board_init_f C函数调用提供环境，也就是栈指针sp初始化*/
	bic	r0, r0, #7	/* 8-byte alignment for ABI compliance */
	mov	sp, r0
	//堆栈、GD、early malloc空间的分配 
	bl	board_init_f_alloc_reserve
	mov	sp, r0
	/* set up gd here, outside any C code */
	mov	r9, r0
	//堆栈、GD、early malloc空间的初始化 
	bl	board_init_f_init_reserve

	mov	r0, #0
	bl	board_init_f
		/*现在在主体uboot中，未使能CONFIG_SPL_BUILD,调用的是Board_f.c (common)*/
		/*
		->board_f.c中board_init_f
			void board_init_f(ulong boot_flags)
			{
				gd->flags = boot_flags;
				gd->have_console = 0;
			
				if (initcall_run_list(init_sequence_f))
					hang();
			
			#if !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
					!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64)
				/ * NOTREACHED - jump_to_copy() does not return * /
				hang();
			#endif
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
					//.创建根设备root的udevice，存放在gd->dm_root中。
					//.根设备其实是一个虚拟设备，主要是为uboot的其他设备提供一个挂载点。
					//.初始化uclass链表gd->uclass_root
					dm_init	->										//将根节点绑定到gd->dm_root上，初始化根节点设备
						//DM_ROOT_NON_CONST是指根设备udevice，root_info是表示根设备的设备信息
        		//device_bind_by_name会查找和设备信息匹配的driver，然后创建对应的udevice和uclass并进行绑定，最后放在DM_ROOT_NON_CONST中。
        		//device_bind_by_name后续我们会进行说明，这里我们暂时只需要了解root根设备的udevice以及对应的uclass都已经创建完成。
						device_bind_by_name(NULL, false, &root_info, &DM_ROOT_NON_CONST);
							lists_driver_lookup_name(info->name);		//从driver table中获取名字为name的driver。
							device_bind_common ->	//匹配设备和驱动，并将设备节点和parent节点建立联系，也就是建立树形结构
								uclass_bind_device	//将该设备挂在对应的U_CLASS链表上
								drv->bind(dev)			//设备驱动的bind接口函数
	              parent->driver->child_post_bind(dev)//父节点驱动的child_post_bind接口函数
	              uc->uc_drv->post_bind//设备所属类的驱动的post_bind接口函数（具体的设备节点就是在这个接口下在soc下进行展开的）
	          //对根设备执行probe操作，device_probe后续再进行说明
						device_probe(DM_ROOT_NON_CONST);//device_probe(dev)进行设备的激活和驱动的注册，这里是根设备
					dm_scan_platdata ->						//从平台设备中解析udevice和uclass,搜索使用宏U_BOOT_DEVICE定义的设备进行驱动匹配，也就是bind子节点
					dm_extended_scan_fdt->
						dm_scan_fdt	->							//在设备树种搜索设备并进行驱动匹配，然后bind
							dm_scan_fdt_node	//具体绑定设备的入口，在该函数中会确定设备是否具有boot,dm-pre-reloc属性，如果没有则不会绑定
		          lists_bind_fdt		//搜索可以匹配到该设备的驱动
		            device_bind_with_driver_data ->	//如果匹配到进行绑定
		            	device_bind_common ->		//匹配设备和驱动，并将设备节点和parent节点建立联系，也就是建立树形结构	
	                  uclass_bind_device//将该设备挂在对应的U_CLASS链表上
	                  drv->bind(dev)//设备驱动的bind接口函数
	                  parent->driver->child_post_bind(dev)//父节点驱动的child_post_bind接口函数
	                  uc->uc_drv->post_bind//设备所属类的驱动的post_bind接口函数（具体的设备节点就是在这个接口下在soc下进行展开的）
						ofnode_to_offset(ofnode_path("/clocks"));			//绑定固定的clocks
						dm_scan_fdt_node		//具体绑定设备的入口，在该函数中会确定设备是否具有boot,dm-pre-reloc属性，如果没有则不会绑定
					dm_scan_other	->		
		
			env_init,//仅仅把环境参数的地址赋值
			init_baud_rate,	//设置波特率
			setup_machine,//设置机器ID
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
	ldr	r0, [r9, #GD_START_ADDR_SP]	/* sp = gd->start_addr_sp */
	bic	r0, r0, #7	/* 8-byte alignment for ABI compliance */
	mov	sp, r0
	ldr	r9, [r9, #GD_BD]		/* r9 = gd->bd */
	sub	r9, r9, #GD_SIZE		/* new GD is below bd */

	adr	lr, here
	ldr	r0, [r9, #GD_RELOC_OFF]		/* r0 = gd->reloc_off */
	add	lr, lr, r0
#if defined(CONFIG_CPU_V7M)
	orr	lr, #1				/* As required by Thumb-only */
#endif
	ldr	r0, [r9, #GD_RELOCADDR]		/* r0 = gd->relocaddr */
	/*将uboot code拷贝到relocaddr，注意这里没有用到！*/
	b	relocate_code
		/*
		ENTRY(relocate_code)
			ldr	r1, =__image_copy_start	/* r1 <- SRC &__image_copy_start *
			subs	r4, r0, r1		/* r4 <- relocation offset *
			注意：这里判断是否运行在sdram上，也就是是否重定位完，如果重定位完就直接跳到后面relocate_done函数
			这里我们之前已经copy_uboot_to_ram();在CONFIG_SYS_TEXT_BASE上运行，所以跳转
			beq	relocate_done		/* skip relocation *
			ldr	r2, =__image_copy_end	/* r2 <- SRC &__image_copy_end *		
		copy_loop:
			ldmia	r1!, {r10-r11}		/* copy from source address [r1]    *
			stmia	r0!, {r10-r11}		/* copy to   target address [r0]    *
			cmp	r1, r2			/* until source end address [r2]    *
			blo	copy_loop		
			/*
			 * fix .rel.dyn relocations
			 *
			ldr	r2, =__rel_dyn_start	/* r2 <- SRC &__rel_dyn_start *
			ldr	r3, =__rel_dyn_end	/* r3 <- SRC &__rel_dyn_end *
		fixloop:
			ldmia	r2!, {r0-r1}		/* (r0,r1) <- (SRC location,fixup) *
			and	r1, r1, #0xff
			cmp	r1, #R_ARM_RELATIVE
			bne	fixnext		
			/* relative fix: increase location by offset *
			add	r0, r0, r4
			ldr	r1, [r0]
			add	r1, r1, r4
			str	r1, [r0]
		fixnext:
			cmp	r2, r3
			blo	fixloop
		
		relocate_done:				跳转到这里		
		#ifdef __XSCALE__					未定义，跳过
			/*
			 * On xscale, icache must be invalidated and write buffers drained,
			 * even with cache disabled - 4.2.7 of xscale core developer's manual
			 *
			mcr	p15, 0, r0, c7, c7, 0	/* invalidate icache *
			mcr	p15, 0, r0, c7, c10, 4	/* drain write buffer *
		#endif		
			/* ARMv4- don't know bx lr but the assembler fails to see that *		
		#ifdef __ARM_ARCH_4__			未定义，执行else
			mov	pc, lr
		#else
			bx	lr					BX指令是ARM指令系统中的带状态切换跳转指令，跳转到lr地址，lr之前被赋值到here函数的地方，所以跳转到here
		#endif		
		ENDPROC(relocate_code)
		*/
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
	movne	r9, r0
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
#if defined(CONFIG_CPU_V7M)
	itt	lo
#endif
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
#if CONFIG_IS_ENABLED(SYS_THUMB_BUILD)
	ldr	lr, =board_init_r	/* this is auto-relocated! */
	bx	lr
#else
	/* call board_init_r */
	/*现在是主体uboot，所以调用的是Board_r.c (common)这个文件中的函数*/
	//接下来在后面我们分析board_init_r函数，这个是最重要的函数！
	ldr	pc, =board_init_r	/* this is auto-relocated! */
	
	/*如果icache是enable，则无效掉icache，保证从sdram中更新指令到cache中。
	接着更新异常向量表首地址，因为code被relocate，所以异常向量表也被relocate。

	从c_runtime_cpu_setup返回，下面一段汇编是将bss段清空。

	接下来分别调用了coloured_LED_init以及red_led_on,很多开发板都会有led指示灯，
	这里可以实现上电指示灯亮，有调试作用。

	最后r0赋值gd指针，r1赋值relocaddr，进入最后的board_init_r !*/
#endif
	/* we should not return here. */
#endif

ENDPROC(_main)
		
/*///////////////////////  board_r.c   //////////////////////// */
void board_init_r(gd_t *new_gd, ulong dest_addr)
{
	/*
	 * Set up the new global data pointer. So far only x86 does this
	 * here.
	 * TODO(sjg@chromium.org): Consider doing this for all archs, or
	 * dropping the new_gd parameter.
	 */
#if CONFIG_IS_ENABLED(X86_64)
	arch_setup_gd(new_gd);
#endif

#ifdef CONFIG_NEEDS_MANUAL_RELOC
	int i;
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
	
	
	/initr_dm,
	initr_mmc,
	//initr_dm，initr_mmc函数涉及比较多，放到后面分析
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
			////在itop4412.h配置CONFIG_BOOTCOMMAND参数
			#ifdef	CONFIG_BOOTCOMMAND
				"bootcmd="	CONFIG_BOOTCOMMAND		"\0"
			#endif
			....................
			//在itop4412.h配置CONFIG_EXTRA_ENV_SETTINGS了很多参数
			//CONFIG_EXTRA_ENV_SETTINGS和CONFIG_BOOTCOMMAND这两个宏的意思就是把你手工在console上敲的代码集合在一起。
			//比如 CONFIG_BOOTCOMMAND  的效果就是在uboot的console上一行一行把uboot命令敲进去（去掉“”和\），一直到
			//run netboot这条命令。这些都是uboot的命令或uboot环境设置。
			#ifdef	CONFIG_EXTRA_ENV_SETTINGS
				CONFIG_EXTRA_ENV_SETTINGS
			#endif
			}
	.................
	最后进入run_main_loop不再返回*/
	if (initcall_run_list(init_sequence_r))
		hang();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
/*-------------------------------------------------------------------------------------------------
uboot涉及设备树的两个函数分析：/common/board_r.c:init_sequence_r[]中initr_dm，initr_mmc函数
--------------------------------------------------------------------------------------------------*/
	//接下来这两个函数就和uboot的设备树有关，新版uboot加入了设备树和DM驱动模型（driver model）
	//和linux一样，u-boot这里也建立了一个驱动模型。比如， of_match来匹配，probe来识别等。
	#ifdef CONFIG_DM
		initr_dm,->									//执行bind操作，初始化一个dm模型的树形结构
			/*
				dm初始化的接口在dm_init_and_scan中。 可以发现在uboot relocate之前的initf_dm和之后
			的initr_dm都调用了这个函数。主要区别在于参数。首先说明一下dts节点中的“u-boot,dm-pre-reloc”
			属性，当设置了这个属性时，则表示这个设备在relocate之前就需要使用。当dm_init_and_scan的参
			数为true时，只会对带有“u-boot,dm-pre-reloc”属性的节点进行解析。而当参数为false的时候，
			则会对所有节点都进行解析。“u-boot,dm-pre-reloc”的情况比较少			*/
			dm_init_and_scan(false);
				.......................//(这里和之前调用的是同一个函数，不再重复解析)
				/* 
					在init_sequence_r中的initr_dm中，完成了FDT的解析，解析了所有的外设node,并针对各个
				节点进行了 udevice和uclass的创建，以及各个组成部分的绑定关系。注意，这里只是绑定，
				即调用了driver的bind函数，但是设备还没有真正激活，也就是还没有执行设备的probe函数。
				将在具体xxx设备初始化阶段进行相关设备的激活和驱动的注册
					在U_Boot DM 初始化阶段(initf_dm 和 initr_dm)，通过调用 dm_init_and_scan(bool pre_reloc_only)
				根据名称 (U_BOOT_DEVICE 中和 driver 的 name，或者 dts 和 driver 的 compatible) 匹配到对应的
				driver，然后调用device_bind_common 函数生成 udevice，udevice 会和 driver 绑定，并根据
				driver 中的uclass id 找到对应的 uclass driver，并生成相应的 uclass， 并把该设备挂到 
				uclass 的设备节点之下。最后调用 driver 的 bind 函数。
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
				*/

	#endif
	#ifdef CONFIG_MMC
		initr_mmc,->						///初始化eMMC和SD卡，分析这个函数 
				mmc_initialize(gd->bd);->
					mmc_probe(bis);->			//这里有两个选择，一种是DM驱动模型，另一种是直接OF_CONTROL,这里选择的是前者
						uclass_get(UCLASS_MMC, &uc);		//从gd->uclass_root链表获取对应的uclass
						uclass_get_device_by_seq(UCLASS_MMC, i, &dev);	//通过MMC号按顺序从uclass中获取udevice
						uclass_foreach_dev(dev, uc)			//遍历 probe uclass下所有的udevice（控制器）
						device_probe(dev);	//设备的激活和驱动的注册，实现过程在前面有解
					mmc_do_preinit();
						mmc_start_init()		//初始化MMC，以及发送CMD0,CMD2...设置mmc
	#endif
	/*
	看一下exynos4412里面的eMMC驱动：drivers/mmc/exynos_dw_mmc.c
		static const struct udevice_id exynos_dwmmc_ids[] = {
		{ .compatible = "samsung,exynos4412-dw-mshc" },
		{ }
	};
		U_BOOT_DRIVER(exynos_dwmmc_drv) = {
		.name		= "exynos_dwmmc",
		.id		= UCLASS_MMC,
		.of_match	= exynos_dwmmc_ids,
		.bind		= exynos_dwmmc_bind,
		.ops		= &dm_dwmci_ops,
		.probe		= exynos_dwmmc_probe,
		.priv_auto_alloc_size	= sizeof(struct dwmci_exynos_priv_data),
		.platdata_auto_alloc_size = sizeof(struct exynos_mmc_plat),
	};
	驱动以{ .compatible = "samsung,exynos4412-dw-mshc" },来匹配的，再看下设备树里面
	mshc_0: dwmmc@12550000 {
		#address-cells = <1>;
		#size-cells = <0>;
		compatible = "samsung,exynos4412-dw-mshc";
		reg = <0x12550000 0x1000>;
		interrupts = <0 131 0>;
		status = "disabled";
	};
	&mshc_0 {
		samsung,bus-width = <4>;
		samsung,timing = <2 1 0>;
		fifoth_val = <0x203f0040>;
		bus_hz = <400000000>;
		div = <0x3>;
		index = <4>;
		status = "okay";
	};
	SD卡驱动：drivers/mmc/s5p_sdhci.c
	sdhci2: sdhci@12530000 {
		#address-cells = <1>;
		#size-cells = <0>;
		compatible = "samsung,exynos4412-sdhci";
		reg = <0x12530000 0x1000>;
		interrupts = <0 77 0>;
		status = "disabled";
	};
	&sdhci2 {
	samsung,bus-width = <4>;
		//cd-gpios = <&gpx0 7 0>; *
		cd-gpios = <&gpk2 2 0>;
		status = "okay";
	};
	设备驱动模型匹配流程参见之前的initr_mmc流程
	*/
/*
有关device_probe()的DM实现函数在后面附录有详解
*/
/*------------------------------------------------------------------------------------------------
initr_dm，initr_mmc函数====>END.
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

*这里顺便提一下itop4412.h中的参数CONFIG_EXTRA_ENV_SETTINGS，其中定义了很多的参数
包括：
	loadaddr=0x40007000\0" \
	"rdaddr=0x48000000\0" \
	"kerneladdr=0x40007000\0" \
	"ramdiskaddr=0x48000000\0" \
	"console=ttySAC2,115200n8\0" \
	"mmcdev=0\0" \
	"bootenv=uEnv.txt\0" \
	"dtb_addr=0x41000000\0" \
	"dtb_name=exynos4412-itop-elite.dtb\0" \
	"loadbootenv=load mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"bootargs=root=/dev/mmcblk1p2 rw console=ttySAC2,115200 init=/linuxrc earlyprintk\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
	"env import -t $loadaddr $filesize\0" \
    "loadbootscript=load mmc ${mmcdev} ${loadaddr} boot.scr\0" \
    "bootscript=echo Running bootscript from mmc${mmcdev} ...; " \
    "source ${loadaddr}\0"
加载地址、内核在内存存放的地址、终端波特率、bootargs等等
猜测eMMC分区也会用到这些参数。****************************************************
继续回来看：
bootm:./cmd/bootm.c
U_BOOT_CMD(
	bootm,	CONFIG_SYS_MAXARGS,	1,	do_bootm,
	"boot application image from memory", bootm_help_text
);

uboot命令中，bootm对应的函数为do_bootm(),在./cmd/bootm.c;
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
int device_probe(struct udevice *dev)
{
    const struct driver *drv;
    int size = 0;
    int ret;
    int seq;

    if (dev->flags & DM_FLAG_ACTIVATED)
        return 0;
// 表示这个设备已经被激活了
    drv = dev->driver;
    assert(drv);
// 获取这个设备对应的driver
    /* Allocate private data if requested and not reentered */
    if (drv->priv_auto_alloc_size && !dev->priv) {
        dev->priv = alloc_priv(drv->priv_auto_alloc_size, drv->flags);
// 为设备分配私有数据
    }
    /* Allocate private data if requested and not reentered */
    size = dev->uclass->uc_drv->per_device_auto_alloc_size;
    if (size && !dev->uclass_priv) {
        dev->uclass_priv = calloc(1, size);
// 为设备所属uclass分配私有数据
    }

// 这里过滤父设备的probe
    seq = uclass_resolve_seq(dev);
    if (seq < 0) {
        ret = seq;
        goto fail;
    }
    dev->seq = seq;
    dev->flags |= DM_FLAG_ACTIVATED;
// 设置udevice的激活标志
    ret = uclass_pre_probe_device(dev);
// uclass在probe device之前的一些函数的调用
    if (drv->ofdata_to_platdata && dev->of_offset >= 0) {
        ret = drv->ofdata_to_platdata(dev);
// 调用driver中的ofdata_to_platdata将dts信息转化为设备的平台数据
    }
    if (drv->probe) {
        ret = drv->probe(dev);
// 调用driver的probe函数，到这里设备才真正激活了
    }
    ret = uclass_post_probe_device(dev);
    return ret;
}
/*主要工作归纳如下：

.分配设备的私有数据

.对父设备进行probe

.执行probe device之前uclass需要调用的一些函数

.调用driver的ofdata_to_platdata，将dts信息转化为设备的平台数据（重要）

.调用driver的probe函数（重要）

.执行probe device之后uclass需要调用的一些函数
*/















































