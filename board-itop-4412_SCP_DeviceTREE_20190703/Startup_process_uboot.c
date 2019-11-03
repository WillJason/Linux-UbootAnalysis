/*
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
		select DM
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
--------------------------------------------------------------------------------
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
		#//LDSCRIPT := /arch/arm/cpu/u-boot.lds
		LDSCRIPT := $(srctree)/arch/$(ARCH)/cpu/u-boot.lds
	endif
endif
---------------------------------
查看/arch/arm/cpu/u-boot.lds:
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
	/*
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
--------------------------------
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

将board/samsung/itop4412/Kconfig 添加到arch/arm/cpu/armv7/exynos/Kconfig
添加如下内容：
source "board/samsung/itop4412/Kconfig"
又添加了这个：
config SPL_LDSCRIPT
	default "board/samsung/common/exynos-uboot-spl.lds" if ARCH_EXYNOS5 || ARCH_EXYNOS4
	在前面定义了CONFIG_ARCH_EXYNOS4=y，所以CONFIG_SPL_LDSCRIPT="board/samsung/common/exynos-uboot-spl.lds"
在最后面endmenu 之前添加。

查看u-boot.lds
*/
















