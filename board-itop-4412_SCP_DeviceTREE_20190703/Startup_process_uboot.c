/*
uboot�汾��2017.11
������һ�´��µ��������̡�
��������˵��
RomBoot --> SPL --> u-boot --> Linux kernel --> file system --> start application
������Samsung exynos4412 uboot���������ΪBL1��BL2��U-BOOT��TZSW�ĸ����֡�
ͨ����������ģʽ��oM�����������ϵ��Ժ󣬿�ʼִ��IROM�еĴ��룬��δ�����ԭ�����ǹ̻���оƬ�У����޷��޸ĵġ�
BL1���Է���4�������豸�ϣ�NAND,SD/MMC,EMMC,USB
IROM�����Ժ󣬻�ȥ��BL1��BL1ִ�����Ժ�ȥ��BL2��BL2������uboot��������ʵ�м仹�и����Ƶ�ramִ�е�һ�׶εĹ�
�̣��ܶ�оƬ���̹̻���ROM֧�ִ�nandflash��SDCARD���ⲿ������������ν���������Ǵ���Щ�ⲿ�����а���һ��
�̶���С��4K/8K/16K�ȣ��Ĵ��뵽�ڲ�RAM�����У���uboot����linux�ںˣ��ں�������ϣ�uboot������������ȫ������
IROM->BL1->BL2->uboot->linux
������������������SPL��Secondary programloader����ʽ������������Щ�仯������������˵û�иı䣬ֻ�����ǵ�һ
�׶ΰ��ƵĶ���ı���(�������).֮ǰ���˵���ubootǰ*K��С�Ĵ��룬���ڰ��˵�spl.���Կ�����ô˵
IROM--->BL1--->BL2--->SPL--->uboot--->linux
------------------------------------
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
		select DM							ѡ�ж�����DM��Ҳ����DM����ģʽ
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
-----------------------------����uboot�ű�ѡ��----------------------------------------------------------------------
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
		#//LDSCRIPT := /arch/arm/cpu/u-boot.lds						���������uboot�����ӽű�
		LDSCRIPT := $(srctree)/arch/$(ARCH)/cpu/u-boot.lds
	endif
endif
---------------------------------
�鿴/arch/arm/cpu/u-boot.lds:* /
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
		#//֮ǰCPUDIR����Ϊarch/arm/cpu/armv7,���Դ˴�Ϊarch/arm/cpu/armv7/start.o
		CPUDIR/start.o (.text*)
		*(.text*)
	}
	..........................so on.
/ *--------------------------------
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

��board/samsung/itop4412/Kconfig ��ӵ�arch/arm/mach-exynos/Kconfig
����������ݣ�
source "board/samsung/itop4412/Kconfig"

�������endmenu ֮ǰ��ӡ�
-------------------------spl uboot�ű�ѡ��-----------------------------------------------------------------------------------
�鿴����Makefile�ж���Ķ����spl���ӽű���λ�����ģ�
	all:		$(ALL-y) cfg
	ALL-$(CONFIG_SPL) += spl/u-boot-spl.bin
	## ��������CONFIG_SPL��make��ʱ��ͻ�ִ��spl/u-boot-spl.bin���Ŀ��
	
	spl/u-boot-spl.bin: spl/u-boot-spl
	@:
	spl/u-boot-spl: tools prepare \
		$(if $(CONFIG_OF_SEPARATE)$(CONFIG_SPL_OF_PLATDATA),dts/dt.dtb) \
		$(if $(CONFIG_OF_SEPARATE)$(CONFIG_TPL_OF_PLATDATA),dts/dt.dtb)
		$(Q)$(MAKE) obj=spl -f $(srctree)/scripts/Makefile.spl all
	## obj=spl ����out/u-bootĿ¼������splĿ¼
	## -f $(srctree)/scripts/Makefile.spl ˵��ִ�е�Makefile�ļ���scripts/Makefile.spl
	## $(MAKE) all �൱��make��Ŀ����all
���ϣ���CONFIG_SPL�������Ƿ���Ҫ�����spl�ļ�
spl/u-boot-spl.lds������ϵ: 
��project-x/u-boot/scripts/Makefile.spl��
������Ҫ��Ϊ���ҵ�һ��ƥ��������ļ���
$(obj)/u-boot-spl.lds: $(LDSCRIPT) FORCE
	$(call if_changed_dep,cpp_lds)
## ������$(LDSCRIPT)��$(LDSCRIPT)���������ӽű����ڵ�λ�ã�
## Ȼ������ӽű�����cpp_lds����֮���Ƶ�$(obj)/u-boot-spl.lds�У�Ҳ����spl/u-boot-spl.lds�С�
## cpp_lds�������ʵ�ֿ�cmd_cpu_lds���壬�����Ƕ�Ӧ���ӽű�����ĺ궨�����չ����

## $(LDSCRIPT)��������:
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
## Ҳ����˵���δ�board/�弶Ŀ¼��cpudirĿ¼��arch/�ܹ�/cpu/Ŀ¼��ȥ����u-boot-spl.lds�ļ���
## ���磬xxxx-4412(exynos4412 armv7)���ջ���./arch/arm/cpu/��������u-boot-spl.lds
�����������ļ�arch/arm/mach-exynos/Kconfig��,�����һ��������������
config SPL_LDSCRIPT
	default "board/samsung/common/exynos-uboot-spl.lds" if ARCH_EXYNOS5 || ARCH_EXYNOS4
	�趨Ĭ��ֵ�����ARCH_EXYNOS5��ARCH_EXYNOS4һ�������ˣ���ô��Ĭ��Ϊdefaultֵ
	�����ǰ�涨����CONFIG_ARCH_EXYNOS4=y������CONFIG_SPL_LDSCRIPT="board/samsung/common/exynos-uboot-spl.lds"
����ǰ��Linker Script��һ����Ч
���ϣ�����ָ����project-X/u-boot/board/samsung/common/exynos-uboot-spl.lds��Ϊ���ӽű���

SPL���õ���uboot����Ĵ���.��Ҫ������������Ҫ��SPLĿ���ļ��������ָ���������أ��������뵽��
ͨ������ѡ�����Խ�SPL��uboot������롢���á�������˵�ı���ѡ�����CONFIG_SPL_BUILD����
make Kconfig��ʱ��ʹ�ܡ����ձ������ɵ�SPL�������ļ���u-boot-spl��u-boot-spl.bin�Լ�u-boot-spl.map
CONFIG_SPL_TEXT_BASE    SPL����ڵ�ַ
�������ļ�itop4412.h��
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
spl�ı����Ǳ���uboot��һ���֣���uboot.bin�ߵ��������������̣����Ҫ�ص�ע�⡣ 
������˵�����ȱ�������uboot��Ҳ����uboot.bin.�ٱ���uboot-spl��Ҳ����uboot-spl.bin,��Ȼ��������
��һ��ģ����Ǳ��������Ƿֿ��ġ�

����Դ���룺������SPL����
�鿴u-boot-spl.lds:ǰ�����������uboot���ӽű�/board/samsung/common/exynos-uboot-spl.lds:	
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
SPL:_start��arch/arm/cpu/armv7/start.S�У�һ��һ�εķ��������£�
*/ 
/*.global�൱��C�����е�Extern�������˱������������������˱�����ȫ�֣��ⲿ���Է���*/
	.globl	reset
	.globl	save_boot_params_ret
