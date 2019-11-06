/*
先来看看配置过程：
执行build_uboot.sh SCP_1GDDR
*分析build_uboot.sh文件
if [ "$1" = "SCP_1GDDR" ]
	then
	make itop_4412_android_config_scp_1GDDR
*打开顶层Makefile查找目标和依赖
itop_4412_android_config_scp_1GDDR:		unconfig
	@$(MKCONFIG) $(@:_config=) arm arm_cortexa9 smdkc210 samsung s5pc210 SCP_1GDDR
	
MKCONFIG	:= $(SRCTREE)/mkconfig
其实执行Make itop_4412_android_config_scp_1GDDR的配置命令就相当于执行了
mkconfig   itop_4412_android  arm arm_cortexa9 smdkc210 samsung s5pc210 SCP_1GDDR这条指令。
*打开mkconfig文件来看看配置:	
echo "ARCH   = $2" >  config.mk
echo "CPU    = $3" >> config.mk
echo "BOARD  = $4" >> config.mk
[ "$5" ] && [ "$5" != "NULL" ] && echo "VENDOR = $5" >> config.mk
[ "$6" ] && [ "$6" != "NULL" ] && echo "SOC    = $6" >> config.mk
if   [ "$7" = "SCP_1GDDR" ] ||   [ "$7" = "SCP_2GDDR" ] || [ "$7" = "SCP_1GDDR_Ubuntu" ] ||   [ "$7" = "SCP_2GDDR_Ubuntu" ]
then 
     echo "CORE   =  SCP" >> config.mk
     ln -sf ${SRCTREE}/board/samsung/smdkc210/lowlevel_init_SCP.S  ${SRCTREE}/board/samsung/smdkc210/lowlevel_init.S	
     ln -sf ${SRCTREE}/cpu/arm_cortexa9/s5pc210/cpu_init_SCP.S     ${SRCTREE}/cpu/arm_cortexa9/s5pc210/cpu_init.S
也就是说ARCH="arm" CPU="arm_cortexa9" BOARD="smdkc210" VENDOR="samsung" SOC="s5pc210" 

*顶层的Makefile里面还有其他语句
# load other configuration
include $(TOPDIR)/config.mk   这句包含了顶层的config.mk
*打开顶层的config.mk文件，有这么一句
ifdef	ARCH
sinclude $(TOPDIR)/lib_$(ARCH)/config.mk	# include architecture dependend rules
endif
也就是说包含了uboot/lib_arm/config.mk
我们也来继续打开这个文件。最后可以看到:
LDSCRIPT := $(SRCTREE)/cpu/$(CPU)/u-boot.lds
我们可以得知链接脚本lds文件是uboot/cpu/arm_cortexa9/u-boot.lds


查看链接脚本
*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text	:
	{
		cpu/arm_cortexa9/start.o		(.text)
		cpu/arm_cortexa9/s5pc210/cpu_init.o	(.text)
		board/samsung/smdkc210/lowlevel_init.o	(.text)
		common/ace_sha1.o (.text)
		*(.text)
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : { *(.data) }

	. = ALIGN(4);
	.got : { *(.got) }

	__u_boot_cmd_start = .;
	.u_boot_cmd : { *(.u_boot_cmd) }
	__u_boot_cmd_end = .;

	. = ALIGN(4);
	__bss_start = .;
	.bss : { *(.bss) }
	_end = .;
}
/*
链接脚本中这些宏的定义在linkage.h中，看字面意思也明白，程序的入口是在_start.，后面是text段，data段等。
_start在cpu/arm_cortexa9/start.S中，一段一段的分析，如下：
*/ 
/*
 *************************************************************************
 *
 * Jump vector table as in table 3.1 in [1]
 *
 *************************************************************************
 */
	.word 0x2000
	.word 0x0
	.word 0x0
	.word 0x0
.globl _start
_start: b	reset
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq
	
/*
 * the actual reset code
 */

reset:
#if 0
	/*
	 * set the cpu to SVC32 mode and IRQ & FIQ disable
	 *设置SVC32管理模式
	 */
	mrs	r0, cpsr
	bic	r0, r0, #0x3f
	orr	r0, r0, #0xd3
	msr	cpsr, r0
#else//*****ly 执行此段代码
	mrs	r0, cpsr
	bic	r0, r0, #0x1f
	orr	r0, r0, #0xd3
	msr	cpsr,r0
