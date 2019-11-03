/*
ִ��u-boot�µ�build.sh���ɱ���uboot������������������ļ�
cd ../u-boot-2017.11/

if [ ! -f .config ]
then
	make itop4412_defconfig
fi
.......
����˵һ�����uboot�����ù��̣�
	����汾��uboot��������������ɣ�һ����uboot/configs/itop4412_deconfig,��һ����uboot/included/configs/itop4412.h  **
�������ļ���ʲô��ͬ�����Ĳ�ͬ����"configs/boardname_defconfig"�е�ѡ�������make menuconfig�н�
������,��"include/configs/boardname.h"�е�ѡ�����뿪������ص�һЩ����,��make menuconfig�����Ҳ���
��Щѡ��ġ��������uboot/configs/itop-4412_deconfigҲ����menuconfig�����õ�CONFIG�Ǹ���Ŀ¼�µ�Kconfig��
Makefile�Լ�ubootԴ�����ж���صĶ�����ʹ�ܵģ���include/configs/boardname.h�����õ�CONFIG�����Ǻ�uboot
Դ������صġ���֮ǰS3c_2440��uboot������һ��ģ�����汾�Ƿֿ��ġ���

��configs/itop4412_deconfig��ʼ����
CONFIG_ARM=y
CONFIG_ARCH_EXYNOS=y
	In--uboot/arch/arm/mach-exynos/Kconfig:
		if ARCH_EXYNOS........endif    ����������Ч���Kconfig�Ĵ���
	In--uboot/arch/arm/Kconfig:
		config ARCH_EXYNOS		ʹ�ܴ�����
		bool "Samsung EXYNOS"
		select DM
		select DM_I2C					ѡ�ж���DM_I2C
		select DM_SPI_FLASH			
		select DM_SERIAL
		select DM_SPI
		select DM_GPIO
		select DM_KEYBOARD
		imply FAT_WRITE
CONFIG_ARCH_EXYNOS4=y
	In--uboot/arch/arm/mach-exynos/Kconfig:
		config ARCH_EXYNOS4		ʹ�ܴ�����
		bool "Exynos4 SoC family"
		select CPU_V7					ѡ�ж���CPU_V7����������
		select BOARD_EARLY_INIT_F
		help
	  Samsung Exynos4 SoC family are based on ARM Cortex-A9 CPU. There
	  are multiple SoCs in this family including Exynos4210, Exynos4412,
	  and Exynos4212.
CONFIG_TARGET_ITOP4412=y
	In--uboot/arch/arm/mach-exynos/Kconfig:
		config TARGET_ITOP4412		ʹ�ܴ�����
		bool "Exynos4412 iTop-4412 board"
		select SUPPORT_SPL				ѡ�ж���SUPPORT_SPL
	In--uboot/board/samsung/itop4412/Kconfig:
		if TARGET_ITOP4412.......endif   ����������Ч���Kconfig�Ĵ���
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
����Kconfig�е�config����
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
	default "armv7" if CPU_V7		֮ǰʹ����CPU_V7����SYS_CPUΪ"armv7"
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
	default 7 if CPU_V7					֮ǰʹ����CPU_V7����SYS_ARM_ARCHΪ"7"
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
�鿴����Makefile�ж���Ķ�������ӽű���λ�����ģ�
u-boot.lds: $(LDSCRIPT) prepare FORCE
	$(call if_changed_dep,cpp_lds)
	
# If board code explicitly specified LDSCRIPT or CONFIG_SYS_LDSCRIPT, use
# that (or fail if absent).  Otherwise, search for a linker script in a
# standard location.

ifndef LDSCRIPT
	#LDSCRIPT := $(srctree)/board/$(BOARDDIR)/u-boot.lds.debug
	ifdef CONFIG_SYS_LDSCRIPT						#δ���壬����
		# need to strip off double quotes
		LDSCRIPT := $(srctree)/$(CONFIG_SYS_LDSCRIPT:"%"=%)
	endif
endif

# If there is no specified link script, we look in a number of places for it
ifndef LDSCRIPT
	ifeq ($(wildcard $(LDSCRIPT)),)
		#//$(BOARDDIR)��config.mk��BOARDDIR = $(VENDOR)/$(BOARD)������ǰ��Ķ���BOARDDIR=samsung/itop4412
		#//LDSCRIPT:=/board/samsung/itop4412/u-boot.lds  �޴��ļ�
		LDSCRIPT := $(srctree)/board/$(BOARDDIR)/u-boot.lds
	endif
	ifeq ($(wildcard $(LDSCRIPT)),)
		#//$(CPUDIR)��config.mk��CPUDIR=arch/$(ARCH)/cpu$(if $(CPU),/$(CPU),)������ǰ��Ķ���CPUDIR=arch/arm/cpu/armv7
		#//LDSCRIPT :=/arch/arm/cpu/armv7/u-boot.lds   �޴��ļ�
		LDSCRIPT := $(srctree)/$(CPUDIR)/u-boot.lds
	endif
	ifeq ($(wildcard $(LDSCRIPT)),)
		#//LDSCRIPT := /arch/arm/cpu/u-boot.lds
		LDSCRIPT := $(srctree)/arch/$(ARCH)/cpu/u-boot.lds
	endif
endif
---------------------------------
�鿴/arch/arm/cpu/u-boot.lds:
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
		#//֮ǰCPUDIR����Ϊarch/arm/cpu/armv7,���Դ˴�Ϊarch/arm/cpu/armv7/start.o
		CPUDIR/start.o (.text*)
		*(.text*)
	}
	..........................so on.
--------------------------------
�����κγ�����ں�����������ʱ�����ģ�uboot������������ӽű������ġ�����ǰ��ķ���uboot
���ӽű�Ŀ¼Ϊarch/arm/cpu/u-boot.lds����������������ļ�����CONFIG_SYS_LDSCRIPT��ָ������
������
��ڵ�ַҲ���������������ģ��������ļ��п�����CONFIG_SYS_TEXT_BASEָ����������ڱ���ʱ����ld��������ѡ��-Ttext��
board/samsung/itop4412/Kconfig ���������
if TARGET_ITOP4412

config SYS_BOARD
	default "itop4412"

config SYS_VENDOR
	default "samsung"

config SYS_CONFIG_NAME
	default "itop4412"

endif

��board/samsung/itop4412/Kconfig ��ӵ�arch/arm/cpu/armv7/exynos/Kconfig
����������ݣ�
source "board/samsung/itop4412/Kconfig"
������������
config SPL_LDSCRIPT
	default "board/samsung/common/exynos-uboot-spl.lds" if ARCH_EXYNOS5 || ARCH_EXYNOS4
	��ǰ�涨����CONFIG_ARCH_EXYNOS4=y������CONFIG_SPL_LDSCRIPT="board/samsung/common/exynos-uboot-spl.lds"
�������endmenu ֮ǰ��ӡ�

�鿴u-boot.lds
*/
