#ifdef CONFIG_ARMV7_LPAE
	.global	switch_to_hypervisor_ret
#endif

reset:
	/* Allow the board to save important registers */
	/*ע�⵽ save_boot_params ������û�н����κβ�����ֱ�ӷ����ˣ�ע�͸�������ջָ��
	��û�г�ʼ����������ջ�б����κ����ݡ�
	ֵ��ע����� .weak α������ .weak save_boot_params
	�������Ϊ��������� save_boot_params �����������ô�õ����������û�ж��壬�Ͷ�������*/
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
	//uboot/include/autoconf.mk���Ѿ�������CONFIG_SKIP_LOWLEVEL_INIT=y
	//������δ���������ֱ��ִ��_main����
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	/*cpu_init_cp15����������cp15Э��������ؼĴ��������ô�������MMU��cache�Լ�tlb�����û��
	����CONFIG_SYS_ICACHE_OFF����icache���ص�mmu�Լ�tlb���������ù��̿��Զ���cp15�Ĵ���
	���������ﲻ��ϸ˵��*/
	bl	cpu_init_cp15
#ifndef CONFIG_SKIP_LOWLEVEL_INIT_ONLY
	/*cpu_init_crit���õ�level_init�������ض���������صĳ�ʼ����������������������һЩpll
	��ʼ����������Ǵ�mem�����������memory��ʼ�����������������mem�����У�ע�⣺���������
	û�����������ڴ��ʼ����*/
	bl	cpu_init_crit
#endif
#endif

	bl	_main
/*///////////////////////  crt0.S   //////////////////////// */
ENTRY(_main)
//#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)	//CONFIG_SPL_BUILD��spl�ﶨ����
	//���Ƚ�CONFIG_SPL_STACK�����ֵ���ص�ջָ��sp�У�����궨��������ͷ�ļ�itop-4412.h��ָ����
	ldr	r0, =(CONFIG_SPL_STACK)
