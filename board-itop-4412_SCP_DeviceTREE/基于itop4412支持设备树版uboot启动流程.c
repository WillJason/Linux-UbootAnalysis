/*
����˵һ�����uboot�����ù��̣�
	����汾��uboot��������������ɣ�һ����uboot/configs/itop-4412_deconfig,��һ����uboot/included/configs/Itop-4412.h  **
�������ļ���ʲô��ͬ�����Ĳ�ͬ����"configs/boardname_defconfig"�е�ѡ�������make menuconfig�н�
������,��"include/configs/boardname.h"�е�ѡ�����뿪������ص�һЩ����,��make menuconfig�����Ҳ���
��Щѡ��ġ��������uboot/configs/itop-4412_deconfigҲ����menuconfig�����õ�CONFIG�Ǹ���Ŀ¼�µ�Kconfig��
Makefile�Լ�ubootԴ�����ж���صĶ�����ʹ�ܵģ���include/configs/boardname.h�����õ�CONFIG�����Ǻ�uboot
Դ������صġ���֮ǰS3c_2440��uboot������һ��ģ�����汾�Ƿֿ��ġ���
��configs/itop-4412_deconfig��ʼ����
CONFIG_SPL=y
CONFIG_ARM=y
CONFIG_ARCH_EXYNOS=y
	In--uboot/arch/arm/cpu/armv7/exynos/Kconfig:
		if ARCH_EXYNOS........endif    ����������Ч���Kconfig�Ĵ���
	In--uboot/arch/arm/Kconfig:
		config ARCH_EXYNOS		ʹ�ܴ�����
		bool "Samsung EXYNOS"
		select CPU_V7   ѡ�ж���CPU_V7����������
CONFIG_TARGET_ITOP-4412=y
	In--uboot/board/samsung/itop-4412/Kconfig:
		if TARGET_ITOP-4412.......endif   ����������Ч���Kconfig�Ĵ���
	In--uboot/arch/arm/cpu/armv7/exynos/Kconfig:
		config TARGET_ITOP-4412		ʹ�ܴ�����
		bool "Exynos4412 Ultimate board"
		select SUPPORT_SPL    ѡ�ж���SUPPORT_SPL
CONFIG_OF_CONTROL=y
	��ubootԴ�������豸����ص���Ч��δ�������
CONFIG_DEFAULT_DEVICE_TREE="exynos4412-itop-4412"
	����ʹ����һ��uboot�豸���ļ�
CONFIG_OF_EMBED=y
	��ubootԴ����Ͷ���Makefile���豸����أ�δ�������
CONFIG_SYS_PROMT="ITOP-4412"
CONFIG_DEBUG_LL=y
CONFIG_DEBUG_UART_PHYS=0x13820000
CONFIG_DEBUG_UART_8250_FLOW_CONTROL=y

CONFIG_DM=y
CONFIG_DM_SERIAL=y
CONFIG_DM_GPIO=y
--------------------------------------------------------------------------------------
����Kconfig�е�config����
--------------------------------------------------------------------------------------
In--uboot/arch/arm/Kconfig:
config SYS_ARCH
	default "arm"  Ĭ��Ϊ�̶�ֵ���ļ�uboot/config.mk������ARCH := $(CONFIG_SYS_ARCH:"%"=%)��Ҳ����ARCH="arm"
config SYS_CPU
        default "arm720t" if CPU_ARM720T
        default "arm920t" if CPU_ARM920T
        default "arm926ejs" if CPU_ARM926EJS
        default "arm946es" if CPU_ARM946ES
        default "arm1136" if CPU_ARM1136
        default "arm1176" if CPU_ARM1176
        default "armv7" if CPU_V7		֮ǰʹ����CPU_V7����SYS_CPUΪ"armv7"
        default "pxa" if CPU_PXA
        default "sa1100" if CPU_SA1100
	default "armv8" if ARM64
In--uboot/arch/arm/cpu/armv7/exynos/Kconfig:
config SYS_SOC
	default "exynos"	Ĭ��Ϊ�̶�ֵ���ļ�uboot/config.mk������SOC := $(CONFIG_SYS_SOC:"%"=%)��Ҳ����SOC="exynos"

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
if TARGET_ITOP-4412   ǰ���Ѿ�ʹ����������Ч

config SYS_BOARD
	default "itop-4412"  Ĭ��Ϊ�̶�ֵ���ļ�uboot/config.mk������BOARD := $(CONFIG_SYS_BOARD:"%"=%)��Ҳ����BOARD="itop-4412"

config SYS_VENDOR
	default "samsung"	Ĭ��Ϊ�̶�ֵ���ļ�uboot/config.mk������VENDOR := $(CONFIG_SYS_VENDOR:"%"=%)��Ҳ����VENDOR="samsung"

config SYS_CONFIG_NAME
	default "itop-4412"

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
	ifdef CONFIG_SYS_LDSCRIPT				#δ���壬����
		# need to strip off double quotes
		LDSCRIPT := $(srctree)/$(CONFIG_SYS_LDSCRIPT:"%"=%)
	endif
endif

# If there is no specified link script, we look in a number of places for it
ifndef LDSCRIPT
	ifeq ($(wildcard $(LDSCRIPT)),)
		#//$(BOARDDIR)��config.mk��BOARDDIR = $(VENDOR)/$(BOARD)������ǰ��Ķ���BOARDDIR=samsung/itop-4412
		#//LDSCRIPT:=/board/samsung/itop-4412/u-boot.lds  �޴��ļ�
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
board/samsung/itop-4412/Kconfig ���������
if TARGET_ITOP-4412

config SYS_BOARD
	default "itop-4412"

config SYS_VENDOR
	default "samsung"

config SYS_CONFIG_NAME
	default "itop-4412"

endif

��board/samsung/itop-4412/Kconfig ��ӵ�arch/arm/cpu/armv7/exynos/Kconfig
����������ݣ�
source "board/samsung/itop-4412/Kconfig"
�������endmenu ֮ǰ��ӡ���make 

 �޸�include/configs/itop-4412.h �����һ���궨�壺
 #define CONFIG_SPL_LDSCRIPT    "board/samsung/common/exynos-uboot-spl.lds"

�鿴u-boot.lds
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
���ӽű�����Щ��Ķ�����linkage.h�У���������˼Ҳ���ף�������������_start.��������text�Σ�data�εȡ�
_start��arch/arm/cpu/armv7/start.S�У�һ��һ�εķ��������£�
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
/*.global�൱��C�����е�Extern�������˱������������������˱�����ȫ�֣��ⲿ���Է���*/
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
	/*ע�⵽ save_boot_params ������û�н����κβ�����ֱ�ӷ����ˣ�ע�͸�������ջָ��
	��û�г�ʼ����������ջ�б����κ����ݡ�
	ֵ��ע����� .weak α������ .weak save_boot_params

�������Ϊ��������� save_boot_params �����������ô�õ����������û�ж��壬�Ͷ�������*/
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
	/*����cp15�������쳣������� */
	/*ע�⣺���������Ѿ�����CONFIG_SKIP_LOWLEVEL_INIT��itop-4412.h��#include <configs/exynos4-common.h>��
	exynos4-common.h��#include "exynos-common.h"��exynos-common.h�ж�����CONFIG_SKIP_LOWLEVEL_INIT������������ֱ��ִ��main*/
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
/*cpu_init_cp15����������cp15Э��������ؼĴ��������ô�������MMU��cache�Լ�tlb�����û�ж���CONFIG_SYS_ICACHE_OFF����icache���ص�mmu�Լ�tlb��
�������ù��̿��Զ���cp15�Ĵ������������ﲻ��ϸ˵��*/
	bl	cpu_init_cp15
	/*cpu_init_crit���õ�level_init�������ض���������صĳ�ʼ����������������������һЩpll
	��ʼ����������Ǵ�mem�����������memory��ʼ�����������������mem�����У�ע�⣺���������
	û�����������ڴ��ʼ����*/
	bl	cpu_init_crit
