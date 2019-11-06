/*
�����������ù��̣�
ִ��build_uboot.sh SCP_1GDDR
*����build_uboot.sh�ļ�
if [ "$1" = "SCP_1GDDR" ]
	then
	make itop_4412_android_config_scp_1GDDR
*�򿪶���Makefile����Ŀ�������
itop_4412_android_config_scp_1GDDR:		unconfig
	@$(MKCONFIG) $(@:_config=) arm arm_cortexa9 smdkc210 samsung s5pc210 SCP_1GDDR
	
MKCONFIG	:= $(SRCTREE)/mkconfig
��ʵִ��Make itop_4412_android_config_scp_1GDDR������������൱��ִ����
mkconfig   itop_4412_android  arm arm_cortexa9 smdkc210 samsung s5pc210 SCP_1GDDR����ָ�
*��mkconfig�ļ�����������:	
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
Ҳ����˵ARCH="arm" CPU="arm_cortexa9" BOARD="smdkc210" VENDOR="samsung" SOC="s5pc210" 

*�����Makefile���滹���������
# load other configuration
include $(TOPDIR)/config.mk   �������˶����config.mk
*�򿪶����config.mk�ļ�������ôһ��
ifdef	ARCH
sinclude $(TOPDIR)/lib_$(ARCH)/config.mk	# include architecture dependend rules
endif
Ҳ����˵������uboot/lib_arm/config.mk
����Ҳ������������ļ��������Կ���:
LDSCRIPT := $(SRCTREE)/cpu/$(CPU)/u-boot.lds
���ǿ��Ե�֪���ӽű�lds�ļ���uboot/cpu/arm_cortexa9/u-boot.lds


�鿴���ӽű�
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
���ӽű�����Щ��Ķ�����linkage.h�У���������˼Ҳ���ף�������������_start.��������text�Σ�data�εȡ�
_start��cpu/arm_cortexa9/start.S�У�һ��һ�εķ��������£�
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
	 *����SVC32����ģʽ
	 */
	mrs	r0, cpsr
	bic	r0, r0, #0x3f
	orr	r0, r0, #0xd3
	msr	cpsr, r0
#else//*****ly ִ�д˶δ���
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
	 *�ر�MMU
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002000	@ clear bits 13 (--V-)
	bic	r0, r0, #0x00000007	@ clear bits 2:0 (-CAM)
	orr	r0, r0, #0x00001000	@ set bit 12 (---I) Icache
	orr	r0, r0, #0x00000002	@ set bit 1  (--A-) Align
	orr	r0, r0, #0x00000800	@ set bit 11 (Z---) BTB
	mcr	p15, 0, r0, c1, c0, 0
#endif