//#else
//	ldr	r0, =(CONFIG_SYS_INIT_SP_ADDR)
//#endif
/*������Ϊboard_init_f C���������ṩ������Ҳ����ջָ��sp��ʼ��*/
	bic	r0, r0, #7	/* 8-byte alignment for ABI compliance */
	mov	sp, r0
	//��ջ��GD��early malloc�ռ�ķ��� 
	bl	board_init_f_alloc_reserve
	mov	sp, r0
	/* set up gd here, outside any C code */
	mov	r9, r0
	//��ջ��GD��early malloc�ռ�ķ��� 
	bl	board_init_f_init_reserve

	mov	r0, #0
	bl	board_init_f
		/*������SPL����ڶ�����CONFIG_SPL��Ҳ�Ͷ�����CONFIG_SPL_BUILD,�ʵ��õ���Spl_boot.c*/
		/*
		->spl_boot.c��board_init_f 		
			void board_init_f(unsigned long bootflag)
			{
				__aligned(8) gd_t local_gd;
				__attribute__((noreturn)) void (*uboot)(void);
			
				����ȫ��ָ�������ֵ
				setup_global_data(&local_gd);
				//����ǻ������˳����ѣ����û�з���̫���ף����д�����ָ����
				//do_lowlevel_init()�������������Ҫ��
				if (do_lowlevel_init()) {
					power_exit_wakeup();
				}
				/*
				//Copy U-boot from mmc to RAM ��uboot code������relocaddr
				copy_uboot_to_ram();->
					//���뺯������
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
					................//ʡ�Դ���
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
					//copy_uboot_to_ram������������һ�㺯��			
				/* Jump to U-Boot image ���������RAM������*//*
				uboot = (void *)CONFIG_SYS_TEXT_BASE;
				(*uboot)();
				/*��SPL��U-Boot�ڶ��׶�����uboot�������תִ�У�����
				 *�������������Ϥ�Ļ����ת�Ļ�Ӧ���ǣ�ldr pc, =CONFIG_SYS_TEXT_BASE
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
	//��SPL����spl_boot.c�е�board_init_r�������Ǹ��պ�����ѭ��
	//��Ȼ��Զ�������е������Ϊ֮ǰ�Ѿ������ڶ��׶���
#endif
	/* we should not return here. */