#endif
//	cache_invalidate_dcache_v7			@ call cache macro
#if 1 //*****ly
cache_init:
	mrc	p15, 0, r0, c0, c0, 0	@ read main ID register
	and	r1, r0, #0x00f00000	@ variant
	and	r2, r0, #0x0000000f	@ revision
	orr	r2, r2, r1, lsr #20-4	@ combine variant and revision
	cmp	r2, #0x30
	mrceq	p15, 0, r0, c1, c0, 1	@ read ACTLR
	orreq	r0, r0, #0x6		@ Enable DP1(2), DP2(1)
	mcreq	p15, 0, r0, c1, c0, 1	@ write ACTLR
	/*
	 * Invalidate L1 I/D
	 */
	mov	r0, #0			@ set up for MCR
	mcr	p15, 0, r0, c8, c7, 0	@ invalidate TLBs
	mcr	p15, 0, r0, c7, c5, 0	@ invalidate icache

	/*
	 * disable MMU stuff and caches
	 *关闭MMU
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002000	@ clear bits 13 (--V-)
	bic	r0, r0, #0x00000007	@ clear bits 2:0 (-CAM)
	orr	r0, r0, #0x00001000	@ set bit 12 (---I) Icache
	orr	r0, r0, #0x00000002	@ set bit 1  (--A-) Align
	orr	r0, r0, #0x00000800	@ set bit 11 (Z---) BTB
	mcr	p15, 0, r0, c1, c0, 0
#endif

/* Read booting information 读取启动信息*/
        ldr	r0, =POWER_BASE
        ldr	r1, [r0,#OMR_OFFSET]
        bic	r2, r1, #0xffffffc1

	/* NAND BOOT */
@	cmp	r2, #0x0		@ 512B 4-cycle
@	moveq	r3, #BOOT_NAND
...................//(此处代码省略，都是关于NAND启动的，我们是eMMC启动，忽略)
	/* SD/MMC BOOT */
	......
	/* eMMC4.3 BOOT */
	.......
	/* eMMC441 BOOT */   //我们使用的是这个，此文仅分析eMMc启动，别的启动莫问我，哼~~
	cmp		r2, #0x28
	moveq	r3, #BOOT_EMMC441
	
  /* NOR BOOT */
	.......

	/* For second device booting */
	/* OneNAND BOOTONG failed */
@	cmp     r2, #0x8
@	moveq   r3, #BOOT_SEC_DEV

	/* Uart BOOTONG failed */
@	cmp     r2, #(0x1<<4)
@	moveq   r3, #BOOT_SEC_DEV
	
	ldr	r0, =INF_REG_BASE
	str	r3, [r0, #INF_REG3_OFFSET] 		

	/* Go setup Memory and board specific bits prior to relocation.*/
	//b     .
	bl	lowlevel_init	/* go setup pll,mux,memory 时钟、内存、串口等初始化*/
	
	ldr	r0, =0x1002330C  /* PS_HOLD_CONTROL register 关于供电锁存类PS_HOLD*/
	ldr	r1, =0x00005300	 /* PS_HOLD output high	*/
	str	r1, [r0]


	/* get ready to call C functions */
	ldr	sp, _TEXT_PHY_BASE	/* setup temp stack pointer */
	sub	sp, sp, #12
	mov	fp, #0			/* no previous frame, so fp=0 */
	
	/* when we already run in ram, we don't need to relocate U-Boot.
	 * and actually, memory controller must be configured before U-Boot
	 * is running in ram.
	 *当已经运行在ram上，就跳过重定位的代码。这里我们还没又运行在ram上，所以不跳到after_copy
	 */
	ldr	r0, =0xff000fff
	bic	r1, pc, r0		/* r0 <- current base addr of code */
	ldr	r2, _TEXT_BASE		/* r1 <- original base addr in ram */
	bic	r2, r2, r0		/* r0 <- current base addr of code */
	cmp     r1, r2                  /* compare r0, r1                  */
	beq     after_copy		/* r0 == r1 then skip flash copy   */



	ldr	r0, =0x11000104  /* GPL2(0)  */
	ldr	r1, =0x00000001	 /* GPL2(0 output high	*/
	str	r1, [r0]
	
	ldr	r0, =0x11000100  /* GPL2(0)  */
	ldr	r1, =0x00000001	 /* GPL2(0 output high	*/
	str	r1, [r0]

	
	/* wait ?us */
	mov	r1, #0x10000
	9:	subs	r1, r1, #1
		bne	9b

	ldr	r0, =INF_REG_BASE
	ldr	r1, [r0, #INF_REG3_OFFSET]

	cmp     r1, #BOOT_EMMC441
	beq     emmc441_boot
	
	emmc441_boot:
	#if defined(CONFIG_CLK_1000_400_200) || defined(CONFIG_CLK_1000_200_200) || defined(CONFIG_CLK_800_400_200)
		ldr	r0, =CMU_BASE
		ldr	r2, =CLK_DIV_FSYS3_OFFSET
		ldr	r1, [r0, r2]
		orr r1, r1, #0x3
		str r1, [r0, r2]
	#endif
		bl		emmc441_uboot_copy
			/*
				可以看的出copy_bl2是个指向某个固定地址的函数，所以猜测这个代码是固化在IROM中。
				copy_emmc441_to_mem copy_bl2 = (copy_emmc441_to_mem)*(u32 *)(0x02020044);	//MSH_ReadFromFIFO_eMMC
				emmc441_endboot_op end_bootop = (emmc441_endboot_op)*(u32 *)(0x02020048);	//MSH_EndBootOp_eMMC
			
				copy_bl2(MOVI_UBOOT_BLKCNT, CFG_PHY_UBOOT_BASE); //mj
				/* stop bootop *
				end_bootop();
			*/
	
		ldr   r0, =0x43e00000
		ldr   r1, [r0]
		ldr   r2, =0x2000
		cmp r1, r2
		bne  second_mmcsd_boot
		b	after_copy
		
		after_copy:
		#if defined(CONFIG_ENABLE_MMU)
		enable_mmu:
			/* enable domain access */
			ldr	r5, =0x0000ffff
			mcr	p15, 0, r5, c3, c0, 0		@load domain access register
		
			/* Set the TTB register */
			ldr	r0, _mmu_table_base
			ldr	r1, =CFG_PHY_UBOOT_BASE
			ldr	r2, =0xfff00000
			bic	r0, r0, r2
			orr	r1, r0, r1
			mcr	p15, 0, r1, c2, c0, 0
		
			/* Enable the MMU */
		mmu_on:
			mrc	p15, 0, r0, c1, c0, 0
			orr	r0, r0, #1
			mcr	p15, 0, r0, c1, c0, 0
			nop
			nop
			nop
			nop
		#endif
		
		#ifdef CONFIG_EVT1
			/* store DMC density information in u-boot C level variable */
			ldr	r0, = CFG_UBOOT_BASE
			sub	r0, r0, #4
			ldr	r1, [r0]
			ldr	r0, _dmc_density
			str	r1, [r0]
		#endif
		
		skip_hw_init:
		/* Set up the stack						    */
		stack_setup:
		#if defined(CONFIG_MEMORY_UPPER_CODE)
			ldr	sp, =(CFG_UBOOT_BASE + CFG_UBOOT_SIZE - 0x1000)
		#else
			...............		
		#endif
		
		clear_bss:
			ldr	r0, _bss_start		/* find start of bss segment        */
			ldr	r1, _bss_end		/* stop here                        */
			mov 	r2, #0x00000000		/* clear                            */
		
		clbss_l:
			str	r2, [r0]		/* clear loop...                    */
			add	r0, r0, #4
			cmp	r0, r1
			ble	clbss_l
			
			ldr	pc, _start_armboot
		
		_start_armboot:
			.word start_armboot
------------------------------------------
lowlevel_init分析
------------------------------------------
/*
	此前board/samsung/smdkc210/lowlevel_init_SCP.S设置为链接，所以我们要看此文件。
	打开board/samsung/smdkc210/lowlevel_init_SCP.S：
*/
.globl lowlevel_init
lowlevel_init:
#if 1//*****ly
	/* use iROM stack in bl2 */
	ldr	sp, =0x02060000
#endif
	push	{lr}

	/* check reset status  */
	ldr     r0, =(INF_REG_BASE + INF_REG1_OFFSET)
        ldr     r1, [r0]

	/* AFTR wakeup reset */
	ldr	r2, =S5P_CHECK_DIDLE
	cmp	r1, r2
	beq	exit_wakeup

	/* Sleep wakeup reset */
	ldr	r2, =S5P_CHECK_SLEEP
	cmp	r1, r2
	beq	wakeup_reset

        /* PS-Hold high */
        ldr r0, =0x1002330c
        ldr r1, [r0]
        orr r1, r1, #0x300
        str r1, [r0]

        ldr     r0, =0x11000c08
        ldr r1, =0x0
        str r1, [r0]

        /* Clear  MASK_WDT_RESET_REQUEST  */
        ldr r0, =0x1002040c
        ldr r1, =0x00
        str r1, [r0]
        
#ifdef check_mem /*liyang 20110822*/
	/* when we already run in ram, we don't need to relocate U-Boot.
	 * and actually, memory controller must be configured before U-Boot
	 * is running in ram.
	 *如果已经运行在ram上就跳过时钟内存初始化；这里我们现在还没有运行在ram上，不跳
	 */
	ldr	r0, =0xff000fff
	bic	r1, pc, r0		/* r0 <- current base addr of code */
	ldr	r2, _TEXT_BASE		/* r1 <- original base addr in ram */
	bic	r2, r2, r0		/* r0 <- current base addr of code */
	cmp     r1, r2                  /* compare r0, r1                  */
	beq     1f			/* r0 == r1 then skip sdram init   */
#endif

	/* init system clock */
	bl system_clock_init_scp
		/*
			① MUX：多路复用，即从多个输入源中选择一个
			② PLL：把低频率的输入时钟提高后输出
			③ DIV：分频器，把高频率的输入时钟降频后输出
				system_clock_init_scp:
				push	{lr}
				
				ldr	r0, =ELFIN_CLOCK_BASE	@0x1003_0000  时钟相关寄存器基地址
				
				CMU_CPU有4个MUX，配置寄存器CLK_SRC_CPU，基地址0x1004_4200。配置完成后，最靠近PLL的两个MUX均切换时钟源至PLL。
			@ CMU_CPU MUX / DIV
				ldr	r1, =0x0										复位
				ldr	r2, =CLK_SRC_CPU_OFFSET
				str	r1, [r0, r2]
			
				ldr r2, =CLK_MUX_STAT_CPU_OFFSET
				ldr r3, =0x01110001
				bl wait_mux_state								读取状态寄存器潘丹复位是否完成，往后依次类似
			
				ldr	r1, =CLK_DIV_DMC0_VAL				设置CMU_DMC0的分频，CMU_CPU的分频CLK_DIV_CPU0和CLK_DIV_CPU1在后面设置的
				ldr	r2, =CLK_DIV_DMC0_OFFSET
				str	r1, [r0, r2]
				ldr	r1, =CLK_DIV_DMC1_VAL				设置CMU_DMC1的分频
				ldr	r2, =CLK_DIV_DMC1_OFFSET
				str	r1, [r0, r2]
			
			@ CMU_TOP MUX / DIV
				..................//省略，具体查看源代码和wz笔记
			
			@ CMU_LEFTBUS MUX / DIV
				..................//省略，具体查看源代码和wz笔记
			
			@ CMU_RIGHTBUS MUX / DIV
				..................//省略，具体查看源代码和wz笔记
			
			@ Set PLL locktime
				/4412有4个PLL：APLL, MPLL, EPLL, and VPLL。
				2个 PHY ：USB PHY 和 HDMI PHY (PHY：物理层， 一般指与外部信号接口的芯片 )：
				① APLL：用于 CPU_BLK ；作为 MPLL 的补充，它也可以给 DMC_BLK 、LEFTBUS_BLK 、RIGHTBUS_BLK 和 CMU_TOP 提供时钟。
				② MPLL：用于DMC_BLK 、LEFTBUS_BLK 、RIGHTBUS_BLK 和 CMU_TOP
				③ EPLL ：主要给音频模块提供时钟
				④ VPLL ：主要给视频系统提供54MHz时钟， 给 G3D(3D图形加速器 )提供时钟。
				⑤ USB PHY ：给 USB 子系统提供 30MHz 和 48MHz 时钟 。
				⑥ HDMI PHY ：产生 54MHz 时钟 。
				推荐使用24M外部晶振作为它们的时钟源。
				理论上讲，4个PLL均可以在22 ～ 1400MHz之间自由设置，但是手册强烈推荐的频率范围为：
			  APLL & MPLL：200 ～ 1400MHz
			  EPLL：90 ～ 416MHz
			  VPLL：100 ～ 440MHz/ 
			  
				ldr	r1, =APLL_LOCK_VAL
				ldr	r2, =APLL_LOCK_OFFSET
				str	r1, [r0, r2]
			
				ldr	r1, =MPLL_LOCK_VAL
				ldr	r2, =MPLL_LOCK_OFFSET
				str	r1, [r0, r2]
			
				ldr	r1, =EPLL_LOCK_VAL
				ldr	r2, =EPLL_LOCK_OFFSET
				str	r1, [r0, r2]
			
				ldr	r1, =VPLL_LOCK_VAL
				ldr	r2, =VPLL_LOCK_OFFSET
				str	r1, [r0, r2]
			
				ldr	r1, =CLK_DIV_CPU0_VAL
				ldr	r2, =CLK_DIV_CPU0_OFFSET
				str	r1, [r0, r2]
				ldr	r1, =CLK_DIV_CPU1_VAL
				ldr	r2, =CLK_DIV_CPU1_OFFSET
				str	r1, [r0, r2]
			
			@ Set APLL				
				..................//省略，具体查看源代码和wz笔记
			@ Set MPLL				
				..................
			@ Set EPLL
				..................			
			@ Set VPLL
				..................
			
			v310_2:
				pop	{pc}   返回，执行后面的mem_ctrl_asm_init_ddr3函数
			
			* end add */	
	/* Memory initialize */
	bl mem_ctrl_asm_init_ddr3
		/*
			Exynos4412中有两个独立的DRAM控制器，分别叫DMC0和DMC1,可以接2组不同的DDR内存。DMC0和DMC1分别支持最大1.5GB的DRAM，
			它们都支持DDR2/DDR3和LPDDR2等，512 Mb, 1 Gb, 2 Gb, 4 Gb and 8 Gbit的内存设备，支持16/32bit的位宽。
			
					.globl mem_ctrl_asm_init_ddr3        目录cpu/arm_cortexa9/s5pc210
			mem_ctrl_asm_init_ddr3:
				push {lr}
			
			*****************************************************************
			*DREX0***********************************************************
			*****************************************************************
			
				ldr	r0, =APB_DMC_0_BASE 
			
				ldr	r1, =0x0
				str	r1, [r0, #DMC_PHYCONTROL2]
			
				ldr	r1, =0x0
				str	r1, [r0, #0x24]
			
				ldr	r1, =0xE3855503
				str	r1, [r0, #DMC_PHYZQCONTROL]
			
				ldr	r1, =0x71101008				
				str	r1, [r0, #DMC_PHYCONTROL0]
			
				ldr	r1, =0x7110100A				
				str	r1, [r0, #DMC_PHYCONTROL0]
			
				ldr	r1, =0x20000086				
				str	r1, [r0, #DMC_PHYCONTROL1]
			
				ldr	r1, =0x71101008				
				str	r1, [r0, #DMC_PHYCONTROL0]
			
				ldr	r1, =0x2000008E				
				str	r1, [r0, #DMC_PHYCONTROL1]
			
				ldr	r1, =0x20000086				
				str	r1, [r0, #DMC_PHYCONTROL1]
			
				ldr	r1, =0x2000008E				
				str	r1, [r0, #DMC_PHYCONTROL1]
			
				ldr	r1, =0x20000086				
				str	r1, [r0, #DMC_PHYCONTROL1]
			
				ldr	r1, =0x0FFF30CA
				str	r1, [r0, #DMC_CONCONTROL]
			
				ldr	r1, =0x00302600				
				str	r1, [r0, #DMC_MEMCONTROL]		Burst宽为8,32-bit，类型为DDR3
					
				/* 2Gb * 8 *
				ldr r1, =0x40801333
				str	r1, [r0, #DMC_MEMCONFIG0]		8 banks，row 15bits，col 10bits，chip_base=40表示从40000000开始
				
			#endif
				ldr	r1, =(0x80000000 | CONFIG_IV_SIZE)
				str	r1, [r0, #DMC_IVCONTROL]
			
				ldr	r1, =0x64000000			
				str	r1, [r0, #DMC_PRECHCONFIG]
			
				ldr	r1, =0x9C4000FF			
				str	r1, [r0, #DMC_PHYCONTROL0]
			
				ldr	r1, =0x000000BB
				str	r1, [r0, #DMC_TIMINGAREF] @TimingAref
			
			#ifdef CONFIG_EVT0_RECOMMEND
				ldr	r1, =0x34A98691
			#else	
				ldr	r1, =0x34498691  
			#endif
			
				ldr	r1, =0x7846654F			//*0x4046654F
				str	r1, [r0, #DMC_TIMINGROW] @TimingRow
				ldr	r1, =0x46400506    				
				str	r1, [r0, #DMC_TIMINGDATA] @TimingData
				ldr	r1, =0x52000A3C    				
				str	r1, [r0, #DMC_TIMINGPOWER] @TimingPower
			
				/* minimum wait time is 100 nano seconds *
				/* 0x64: wait 250 nano seconds at ARMCLK 1.5 Ghz *
				mov	r2, #0x64
				bl dmc_delay
			
				ldr	r1, =0x07000000       				
				str	r1, [r0, #DMC_DIRECTCMD] 
			
				/* minimum wait time is 200 micro seconds *
				/* 0x19000: wait 250 micro seconds at ARMCLK 1.5 Ghz *
				mov	r2, #0x19000
				bl dmc_delay
			
				ldr	r1, =0x00020000       				
				str	r1, [r0, #DMC_DIRECTCMD]
			
				/* minimum wait time is 20 micro seconds *
				/* 0x2700: wait 25 micro seconds at ARMCLK 1.5 Ghz *
				mov	r2, #0x2700
				bl dmc_delay
			
				ldr	r1, =0x00030000       				
				str	r1, [r0, #DMC_DIRECTCMD] 
			
				/* minimum wait time is 1 micro seconds *
				/* 0x3f0: wait 2.5 micro seconds at ARMCLK 1.5 Ghz *
				mov	r2, #0x3f0
				bl dmc_delay
			
				ldr	r1, =0x00010000
				str	r1, [r0, #DMC_DIRECTCMD] 
				ldr	r1, =0x00000100       				
				str	r1, [r0, #DMC_DIRECTCMD] 
			
				mov	r2, #0x3f0
				bl dmc_delay
			
				ldr	r1, =0x00000420       				
				str	r1, [r0, #DMC_DIRECTCMD]
			
				mov	r2, #0x3f0
				bl dmc_delay
			
				ldr	r1, =0x0A000000
				str	r1, [r0, #DMC_DIRECTCMD]
			
				mov	r2, #0x3f0
				bl dmc_delay
			
			/*****************************************************************/
			/*DREX1***********************************************************/
			/*****************************************************************/
				..................//省略，具体查看源代码和wz笔记
				pop {pc}
			/* end modify *
		*/
	bl tzpc_init
		/*
			.......................//代码就省略了
			TZPC提供了将内存区域划分为secure和non-secure的软件接口，有两种办法可以做到。
			
			1.Programmable protection bits that can be allocated to areas of memory as
			determined by an external decoder.
			设置地址解码器所指定的内存区域的保护位（通过TZPCDECPROT）
			2.Programmable region size value for use by an AXI TrustZone Memory Adapter
			(TZMA). You can use this to split the RAM into two regions:
			— one secure
			— one non-secure.
			设置TZMA所使用的内存区大小，可以分割RAM为两个区域：一个secure，一个non-secure （通过TZPCR0SIZE）
		*/
	b	1f
	............//（省略一些未运行到的代码）
1:

#ifdef CONFIG_EVT1___
	/* store DMC density information in DRAM */
	/* mem_ctrl_asm_init returns dmc_density in r6 */
	ldr	r0, =CFG_UBOOT_BASE
	sub	r0, r0, #4
	str	r6, [r0]
#endif

out:
	/* for UART 
	uart_asm_init: Initialize UART in asm mode, 115200bps fixed.
 	* void uart_asm_init(void)*/
	bl uart_asm_init

	/* Print 'K' */
	ldr	r0, =ELFIN_UART_CONSOLE_BASE
	ldr	r1, =0x4b4b4b4b
	str	r1, [r0, #UTXH_OFFSET]

	/* 2010.08.27 by icarus : for temporary 3D clock fix */
	ldr	r1, =0x1
	ldr	r2, =0x1003C22C
	str	r1, [r2]
	ldr	r1, =0x2
	ldr	r2, =0x1003C52C
	str	r1, [r2]

	/* 2010.10.17 by icarus : for temporary MFC clock fix */
	ldr	r1, =0x3
	ldr	r2, =0x1003C528
	str	r1, [r2]

	pop	{pc}     
//之前push	{lr}入栈，这里又出栈存放到PC，相当于跳回start.S，由于这里很多函数用到了lr，所以不能用bl或mov指令返回
/*--------------------------------------------------------------------------------------------------------
继续从start_armboot分析
--------------------------------------------------------------------------------------------------------*/
	/*
		init_sequence执行一系列初始化函数以实现后半部分板级初始化，设置机器ID之类的
		比如：
		board_init();里设置gd->bd->bi_arch_number = MACH_TYPE;机器ID
		env_init,->
			//配置文件中定义了CONFIG_ENV_IS_IN_AUTO
			//common目录Makefile:COBJS-$(CONFIG_ENV_IS_IN_AUTO) += env_auto.o;使用common/Env_auto.c
			gd->env_addr  = (ulong)&default_environment[0];		
				uchar default_environment[] = {
				#ifdef	CONFIG_BOOTARGS
					"bootargs="	CONFIG_BOOTARGS			"\0"
				#endif
				#ifdef	CONFIG_BOOTCOMMAND
					"bootcmd="	CONFIG_BOOTCOMMAND		"\0"
				#endif
				............
				}
		init_baudrate,		/* initialze baudrate settings *
		serial_init,		/* serial communications setup *
		console_init_f,		/* stage 1 init of console *
		......
	*/
	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}
	................
	
	#ifdef CONFIG_GENERIC_MMC
	puts ("MMC:   ");
	mmc_exist = mmc_initialize (gd->bd);
		/*
			mmc_initialize->
				cpu_mmc_init(bis);->
					#if defined(CONFIG_S3C_HSMMC) || defined(CONFIG_S5P_MSHC)   //CONFIG_S3C_HSMMC在配置文件中定义了，所以这段代码有效
					setup_hsmmc_clock();->
						设置CLK_SRC_FSYS为 0x00000600，也就是说 MMC2 clock src = SCLKMPLL
						设置CLK_DIV_FSYS2，设置分频系数。假设为0x09010901，这样在MPLL为800MHz的情况下，先二分频再十分频，最后SD的时钟是40MHz。
						
						设置CLK_SRC_FSYS为 0x00060000，也就是说 MMC4 clock src = SCLKMPLL
						设置CLK_DIV_FSYS3，设置分频系数。假设为0x09010901，这样在MPLL为800MHz的情况下，先二分频再十分频，最后eMMC控制器的时钟是40MHz。
					setup_hsmmc_cfg_gpio();    //初始化gpio端口，设置为某个复用的功能
					#endif
					#ifdef USE_MMC4					//MMC4是GPK0_0~GPK0_6以及GPK1_0~GPK1_6，也可以说包括了SD_0和SD_1。
						ret = smdk_s5p_mshc_init();		//mmc = &mmc_channel[channel];并设置mmc->init,set_ios,send_cmd函数，以及emmc相关控制器的寄存器的基地址
					#endif
					//MMC0是GPK0_0~GPK0_6,也就是SD_0。MMC1是GPK1_0~GPK1_6,也就是SD_1。MMC3是GPK3_0~GPK2_6,也就是SD_3。
					#ifdef USE_MMC2					//MMC2是GPK2_0~GPK2_6,也就是SD_2。
						ret = smdk_s3c_hsmmc_init();	//mmc = &mmc_channel[channel];并设置mmc->init,set_ios,send_cmd函数，以及sd相关控制器的寄存器的基地址
					#endif
					return ret;
				for (dev = 0; dev < cur_dev_num; dev++)
				{
					mmc = find_mmc_device(dev);		//查找mmc设备
					if (mmc) 
					{
						err = mmc_init(mmc);->
							err = host->init(host);初始化，host是emmc会调用之前设置好的emmc对应的初始化函数,SD卡也会调用对应的init函数
							err = mmc_go_idle(host);设置cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;相当于发送了CMD0命令，实现复位
							...............//省略一些SD卡相关的函数
							mmc_startup(host);->
								err = mmc_send_cmd(host, &cmd, NULL);	//使用该函数依次发送CMD2,CMD3,CMD9,CMD7
								err = mmc_read_ext_csd(host);//查一下Extended CSD Register 的 [192] 字节，读取Revision
								err = mmc_change_freq(host); //查CSD寄存器[196]字节，获取type并选择时钟模式，对照后0 1 2 位为1，即支持HS52-DDR HS52 HS26
								mmc_set_ios(host);
								init_raw_area_table(&host->block_dev);//设置分区参数
					}
				}
		*/
	if (mmc_exist != 0)
	{
		puts ("0 MB\n");
	}
	#endif
	.............//省略，具体去看源代码
	board_late_init ();
		/*
			int ret=check_bootmode();->
				boot_dev = chk_bootdev();->
					sprintf(run_cmd,"emmc open 0");
					run_command(run_cmd, 0);
		
					//sprintf(run_cmd,"mmc read 0 0x40008000 0x30 0x20");
					sprintf(run_cmd,"mmc read 0 %lx %lx %lx",CFG_PHY_KERNEL_BASE,start_blk,blkcnt);//不太明白为什么要运行这句
					run_command(run_cmd, 0);

					//* switch mmc to normal paritition *
					sprintf(run_cmd,"emmc close 0");
					run_command(run_cmd, 0);
					return 0;
				...............
			sprintf(boot_cmd, "movi read kernel 40008000;movi read rootfs 40df0000 100000;bootm 40008000 40df0000");
			setenv("bootcmd", boot_cmd);//设置bootcmd
		*/
	main_loop ();
/*------------------------------------------
继续从main_loop分析
------------------------------------------*/
....................//省略，具体看源代码
s = getenv ("bootcmd");
/*
之前设置了bootcmd参数
U_BOOT_CMD(
		movi,	5,	0,	do_movi,
		"movi read  {u-boot | kernel} {addr} - Read data from sd/mmc\n"
		.............
	  );
分析do_movi函数
*/
	cmd = argv[1];
	switch (cmd[0]) {
	case 'r':
		rw = 0;	/* read case */
		break;
	}
	cmd = argv[2];
	switch (cmd[0]) {
	case 'k':
		if (argc != 4)
			goto usage;
		attribute = 0x4;
		addr = simple_strtoul(argv[3], NULL, 16);
		break;
	}
	if(OmPin == BOOT_EMMC441)
	{
		boot_dev = 0;		
	}
	/* kernel r/w */
	if (attribute == 0x4) {
		for (i=0, image = raw_area_control.image; i<15; i++) {
			if (image[i].attribute == attribute)
				break;
		}
		start_blk = image[i].start_blk;
		blkcnt = image[i].used_blk;
		printf("%s kernel.. %ld, %ld ", rw ? "writing" : "reading",
				start_blk, blkcnt);
		if (1 == fuse_by_fastboot)
			/*run_cmd=mmc read 0 addr, start_blk, blkcnt */
			sprintf(run_cmd,"mmc %s %d 0x%lx 0x%lx 0x%lx",
						rw ? "write":"read",boot_dev,
						addr, start_blk, blkcnt);
		
		run_command(run_cmd, 0);
		printf("completed\n");
		return 1;
	}
/*
之前设置了mmc参数
U_BOOT_CMD(
	mmc, 6, 1, do_mmcops,
	"read <device num> addr blk# cnt\n"
	............
	);

分析do_mmcops函数
*/
 	if (strcmp(argv[1], "read") == 0) {
		int dev = simple_strtoul(argv[2], NULL, 10);
		void *addr = (void *)simple_strtoul(argv[3], NULL, 16);
		u32 cnt = simple_strtoul(argv[5], NULL, 16);
		u32 blk = simple_strtoul(argv[4], NULL, 16);
		u32 n;

		struct mmc *mmc = find_mmc_device(dev);
	
		if (!mmc)
			return 1;

		printf("\nMMC read: dev # %d, block # %d, count %d ...",
			dev, blk, cnt);

		n = mmc->block_dev.block_read(dev, blk, cnt, addr);

		/* flush cache after read */
		flush_cache((ulong)addr, cnt * 512); /* FIXME */

		printf("%d blocks read: %s\n",
			n, (n==cnt) ? "OK" : "ERROR");
		return (n == cnt) ? 0 : 1;
	}
/*
之前设置了bootm参数
U_BOOT_CMD(
	bootm,	CONFIG_SYS_MAXARGS,	1,	do_bootm,
	"boot application image from memory",
	"[addr [arg ...]]\n    - boot application image stored in memory\n"
	"\tpassing arguments 'arg ...'; when booting a Linux kernel,\n"
	"\t'arg' can be the address of an initrd image\n"
	......................
	);

分析do_bootm函数
*/
int do_bootm (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#ifdef CONFIG_ZIMAGE_BOOT
#define LINUX_ZIMAGE_MAGIC	0x016f2818
	image_header_t	*hdr;
	ulong		addr;

	/* find out kernel image address */
	if (argc < 2) {
		addr = load_addr;
		debug ("*  kernel: default image load address = 0x%08lx\n",
				load_addr);
	} else {
		addr = simple_strtoul(argv[1], NULL, 16);
	}

	if (*(ulong *)(addr + 9*4) == LINUX_ZIMAGE_MAGIC) {
		u32 val;
		printf("Boot with zImage\n");

		//addr = virt_to_phys(addr);
		hdr = (image_header_t *)addr;
		hdr->ih_os = IH_OS_LINUX;
		hdr->ih_ep = ntohl(addr);
		
		memmove (&images.legacy_hdr_os_copy, hdr, sizeof(image_header_t));

		/* save pointer to image header */
		images.legacy_hdr_os = hdr;

		images.legacy_hdr_valid = 1;

		goto after_header_check;
	}
#endif

	if (bootm_start(cmdtp, flag, argc, argv))
		return 1;

	/*
	 * We have reached the point of no return: we are going to
	 * overwrite all exception vector code, so we cannot easily
	 * recover from any failures any more...
	 */
	iflag = disable_interrupts();

	ret = bootm_load_os(images.os, &load_end, 1);

	lmb_reserve(&images.lmb, images.os.load, (load_end - images.os.load));

	show_boot_progress (8);

#if defined(CONFIG_ZIMAGE_BOOT)
after_header_check:
	images.os.os = hdr->ih_os;
	images.ep = image_get_ep (&images.legacy_hdr_os_copy);
#endif

	boot_fn = boot_os[images.os.os];
		/*
			#ifdef CONFIG_BOOTM_LINUX
				[IH_OS_LINUX] = do_bootm_linux,
			#endif
			...........
		*/
	
	arch_preboot_os();

	boot_fn(0, argc, argv, &images);

	return 1;
}
/*------------------------------------------
do_bootm_linux分析
------------------------------------------*/
int do_bootm_linux(int flag, int argc, char *argv[], bootm_headers_t *images)
{
	int	machid = bd->bi_arch_number;
	void	(*theKernel)(int zero, int arch, uint params);
	
	#ifdef CONFIG_CMDLINE_TAG
		char *commandline = getenv ("bootargs");
	#endif
	
	theKernel = (void (*)(int, int, uint))images->ep;

	s = getenv ("machid");
	
	ret = boot_get_ramdisk(argc, argv, images, IH_ARCH_ARM, 
			&(images->rd_start), &(images->rd_end));
	if(ret)
		printf("[err] boot_get_ramdisk\n");

	show_boot_progress (15);

	debug ("## Transferring control to Linux (at address %08lx) ...\n",
	       (ulong) theKernel);
	/*设置参数，这是十分关键的一步*/
	#if defined (CONFIG_SETUP_MEMORY_TAGS) || \
    defined (CONFIG_CMDLINE_TAG) || \
    defined (CONFIG_INITRD_TAG) || \
    defined (CONFIG_SERIAL_TAG) || \
    defined (CONFIG_REVISION_TAG) || \
    defined (CONFIG_LCD) || \
    defined (CONFIG_VFD)
	setup_start_tag (bd);
	#ifdef CONFIG_SERIAL_TAG
		setup_serial_tag (&params);
	#endif
	#ifdef CONFIG_REVISION_TAG
		setup_revision_tag (&params);
	#endif
	#ifdef CONFIG_SETUP_MEMORY_TAGS
		setup_memory_tags (bd);
	#endif
	#ifdef CONFIG_CMDLINE_TAG
		setup_commandline_tag (bd, commandline);
	#endif
	#ifdef CONFIG_INITRD_TAG
		if (images->rd_start && images->rd_end)
			setup_initrd_tag (bd, images->rd_start, images->rd_end);
	#endif
	#if defined (CONFIG_VFD) || defined (CONFIG_LCD)
		setup_videolfb_tag ((gd_t *) gd);
	#endif
		setup_end_tag (bd);
	#endif

	/* we assume that the kernel is in place */
	printf ("\nStarting kernel ...\n\n");
	cleanup_before_linux ();

	//启动kernel
	theKernel (0, machid, bd->bi_boot_params);
	/* does not return */

	return 1;
}


















