#endif

	bl	_main
/*-------------------------------------
�����н�Ϊ���͡�
-------------------------------------*/
ENTRY(cpu_init_crit)
/*��ע�Ϳ������ף�cpu_init_crit���õ�lowlevel_init���������ض���������صĳ�ʼ�����������������
�����һЩpll��ʼ����������Ǵ�mem�����������memory��ʼ�����������������mem�����С�
lowlevel_init����������Ҫ��ֲ��ʵ�֣���clk��ʼ���Լ�ddr��ʼ��

��cpu_init_crit���غ�_start�Ĺ���������ˣ���������Ҫ����_main���ܽ�һ��_start������

1 ǰ���ܽ���Ĳ��֣���ʼ���쳣����������svcģʽ�����ж�

2 ����cp15����ʼ��mmu cache tlbq

3 �弶��ʼ����pll memory��ʼ��*/
	/*
	 * Jump to board specific initialization...
	 * The Mask ROM will have already initialized
	 * basic memory. Go here to bump up clock rate and handle
	 * wake up conditions.
	 */
	b	lowlevel_init		@ go setup pll,mux,memory
ENDPROC(cpu_init_crit)
#endif



//_main������arch/arm/lib/crt0.S�У�mian������������ע��������ϸ��˵�� 
/*///////////////////////  crt0.S   //////////////////////// */
ENTRY(_main)