/* Read booting information ��ȡ������Ϣ*/
        ldr	r0, =POWER_BASE
        ldr	r1, [r0,#OMR_OFFSET]
        bic	r2, r1, #0xffffffc1

	/* NAND BOOT */
@	cmp	r2, #0x0		@ 512B 4-cycle
@	moveq	r3, #BOOT_NAND
...................//(�˴�����ʡ�ԣ����ǹ���NAND�����ģ�������eMMC����������)
	/* SD/MMC BOOT */
	......
	/* eMMC4.3 BOOT */
	.......
	/* eMMC441 BOOT */   //����ʹ�õ�����������Ľ�����eMMc�������������Ī���ң���~~
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
	bl	lowlevel_init	/* go setup pll,mux,memory ʱ�ӡ��ڴ桢���ڵȳ�ʼ��*/
	
	ldr	r0, =0x1002330C  /* PS_HOLD_CONTROL register ���ڹ���������PS_HOLD*/
	ldr	r1, =0x00005300	 /* PS_HOLD output high	*/
	str	r1, [r0]


	/* get ready to call C functions */
	ldr	sp, _TEXT_PHY_BASE	/* setup temp stack pointer */
	sub	sp, sp, #12
	mov	fp, #0			/* no previous frame, so fp=0 */
	
	/* when we already run in ram, we don't need to relocate U-Boot.
	 * and actually, memory controller must be configured before U-Boot
	 * is running in ram.
	 *���Ѿ�������ram�ϣ��������ض�λ�Ĵ��롣�������ǻ�û��������ram�ϣ����Բ�����after_copy
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
				���Կ��ĳ�copy_bl2�Ǹ�ָ��ĳ���̶���ַ�ĺ��������Բ²���������ǹ̻���IROM�С�
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
lowlevel_init����
------------------------------------------
/*
	��ǰboard/samsung/smdkc210/lowlevel_init_SCP.S����Ϊ���ӣ���������Ҫ�����ļ���
	��board/samsung/smdkc210/lowlevel_init_SCP.S��
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
	 *����Ѿ�������ram�Ͼ�����ʱ���ڴ��ʼ���������������ڻ�û��������ram�ϣ�����
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
			�� MUX����·���ã����Ӷ������Դ��ѡ��һ��
			�� PLL���ѵ�Ƶ�ʵ�����ʱ����ߺ����
			�� DIV����Ƶ�����Ѹ�Ƶ�ʵ�����ʱ�ӽ�Ƶ�����
				system_clock_init_scp:
				push	{lr}
				
				ldr	r0, =ELFIN_CLOCK_BASE	@0x1003_0000  ʱ����ؼĴ�������ַ
				
				CMU_CPU��4��MUX�����üĴ���CLK_SRC_CPU������ַ0x1004_4200��������ɺ����PLL������MUX���л�ʱ��Դ��PLL��
			@ CMU_CPU MUX / DIV
				ldr	r1, =0x0										��λ
				ldr	r2, =CLK_SRC_CPU_OFFSET
				str	r1, [r0, r2]
			
				ldr r2, =CLK_MUX_STAT_CPU_OFFSET
				ldr r3, =0x01110001
				bl wait_mux_state								��ȡ״̬�Ĵ����˵���λ�Ƿ���ɣ�������������
			
				ldr	r1, =CLK_DIV_DMC0_VAL				����CMU_DMC0�ķ�Ƶ��CMU_CPU�ķ�ƵCLK_DIV_CPU0��CLK_DIV_CPU1�ں������õ�
				ldr	r2, =CLK_DIV_DMC0_OFFSET
				str	r1, [r0, r2]
				ldr	r1, =CLK_DIV_DMC1_VAL				����CMU_DMC1�ķ�Ƶ
				ldr	r2, =CLK_DIV_DMC1_OFFSET
				str	r1, [r0, r2]
			
			@ CMU_TOP MUX / DIV
				..................//ʡ�ԣ�����鿴Դ�����wz�ʼ�
			
			@ CMU_LEFTBUS MUX / DIV
				..................//ʡ�ԣ�����鿴Դ�����wz�ʼ�
			
			@ CMU_RIGHTBUS MUX / DIV
				..................//ʡ�ԣ�����鿴Դ�����wz�ʼ�
			
			@ Set PLL locktime
				/4412��4��PLL��APLL, MPLL, EPLL, and VPLL��
				2�� PHY ��USB PHY �� HDMI PHY (PHY������㣬 һ��ָ���ⲿ�źŽӿڵ�оƬ )��
				�� APLL������ CPU_BLK ����Ϊ MPLL �Ĳ��䣬��Ҳ���Ը� DMC_BLK ��LEFTBUS_BLK ��RIGHTBUS_BLK �� CMU_TOP �ṩʱ�ӡ�
				�� MPLL������DMC_BLK ��LEFTBUS_BLK ��RIGHTBUS_BLK �� CMU_TOP
				�� EPLL ����Ҫ����Ƶģ���ṩʱ��
				�� VPLL ����Ҫ����Ƶϵͳ�ṩ54MHzʱ�ӣ� �� G3D(3Dͼ�μ����� )�ṩʱ�ӡ�
				�� USB PHY ���� USB ��ϵͳ�ṩ 30MHz �� 48MHz ʱ�� ��
				�� HDMI PHY ������ 54MHz ʱ�� ��
				�Ƽ�ʹ��24M�ⲿ������Ϊ���ǵ�ʱ��Դ��
				�����Ͻ���4��PLL��������22 �� 1400MHz֮���������ã������ֲ�ǿ���Ƽ���Ƶ�ʷ�ΧΪ��
			  APLL & MPLL��200 �� 1400MHz
			  EPLL��90 �� 416MHz
			  VPLL��100 �� 440MHz/ 
			  
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
				..................//ʡ�ԣ�����鿴Դ�����wz�ʼ�
			@ Set MPLL				
				..................
			@ Set EPLL
				..................			
			@ Set VPLL
				..................
			
			v310_2:
				pop	{pc}   ���أ�ִ�к����mem_ctrl_asm_init_ddr3����
			
			* end add */	
	/* Memory initialize */
	bl mem_ctrl_asm_init_ddr3
		/*
			Exynos4412��������������DRAM���������ֱ��DMC0��DMC1,���Խ�2�鲻ͬ��DDR�ڴ档DMC0��DMC1�ֱ�֧�����1.5GB��DRAM��
			���Ƕ�֧��DDR2/DDR3��LPDDR2�ȣ�512 Mb, 1 Gb, 2 Gb, 4 Gb and 8 Gbit���ڴ��豸��֧��16/32bit��λ��
			
					.globl mem_ctrl_asm_init_ddr3        Ŀ¼cpu/arm_cortexa9/s5pc210
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
				str	r1, [r0, #DMC_MEMCONTROL]		Burst��Ϊ8,32-bit������ΪDDR3
					
				/* 2Gb * 8 *
				ldr r1, =0x40801333
				str	r1, [r0, #DMC_MEMCONFIG0]		8 banks��row 15bits��col 10bits��chip_base=40��ʾ��40000000��ʼ
				
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
				..................//ʡ�ԣ�����鿴Դ�����wz�ʼ�
				pop {pc}
			/* end modify *
		*/
	bl tzpc_init
		/*
			.......................//�����ʡ����
			TZPC�ṩ�˽��ڴ����򻮷�Ϊsecure��non-secure������ӿڣ������ְ취����������
			
			1.Programmable protection bits that can be allocated to areas of memory as
			determined by an external decoder.
			���õ�ַ��������ָ�����ڴ�����ı���λ��ͨ��TZPCDECPROT��
			2.Programmable region size value for use by an AXI TrustZone Memory Adapter
			(TZMA). You can use this to split the RAM into two regions:
			�� one secure
			�� one non-secure.
			����TZMA��ʹ�õ��ڴ�����С�����Էָ�RAMΪ��������һ��secure��һ��non-secure ��ͨ��TZPCR0SIZE��
		*/
	b	1f
	............//��ʡ��һЩδ���е��Ĵ��룩
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
//֮ǰpush	{lr}��ջ�������ֳ�ջ��ŵ�PC���൱������start.S����������ܶຯ���õ���lr�����Բ�����bl��movָ���
/*--------------------------------------------------------------------------------------------------------
������start_armboot����
--------------------------------------------------------------------------------------------------------*/
	/*
		init_sequenceִ��һϵ�г�ʼ��������ʵ�ֺ�벿�ְ弶��ʼ�������û���ID֮���
		���磺
		board_init();������gd->bd->bi_arch_number = MACH_TYPE;����ID
		env_init,->
			//�����ļ��ж�����CONFIG_ENV_IS_IN_AUTO
			//commonĿ¼Makefile:COBJS-$(CONFIG_ENV_IS_IN_AUTO) += env_auto.o;ʹ��common/Env_auto.c
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
					#if defined(CONFIG_S3C_HSMMC) || defined(CONFIG_S5P_MSHC)   //CONFIG_S3C_HSMMC�������ļ��ж����ˣ�������δ�����Ч
					setup_hsmmc_clock();->
						����CLK_SRC_FSYSΪ 0x00000600��Ҳ����˵ MMC2 clock src = SCLKMPLL
						����CLK_DIV_FSYS2�����÷�Ƶϵ��������Ϊ0x09010901��������MPLLΪ800MHz������£��ȶ���Ƶ��ʮ��Ƶ�����SD��ʱ����40MHz��
						
						����CLK_SRC_FSYSΪ 0x00060000��Ҳ����˵ MMC4 clock src = SCLKMPLL
						����CLK_DIV_FSYS3�����÷�Ƶϵ��������Ϊ0x09010901��������MPLLΪ800MHz������£��ȶ���Ƶ��ʮ��Ƶ�����eMMC��������ʱ����40MHz��
					setup_hsmmc_cfg_gpio();    //��ʼ��gpio�˿ڣ�����Ϊĳ�����õĹ���
					#endif
					#ifdef USE_MMC4					//MMC4��GPK0_0~GPK0_6�Լ�GPK1_0~GPK1_6��Ҳ����˵������SD_0��SD_1��
						ret = smdk_s5p_mshc_init();		//mmc = &mmc_channel[channel];������mmc->init,set_ios,send_cmd�������Լ�emmc��ؿ������ļĴ����Ļ���ַ
					#endif
					//MMC0��GPK0_0~GPK0_6,Ҳ����SD_0��MMC1��GPK1_0~GPK1_6,Ҳ����SD_1��MMC3��GPK3_0~GPK2_6,Ҳ����SD_3��
					#ifdef USE_MMC2					//MMC2��GPK2_0~GPK2_6,Ҳ����SD_2��
						ret = smdk_s3c_hsmmc_init();	//mmc = &mmc_channel[channel];������mmc->init,set_ios,send_cmd�������Լ�sd��ؿ������ļĴ����Ļ���ַ
					#endif
					return ret;
				for (dev = 0; dev < cur_dev_num; dev++)
				{
					mmc = find_mmc_device(dev);		//����mmc�豸
					if (mmc) 
					{
						err = mmc_init(mmc);->
							err = host->init(host);��ʼ����host��emmc�����֮ǰ���úõ�emmc��Ӧ�ĳ�ʼ������,SD��Ҳ����ö�Ӧ��init����
							err = mmc_go_idle(host);����cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;�൱�ڷ�����CMD0���ʵ�ָ�λ
							...............//ʡ��һЩSD����صĺ���
							mmc_startup(host);->
								err = mmc_send_cmd(host, &cmd, NULL);	//ʹ�øú������η���CMD2,CMD3,CMD9,CMD7
								err = mmc_read_ext_csd(host);//��һ��Extended CSD Register �� [192] �ֽڣ���ȡRevision
								err = mmc_change_freq(host); //��CSD�Ĵ���[196]�ֽڣ���ȡtype��ѡ��ʱ��ģʽ�����պ�0 1 2 λΪ1����֧��HS52-DDR HS52 HS26
								mmc_set_ios(host);
								init_raw_area_table(&host->block_dev);//���÷�������
					}
				}
		*/
	if (mmc_exist != 0)
	{
		puts ("0 MB\n");
	}
	#endif
	.............//ʡ�ԣ�����ȥ��Դ����
	board_late_init ();
		/*
			int ret=check_bootmode();->
				boot_dev = chk_bootdev();->
					sprintf(run_cmd,"emmc open 0");
					run_command(run_cmd, 0);
		
					//sprintf(run_cmd,"mmc read 0 0x40008000 0x30 0x20");
					sprintf(run_cmd,"mmc read 0 %lx %lx %lx",CFG_PHY_KERNEL_BASE,start_blk,blkcnt);//��̫����ΪʲôҪ�������
					run_command(run_cmd, 0);

					//* switch mmc to normal paritition *
					sprintf(run_cmd,"emmc close 0");
					run_command(run_cmd, 0);
					return 0;
				...............
			sprintf(boot_cmd, "movi read kernel 40008000;movi read rootfs 40df0000 100000;bootm 40008000 40df0000");
			setenv("bootcmd", boot_cmd);//����bootcmd
		*/
	main_loop ();
/*------------------------------------------
������main_loop����
------------------------------------------*/
....................//ʡ�ԣ����忴Դ����
s = getenv ("bootcmd");
/*
֮ǰ������bootcmd����
U_BOOT_CMD(
		movi,	5,	0,	do_movi,
		"movi read  {u-boot | kernel} {addr} - Read data from sd/mmc\n"
		.............
	  );
����do_movi����
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
֮ǰ������mmc����
U_BOOT_CMD(
	mmc, 6, 1, do_mmcops,
	"read <device num> addr blk# cnt\n"
	............
	);

����do_mmcops����
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
֮ǰ������bootm����
U_BOOT_CMD(
	bootm,	CONFIG_SYS_MAXARGS,	1,	do_bootm,
	"boot application image from memory",
	"[addr [arg ...]]\n    - boot application image stored in memory\n"
	"\tpassing arguments 'arg ...'; when booting a Linux kernel,\n"
	"\t'arg' can be the address of an initrd image\n"
	......................
	);

����do_bootm����
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
do_bootm_linux����
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
	/*���ò���������ʮ�ֹؼ���һ��*/
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

	//����kernel
	theKernel (0, machid, bd->bi_boot_params);
	/* does not return */

	return 1;
}


















