#endif
ENDPROC(_main)				
/*-------------------------------------------------------------------------------------------------
uboot�漰�ײ��ʼ���ĺ���������arch/arm/mach-exynos/spl_boot.c:do_lowlevel_init()����
--------------------------------------------------------------------------------------------------*/
int do_lowlevel_init(void)
{
		arch_cpu_init();
		get_reset_status()			//��ȡ��λ״̬������Ӧ��ֵ��������
		switch (reset_status)	//Ȼ��������Ӧ�Ķ�����actions = DO_CLOCKS | DO_MEM_RESET | DO_POWER;
		set_ps_hold_ctrl();			//���ڹ���������PS_HOLD;
		system_clock_init();->		//ϵͳʱ�ӳ�ʼ����
			/*
			struct exynos4_clock *clk =
			(struct exynos4_clock *)samsung_get_base_clock();

			writel(CLK_SRC_CPU_VAL, &clk->src_cpu);����4412 CLK_SRC_CPU�Ĵ�����CMU_CPU��4��MUX��������ЩMUX
		
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
		exynos_pinmux_config(PERIPH_ID_UART2, PINMUX_FLAG_NONE);//�˿�����	
		debug_uart_init();->						//���Դ��ڣ�
		mem_ctrl_init(actions & DO_MEM_RESET);->			//ddr�ڴ��ʼ����
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
					//8 banks��row 15bits��col 10bits��chip_base=40��ʾ��40000000��ʼ
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
		//tzpc_init();					//tzpc�����ȫԴ����ȥ���ˣ�ԭ��δ֪				
}
			
/*------------------------------------------------------------------------------------------------
do_lowlevel_init()����====>END.
-------------------------------------------------------------------------------------------------*/		
//SPL�׶ν�����������������������
/*
����Դ���룺����������U-boot���֣��ڶ��׶Σ�
�鿴u-boot.lds:ǰ�����������uboot���ӽű�/arch/arm/cpu/u-boot.lds:
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
		//֮ǰCPUDIR����Ϊarch/arm/cpu/armv7,���Դ˴�Ϊarch/arm/cpu/armv7/start.o
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
���ӽű�����Щ��Ķ�����linkage.h�У���������˼Ҳ���ף�������������_start.��������text�Σ�data�εȡ�
_start��arch/arm/cpu/armv7/start.S�У�һ��һ�εķ��������£�
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
/*.global�൱��C�����е�Extern�������˱������������������˱�����ȫ�֣��ⲿ���Է���*/
	.globl	reset
	.globl	save_boot_params_ret
#ifdef CONFIG_ARMV7_LPAE
	.global	switch_to_hypervisor_ret
#endif

reset:
	/* Allow the board to save important registers */
	/*ע�⵽ save_boot_params ������û�н����κβ�����ֱ�ӷ����ˣ�ע�͸�������ջָ��
	��û�г�ʼ����������ջ�б����κ����ݡ�
	ֵ��ע����� .weak α������ .weak save_boot_params
	�������Ϊ��������� save_boot_params �����������ô�õ����������û�ж��壬�Ͷ�������*/
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
	//uboot/include/autoconf.mk���Ѿ�������CONFIG_SKIP_LOWLEVEL_INIT=y
	//������δ���������ֱ��ִ��_main����
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	/*cpu_init_cp15����������cp15Э��������ؼĴ��������ô�������MMU��cache�Լ�tlb�����û��
	����CONFIG_SYS_ICACHE_OFF����icache���ص�mmu�Լ�tlb���������ù��̿��Զ���cp15�Ĵ���
	���������ﲻ��ϸ˵��*/
	bl	cpu_init_cp15
#ifndef CONFIG_SKIP_LOWLEVEL_INIT_ONLY
	/*cpu_init_crit���õ�level_init�������ض���������صĳ�ʼ����������������������һЩpll
	��ʼ����������Ǵ�mem�����������memory��ʼ�����������������mem�����У�ע�⣺���������
	û�����������ڴ��ʼ����*/
	bl	cpu_init_crit
#endif
#endif
	bl	_main
----------------------------------------------------------------------
//_main������arch/arm/lib/crt0.S�У�mian������������ע��������ϸ��˵�� 
/*///////////////////////  crt0.S   //////////////////////// */
ENTRY(_main)
/*
 * Set up initial C runtime environment and call board_init_f(0).
 */

//#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)
//	ldr	r0, =(CONFIG_SPL_STACK)
//#else
	/*���Ƚ�CONFIG_SYS_INIT_SP_ADDR�����ֵ���ص�ջָ��sp�У�����궨��������ͷ�ļ�itop-4412.h��ָ����
	CONFIG_SYS_TEXT_BASE+UBOOT_SIZE-0x1000*/
	ldr	r0, =(CONFIG_SYS_INIT_SP_ADDR)
//#endif
	/*������Ϊboard_init_f C���������ṩ������Ҳ����ջָ��sp��ʼ��*/
	bic	r0, r0, #7	/* 8-byte alignment for ABI compliance */
	mov	sp, r0
	//��ջ��GD��early malloc�ռ�ķ��� 
	bl	board_init_f_alloc_reserve
	mov	sp, r0
	/* set up gd here, outside any C code */
	mov	r9, r0
	//��ջ��GD��early malloc�ռ�ĳ�ʼ�� 
	bl	board_init_f_init_reserve

	mov	r0, #0
	bl	board_init_f
		/*����������uboot�У�δʹ��CONFIG_SPL_BUILD,���õ���Board_f.c (common)*/
		/*
		->board_f.c��board_init_f
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
		//������ø�������init_sequence_f,��������������ǳ���Ҫ�Ĳ���
		/*
		static init_fnc_t init_sequence_f[] = {
			........
			/ *
			initf_dm
			�����漰����DM����ģ����صĶ���.
			1��DM����ģ�͵�һ������bind->ofdata_to_platdata(��ѡ)->probe
			2��������bind����ʱ������ɵģ���Ҫ����豸�������İ󶨣��Լ�node ע��node��ƥ�䵽�������豸�ڵ�
			֮������ӡ�ofdata_to_platdata(��ѡ)->probe ������device_probe��������ɵġ�
			initf_dm��Ҫ�ǳ�ʼ����������uclass�������󶨺���DM_FLAG_PRE_RELOC��־λ�������豸�����ԵĽڵ�
			* /
			initf_dm,->       								//ִ��bind��������ʼ��һ��dmģ�͵����νṹ
				dm_init_and_scan(true)->				//��ʼ�����ڵ��豸����bind���ڵ�Ĵ���u-boot,dm-pre-reloc���Ե�һ���ӽڵ㡣��DM���г�ʼ�����豸�Ľ���
					//DM�ĳ�ʼ��
					//.�������豸root��udevice�������gd->dm_root�С�
					//.���豸��ʵ��һ�������豸����Ҫ��Ϊuboot�������豸�ṩһ�����ص㡣
					//.��ʼ��uclass����gd->uclass_root
					dm_init	->										//�����ڵ�󶨵�gd->dm_root�ϣ���ʼ�����ڵ��豸
						//DM_ROOT_NON_CONST��ָ���豸udevice��root_info�Ǳ�ʾ���豸���豸��Ϣ
        		//device_bind_by_name����Һ��豸��Ϣƥ���driver��Ȼ�󴴽���Ӧ��udevice��uclass�����а󶨣�������DM_ROOT_NON_CONST�С�
        		//device_bind_by_name�������ǻ����˵��������������ʱֻ��Ҫ�˽�root���豸��udevice�Լ���Ӧ��uclass���Ѿ�������ɡ�
						device_bind_by_name(NULL, false, &root_info, &DM_ROOT_NON_CONST);
							lists_driver_lookup_name(info->name);		//��driver table�л�ȡ����Ϊname��driver��
							device_bind_common ->	//ƥ���豸�������������豸�ڵ��parent�ڵ㽨����ϵ��Ҳ���ǽ������νṹ
								uclass_bind_device	//�����豸���ڶ�Ӧ��U_CLASS������
								drv->bind(dev)			//�豸������bind�ӿں���
	              parent->driver->child_post_bind(dev)//���ڵ�������child_post_bind�ӿں���
	              uc->uc_drv->post_bind//�豸�������������post_bind�ӿں�����������豸�ڵ����������ӿ�����soc�½���չ���ģ�
	          //�Ը��豸ִ��probe������device_probe�����ٽ���˵��
						device_probe(DM_ROOT_NON_CONST);//device_probe(dev)�����豸�ļ����������ע�ᣬ�����Ǹ��豸
					dm_scan_platdata ->						//��ƽ̨�豸�н���udevice��uclass,����ʹ�ú�U_BOOT_DEVICE������豸��������ƥ�䣬Ҳ����bind�ӽڵ�
					dm_extended_scan_fdt->
						dm_scan_fdt	->							//���豸���������豸����������ƥ�䣬Ȼ��bind
							dm_scan_fdt_node	//������豸����ڣ��ڸú����л�ȷ���豸�Ƿ����boot,dm-pre-reloc���ԣ����û���򲻻��
		          lists_bind_fdt		//��������ƥ�䵽���豸������
		            device_bind_with_driver_data ->	//���ƥ�䵽���а�
		            	device_bind_common ->		//ƥ���豸�������������豸�ڵ��parent�ڵ㽨����ϵ��Ҳ���ǽ������νṹ	
	                  uclass_bind_device//�����豸���ڶ�Ӧ��U_CLASS������
	                  drv->bind(dev)//�豸������bind�ӿں���
	                  parent->driver->child_post_bind(dev)//���ڵ�������child_post_bind�ӿں���
	                  uc->uc_drv->post_bind//�豸�������������post_bind�ӿں�����������豸�ڵ����������ӿ�����soc�½���չ���ģ�
						ofnode_to_offset(ofnode_path("/clocks"));			//�󶨹̶���clocks
						dm_scan_fdt_node		//������豸����ڣ��ڸú����л�ȷ���豸�Ƿ����boot,dm-pre-reloc���ԣ����û���򲻻��
					dm_scan_other	->		
		
			env_init,//�����ѻ��������ĵ�ַ��ֵ
			init_baud_rate,	//���ò�����
			setup_machine,//���û���ID
			........
		}
		*/
#if ! defined(CONFIG_SPL_BUILD)

/*
 * Set up intermediate environment (new sp and gd) and call
 * relocate_code(addr_moni). Trick here is that we'll return
 * 'here' but relocated.
 */
	/*ǰ4�����ʵ������gd�ṹ��ĸ��¡�
	���ȸ���sp�����ҽ�sp 8�ֽڶ��룬������溯������ջ�ܶ��룬

	Ȼ���ȡgd->bd��ַ��r9�У���Ҫע�⣬��board_init_f��gd->bd�Ѿ�
	����Ϊ�·����bd�ˣ���һ����ཫr9����bd��size�������ͻ�ȡ����board_init_f���·����gd�ˣ�

	����������Ϊrelocate_code��׼�������ȼ���here��ַ��Ȼ������µ�ַ
	ƫ������lr������code relocate�����here�ˣ�relocate_code������ת��lr��������λ�õ�here��

	�����r0�б���code���µ�ַ����ת��relocate_code*/
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
	/*��uboot code������relocaddr��ע������û���õ���*/
	b	relocate_code
		/*
		ENTRY(relocate_code)
			ldr	r1, =__image_copy_start	/* r1 <- SRC &__image_copy_start *
			subs	r4, r0, r1		/* r4 <- relocation offset *
			ע�⣺�����ж��Ƿ�������sdram�ϣ�Ҳ�����Ƿ��ض�λ�꣬����ض�λ���ֱ����������relocate_done����
			��������֮ǰ�Ѿ�copy_uboot_to_ram();��CONFIG_SYS_TEXT_BASE�����У�������ת
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
		
		relocate_done:				��ת������		
		#ifdef __XSCALE__					δ���壬����
			/*
			 * On xscale, icache must be invalidated and write buffers drained,
			 * even with cache disabled - 4.2.7 of xscale core developer's manual
			 *
			mcr	p15, 0, r0, c7, c7, 0	/* invalidate icache *
			mcr	p15, 0, r0, c7, c10, 4	/* drain write buffer *
		#endif		
			/* ARMv4- don't know bx lr but the assembler fails to see that *		
		#ifdef __ARM_ARCH_4__			δ���壬ִ��else
			mov	pc, lr
		#else
			bx	lr					BXָ����ARMָ��ϵͳ�еĴ�״̬�л���תָ���ת��lr��ַ��lr֮ǰ����ֵ��here�����ĵط���������ת��here
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
	/*����������uboot�����Ե��õ���Board_r.c (common)����ļ��еĺ���*/
	//�������ں������Ƿ���board_init_r���������������Ҫ�ĺ�����
	ldr	pc, =board_init_r	/* this is auto-relocated! */
	
	/*���icache��enable������Ч��icache����֤��sdram�и���ָ�cache�С�
	���Ÿ����쳣�������׵�ַ����Ϊcode��relocate�������쳣������Ҳ��relocate��

	��c_runtime_cpu_setup���أ�����һ�λ���ǽ�bss����ա�

	�������ֱ������coloured_LED_init�Լ�red_led_on,�ܶ࿪���嶼����ledָʾ�ƣ�
	�������ʵ���ϵ�ָʾ�������е������á�

	���r0��ֵgdָ�룬r1��ֵrelocaddr����������board_init_r !*/
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
	/*����initcall_run_list(init_sequence_r)����ִ��һϵ�г�ʼ��������ʵ�ֺ�벿
	�ְ弶��ʼ��
	
	
	/initr_dm,
	initr_mmc,
	//initr_dm��initr_mmc�����漰�Ƚ϶࣬�ŵ��������
	initr_env,	->
		set_default_env(NULL);->
			himport_r(&env_htab, (char *)default_environment,
			sizeof(default_environment), '\0', flags, 0,
			0, NULL) == 0)
			//ʹ��Ĭ�ϵ�����
			const uchar default_environment[] = {
			#ifdef	CONFIG_BOOTARGS
				"bootargs="	CONFIG_BOOTARGS			"\0"
			#endif
			////��itop4412.h����CONFIG_BOOTCOMMAND����
			#ifdef	CONFIG_BOOTCOMMAND
				"bootcmd="	CONFIG_BOOTCOMMAND		"\0"
			#endif
			....................
			//��itop4412.h����CONFIG_EXTRA_ENV_SETTINGS�˺ܶ����
			//CONFIG_EXTRA_ENV_SETTINGS��CONFIG_BOOTCOMMAND�����������˼���ǰ����ֹ���console���õĴ��뼯����һ��
			//���� CONFIG_BOOTCOMMAND  ��Ч��������uboot��console��һ��һ�а�uboot�����ý�ȥ��ȥ��������\����һֱ��
			//run netboot���������Щ����uboot�������uboot�������á�
			#ifdef	CONFIG_EXTRA_ENV_SETTINGS
				CONFIG_EXTRA_ENV_SETTINGS
			#endif
			}
	.................
	������run_main_loop���ٷ���*/
	if (initcall_run_list(init_sequence_r))
		hang();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
/*-------------------------------------------------------------------------------------------------
uboot�漰�豸������������������/common/board_r.c:init_sequence_r[]��initr_dm��initr_mmc����
--------------------------------------------------------------------------------------------------*/
	//�����������������ͺ�uboot���豸���йأ��°�uboot�������豸����DM����ģ�ͣ�driver model��
	//��linuxһ����u-boot����Ҳ������һ������ģ�͡����磬 of_match��ƥ�䣬probe��ʶ��ȡ�
	#ifdef CONFIG_DM
		initr_dm,->									//ִ��bind��������ʼ��һ��dmģ�͵����νṹ
			/*
				dm��ʼ���Ľӿ���dm_init_and_scan�С� ���Է�����uboot relocate֮ǰ��initf_dm��֮��
			��initr_dm�������������������Ҫ�������ڲ���������˵��һ��dts�ڵ��еġ�u-boot,dm-pre-reloc��
			���ԣ����������������ʱ�����ʾ����豸��relocate֮ǰ����Ҫʹ�á���dm_init_and_scan�Ĳ�
			��Ϊtrueʱ��ֻ��Դ��С�u-boot,dm-pre-reloc�����ԵĽڵ���н�������������Ϊfalse��ʱ��
			�������нڵ㶼���н�������u-boot,dm-pre-reloc��������Ƚ���			*/
			dm_init_and_scan(false);
				.......................//(�����֮ǰ���õ���ͬһ�������������ظ�����)
				/* 
					��init_sequence_r�е�initr_dm�У������FDT�Ľ��������������е�����node,����Ը���
				�ڵ������ udevice��uclass�Ĵ������Լ�������ɲ��ֵİ󶨹�ϵ��ע�⣬����ֻ�ǰ󶨣�
				��������driver��bind�����������豸��û���������Ҳ���ǻ�û��ִ���豸��probe������
				���ھ���xxx�豸��ʼ���׶ν�������豸�ļ����������ע��
					��U_Boot DM ��ʼ���׶�(initf_dm �� initr_dm)��ͨ������ dm_init_and_scan(bool pre_reloc_only)
				�������� (U_BOOT_DEVICE �к� driver �� name������ dts �� driver �� compatible) ƥ�䵽��Ӧ��
				driver��Ȼ�����device_bind_common �������� udevice��udevice ��� driver �󶨣�������
				driver �е�uclass id �ҵ���Ӧ�� uclass driver����������Ӧ�� uclass�� ���Ѹ��豸�ҵ� 
				uclass ���豸�ڵ�֮�¡������� driver �� bind ������
					�������XXX�豸��ʼ������������Ҫ�Ĺ����ǵ���uclass_first_device(UCLASS_XXX, &dev)
				����XXX_probe�ȵȺ�������uclass���豸�����л�ȡ��һ��udevice�����ҽ���probe�����գ���ͨ����
				��device_probe(dev)����XXX�豸�ļ����������ע��
					�������device_probe(dev)��ʵ�ֵĲ��ֹ��̣�
					.�����豸��˽������
					.�Ը��豸����probe				
					.ִ��probe device֮ǰuclass��Ҫ���õ�һЩ����				
					.����driver��ofdata_to_platdata����dts��Ϣת��Ϊ�豸��ƽ̨���ݣ���Ҫ��		
					.����driver��probe��������Ҫ��		
					.ִ��probe device֮��uclass��Ҫ���õ�һЩ����
				*/

	#endif
	#ifdef CONFIG_MMC
		initr_mmc,->						///��ʼ��eMMC��SD��������������� 
				mmc_initialize(gd->bd);->
					mmc_probe(bis);->			//����������ѡ��һ����DM����ģ�ͣ���һ����ֱ��OF_CONTROL,����ѡ�����ǰ��
						uclass_get(UCLASS_MMC, &uc);		//��gd->uclass_root�����ȡ��Ӧ��uclass
						uclass_get_device_by_seq(UCLASS_MMC, i, &dev);	//ͨ��MMC�Ű�˳���uclass�л�ȡudevice
						uclass_foreach_dev(dev, uc)			//���� probe uclass�����е�udevice����������
						device_probe(dev);	//�豸�ļ����������ע�ᣬʵ�ֹ�����ǰ���н�
					mmc_do_preinit();
						mmc_start_init()		//��ʼ��MMC���Լ�����CMD0,CMD2...����mmc
	#endif
	/*
	��һ��exynos4412�����eMMC������drivers/mmc/exynos_dw_mmc.c
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
	������{ .compatible = "samsung,exynos4412-dw-mshc" },��ƥ��ģ��ٿ����豸������
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
	SD��������drivers/mmc/s5p_sdhci.c
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
	�豸����ģ��ƥ�����̲μ�֮ǰ��initr_mmc����
	*/
/*
�й�device_probe()��DMʵ�ֺ����ں��渽¼�����
*/
/*------------------------------------------------------------------------------------------------
initr_dm��initr_mmc����====>END.
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
	/*main_loop�е���bootdelay_process�����ж���delay time����û�м����£�������
	prompt"Hit any key to stop autoboot"��ͬʱreturn��s="bootcmd"*/
	s = bootdelay_process();
	if (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);
/*��û�м�������autoboot_command(s)(s="bootcmd").�����bootcmd��Ҫ�����ں˵�λ�����ã�
��bootcmdδ��ֵ��U-boot��һֱ�ȴ����������bootm��Ҳ�������ںˣ������������*/
	autoboot_command(s);

	cli_loop();
}

