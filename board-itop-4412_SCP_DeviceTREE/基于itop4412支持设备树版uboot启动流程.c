/*
对于任何程序，入口函数是在链接时决定的，uboot的入口是由链接脚本决定的。uboot下armv7链接脚本默认目录为arch/arm/cpu/u-boot.lds。这个可以在配置文件中与CONFIG_SYS_LDSCRIPT来指定。
入口地址也是由连接器决定的，在配置文件中可以由CONFIG_SYS_TEXT_BASE指定。这个会在编译时加在ld连接器的选项-Ttext中
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
-------------------------------------
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
















 
 
 
 
 
 