/*
 * Set up initial C runtime environment and call board_init_f(0).
 */

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)
	ldr	sp, =(CONFIG_SPL_STACK)
#else
/*���Ƚ�CONFIG_SYS_INIT_SP_ADDR�����ֵ���ص�ջָ��sp�У�����궨��������ͷ�ļ�itop-4412.h��ָ����
CONFIG_SYS_TEXT_BASE+UBOOT_SIZE-0x1000*/
	ldr	sp, =(CONFIG_SYS_INIT_SP_ADDR)
#endif
/*������Ϊboard_init_f C���������ṩ������Ҳ����ջָ��sp��ʼ��*/
	bic	sp, sp, #7	/* 8-byte alignment for ABI compliance */
	mov	r2, sp
	sub	sp, sp, #GD_SIZE	/* allocate one GD above SP */
	bic	sp, sp, #7	/* 8-byte alignment for ABI compliance */
	mov	r9, sp		/* GD is above SP */
	mov	r1, sp
	mov	r0, #0
/*���sp������r9�Ĵ����С����r9�Ĵ����еĵ�ַ����gd�ṹ����׵�ַ*/
	/*���GD�ռ䣬�Դ����¸�ֵ*/
clr_gd:
	cmp	r1, r2			/* while not at end of GD */
	strlo	r0, [r1]		/* clear 32-bit GD word */
	addlo	r1, r1, #4		/* move to next */
	blo	clr_gd