/*///////////////////////  autoboot.c   //////////////////////// */

/*main�������õ�����

bootcmd�Ƿ��ж��壿
./include/env_default.h����bootcmdΪCONFIG_BOOTCOMMAND
./inlcude/itop-4412.h�ж���CONFIG_BOOTCOMMAND��ֵ������bootm��������Ҫ���ĵ��ص㡣
"mmc read ${loadaddr} 0x1000 0x4000; mmc read ${dtb_addr} 0x800 0xa0; bootm ${loadaddr} - ${dtb_addr}" \
mmc read 0x40007000 0x1000 0x4000�����洢�豸�ϴӿ��0x1000��ʼ��0x4000���洢��Ķ���
�������ڴ�0x40007000��ʼ�Ŀռ��ڡ�

*����˳����һ��itop4412.h�еĲ���CONFIG_EXTRA_ENV_SETTINGS�����ж����˺ܶ�Ĳ���
������
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
���ص�ַ���ں����ڴ��ŵĵ�ַ���ն˲����ʡ�bootargs�ȵ�
�²�eMMC����Ҳ���õ���Щ������****************************************************
������������
bootm:./cmd/bootm.c
U_BOOT_CMD(
	bootm,	CONFIG_SYS_MAXARGS,	1,	do_bootm,
	"boot application image from memory", bootm_help_text
);

uboot�����У�bootm��Ӧ�ĺ���Ϊdo_bootm(),��./cmd/bootm.c;
bootm - boot application image from image in memory 
do_bootm()����do_bootm_states()��./common/bootm.c;

do_bootm_states()
����bootm_find_os->boot_get_kernel
printf("## Booting kernel from Legacy Image at %08lx ...\n",
		       img_addr);
����bootm_find_other->bootm_find_ramdisk_fdt->bootm_find_fdt->boot_get_fdt->
printf("   Booting using the fdt blob at %#08lx\n", fdt_addr);

��ӡdo_bootm_states: reserve fdt done.
������bootm_os_get_boot_func()
bootm_os_get_func()��boot_os[os]�����ж�Ӧ��linux��صĺ�����ڣ�
boot_os[os]�������ҵ�linux�����do_bootm_linux()

do_bootm_linux()��arch/arm/lib.
boot_prep_linux(images);ʵ��uboot��kernel�Ĳ������ݣ�tag��
����image_setup_linux(images)Ȼ�����boot_relocate_fdt().
boot_relocate_fdt����bootmap��
��������һ��δ��ʹ�õ��ڴ棬���Ž�DTB�������ݸ��Ƶ�������򣨼���lmb�����������


boot_jump_linux(images, flag);����kernel����ӡ��## Transferring control to 
Linux (at address 40007000)...
*/
void autoboot_command(const char *s)
{
	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

/*//abortboot���������ʱ���ڵ����㣬����û������ʱ�����н��յ��������������ںˡ�
abortboot��������ӡHit any key to stop autoboot:��*/
	if (stored_bootdelay != -1 && s && !abortboot(stored_bootdelay)) {
#if defined(CONFIG_AUTOBOOT_KEYED) && !defined(CONFIG_AUTOBOOT_KEYED_CTRLC)
		int prev = disable_ctrlc(1);	/* disable Control C checking */
#endif

/*run_command_list�е�����hush shell�����������(parse_stream_outer����)������bootcmd�е��������*/
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






//��¼��
int device_probe(struct udevice *dev)
{
    const struct driver *drv;
    int size = 0;
    int ret;
    int seq;

    if (dev->flags & DM_FLAG_ACTIVATED)
        return 0;
// ��ʾ����豸�Ѿ���������
    drv = dev->driver;
    assert(drv);
// ��ȡ����豸��Ӧ��driver
    /* Allocate private data if requested and not reentered */
    if (drv->priv_auto_alloc_size && !dev->priv) {
        dev->priv = alloc_priv(drv->priv_auto_alloc_size, drv->flags);
// Ϊ�豸����˽������
    }
    /* Allocate private data if requested and not reentered */
    size = dev->uclass->uc_drv->per_device_auto_alloc_size;
    if (size && !dev->uclass_priv) {
        dev->uclass_priv = calloc(1, size);
// Ϊ�豸����uclass����˽������
    }

// ������˸��豸��probe
    seq = uclass_resolve_seq(dev);
    if (seq < 0) {
        ret = seq;
        goto fail;
    }
    dev->seq = seq;
    dev->flags |= DM_FLAG_ACTIVATED;
// ����udevice�ļ����־
    ret = uclass_pre_probe_device(dev);
// uclass��probe device֮ǰ��һЩ�����ĵ���
    if (drv->ofdata_to_platdata && dev->of_offset >= 0) {
        ret = drv->ofdata_to_platdata(dev);
// ����driver�е�ofdata_to_platdata��dts��Ϣת��Ϊ�豸��ƽ̨����
    }
    if (drv->probe) {
        ret = drv->probe(dev);
// ����driver��probe�������������豸������������
    }
    ret = uclass_post_probe_device(dev);
    return ret;
}
/*��Ҫ�����������£�

.�����豸��˽������

.�Ը��豸����probe

.ִ��probe device֮ǰuclass��Ҫ���õ�һЩ����

.����driver��ofdata_to_platdata����dts��Ϣת��Ϊ�豸��ƽ̨���ݣ���Ҫ��

.����driver��probe��������Ҫ��

.ִ��probe device֮��uclass��Ҫ���õ�һЩ����
*/















