#if defined(CONFIG_SYS_MALLOC_F_LEN)/*���δ���壬����*/
	sub	sp, sp, #CONFIG_SYS_MALLOC_F_LEN
	str	sp, [r9, #GD_MALLOC_BASE]
#endif
	/* mov r0, #0 not needed due to above code */
	bl led_on
	bl board_init_f
		/*����ӡ��Ϣ��������ò�Ƶ�����Spl_boot.c (arch\arm\cpu\armv7\exynos):��Board_f.c (common)����
		���ļ�������һ����weak����*/
		/*
		->spl_boot.c��board_init_f
			void board_init_f(unsigned long bootflag)
			{
				__aligned(8) gd_t local_gd;
				__attribute__((noreturn)) void (*uboot)(void);
			
				����ȫ��ָ�������ֵ
				setup_global_data(&local_gd);
				//����ǻ������˳����ѣ����û�з���̫���ף����д�����ָ����
				if (do_lowlevel_init()) {
					power_exit_wakeup();
				}
				���ﺯ��
				do_lowlevel_init()
					->arch_cpu_init()��
					reset_status = get_reset_status();��ȡ��λ״̬������Ӧ��ֵ��������
					switch (reset_status)Ȼ��������Ӧ�Ķ�����actions = DO_CLOCKS | DO_MEM_RESET | DO_POWER;
					set_ps_hold_ctrl();���ڹ���������PS_HOLD;
					system_clock_init();ϵͳʱ�ӳ�ʼ����
					test_uart();���Դ��ڣ�
					mem_ctrl_init(actions & DO_MEM_RESET);ddr�ڴ��ʼ����
					//tzpc_init();tzpc�����ȫԴ����ȥ���ˣ�ԭ��δ֪
				/*
				
				//Copy U-boot from mmc to RAM ��uboot code������relocaddr
				copy_uboot_to_ram();
			
				/*ʹ��printascii������ӡprink������Ϣ*//*
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
	/*ǰ4�����ʵ������gd�ṹ��ĸ��¡�
	���ȸ���sp�����ҽ�sp 8�ֽڶ��룬������溯������ջ�ܶ��룬

	Ȼ���ȡgd->bd��ַ��r9�У���Ҫע�⣬��board_init_f��gd->bd�Ѿ�
	����Ϊ�·����bd�ˣ���һ����ཫr9����bd��size�������ͻ�ȡ����board_init_f���·����gd�ˣ�

	����������Ϊrelocate_code��׼�������ȼ���here��ַ��Ȼ������µ�ַ
	ƫ������lr������code relocate�����here�ˣ�relocate_code������ת��lr��������λ�õ�here��

	�����r0�б���code���µ�ַ����ת��relocate_code*/
	ldr	sp, [r9, #GD_START_ADDR_SP]	/* sp = gd->start_addr_sp */
	bic	sp, sp, #7	/* 8-byte alignment for ABI compliance */
	ldr	r9, [r9, #GD_BD]		/* r9 = gd->bd */
	sub	r9, r9, #GD_SIZE		/* new GD is below bd */

	adr	lr, here
	ldr	r0, [r9, #GD_RELOC_OFF]		/* r0 = gd->reloc_off */
	add	lr, lr, r0
	ldr	r0, [r9, #GD_RELOCADDR]		/* r0 = gd->relocaddr */
	/*��uboot code������relocaddr��ע������û���õ���*/
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
				ע�⣺�����ж��Ƿ�������sdram�ϣ�Ҳ�����Ƿ��ض�λ�꣬����ض�λ���ֱ����������relocate_done����
				��������֮ǰ�Ѿ�copy_uboot_to_ram();��CONFIG_SYS_TEXT_BASE�����У�������ת
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
			
			relocate_done:    ��ת������
			
			#ifdef __XSCALE__        δ���壬����
				
				//* On xscale, icache must be invalidated and write buffers drained,
				// * even with cache disabled - 4.2.7 of xscale core developer's manual
				//*
				mcr	p15, 0, r0, c7, c7, 0	//* invalidate icache *
				mcr	p15, 0, r0, c7, c10, 4	//* drain write buffer *
			#endif
			
				// ARMv4- don't know bx lr but the assembler fails to see that 
			
			#ifdef __ARM_ARCH_4__     δ���壬ִ��else
				mov	pc, lr
			#else
				bx	lr    BXָ����ARMָ��ϵͳ�еĴ�״̬�л���תָ���ת��lr��ַ��lr֮ǰ����ֵ��here�����ĵط���������ת��here
			#endif
			
			ENDPROC(relocate_code)
	*/
	//���Բο���λ��ţ�Ĳ���http://blog.csdn.net/skyflying2012/article/details/37660265
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
	/*ò�Ƶ���Spl_boot.c (arch\arm\cpu\armv7\exynos)�� Board_r.c (common)�������ļ�������һ��Ҳ��weak����*/
	ldr	pc, =board_init_r	/* this is auto-relocated! */

	/*���icache��enable������Ч��icache����֤��sdram�и���ָ�cache�С�
	���Ÿ����쳣�������׵�ַ����Ϊcode��relocate�������쳣������Ҳ��relocate��

	��c_runtime_cpu_setup���أ�����һ�λ���ǽ�bss����ա�

	�������ֱ������coloured_LED_init�Լ�red_led_on,�ܶ࿪���嶼����ledָʾ�ƣ�
	�������ʵ���ϵ�ָʾ�������е������á�

	���r0��ֵgdָ�룬r1��ֵrelocaddr����������board_init_r !*/
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
	/*����initcall_run_list(init_sequence_r)����ִ��һϵ�г�ʼ��������ʵ�ֺ�벿
	�ְ弶��ʼ��
	board_init,	// Setup chipselects ���û���ID
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
			////��itop-4412.h����CONFIG_BOOTCOMMAND����
			#ifdef	CONFIG_BOOTCOMMAND
				"bootcmd="	CONFIG_BOOTCOMMAND		"\0"
			#endif
			....................
			//��itop-4412.h����CONFIG_EXTRA_ENV_SETTINGS�˺ܶ����
			//CONFIG_EXTRA_ENV_SETTINGS��CONFIG_BOOTCOMMAND�����������˼���ǰ����ֹ���console���õĴ��뼯����һ��
			//���� CONFIG_BOOTCOMMAND  ��Ч��������uboot��console��һ��һ�а�uboot�����ý�ȥ��ȥ��������\����һֱ��
			//run netboot���������Щ����uboot�������uboot�������á�
			#ifdef	CONFIG_EXTRA_ENV_SETTINGS
				CONFIG_EXTRA_ENV_SETTINGS
			#endif
			}
	������run_main_loop���ٷ���*/
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

*����˳����һ��itop-4412.h�еĲ���CONFIG_EXTRA_ENV_SETTINGS�����ж����˺ܶ�Ĳ���
������
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
���ص�ַ���ں����ڴ��ŵĵ�ַ���ն˲����ʡ�bootargs�ȵ�
�²�eMMC����Ҳ���õ���Щ������****************************************************

������������
bootm:./common/cmd_bootm.c
U_BOOT_CMD(
	bootm,	CONFIG_SYS_MAXARGS,	1,	do_bootm,
	"boot application image from memory", bootm_help_text
);
uboot�����У�bootm��Ӧ�ĺ���Ϊdo_bootm(),��./common/cmd_bootm.c;
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






















 
 
 
 
 
 