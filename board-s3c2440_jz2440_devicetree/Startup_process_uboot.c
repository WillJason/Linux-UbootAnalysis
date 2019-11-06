/*
�����������ù��̣�
ִ��make 100ask24x0_config   
*�򿪶���Makefile����Ŀ�������
100ask24x0_config	:	unconfig
	@$(MKCONFIG) $(@:_config=) arm arm920t 100ask24x0 NULL s3c24x0

MKCONFIG	:= $(SRCTREE)/mkconfig
��ʵִ��make 100ask24x0_config������������൱��ִ����
mkconfig 100ask24x0 arm arm920t 100ask24x0 NULL s3c24x0	����ָ��
*��mkconfig�ļ�����������:	
echo "ARCH   = $2" >  config.mk
echo "CPU    = $3" >> config.mk
echo "BOARD  = $4" >> config.mk
[ "$5" ] && [ "$5" != "NULL" ] && echo "VENDOR = $5" >> config.mk
[ "$6" ] && [ "$6" != "NULL" ] && echo "SOC    = $6" >> config.mk	
Ҳ����˵ARCH="arm" CPU="arm920t" BOARD="100ask24x0" VENDOR=NULL SOC="s3c24x0" 
	
*�����Makefile���滹���������	
# load other configuration
include $(TOPDIR)/config.mk	    �������˶����config.mk
*�򿪶����config.mk�ļ�������ô����
ifdef	ARCH
#//Ҳ����˵������uboot/arm_config.mk
sinclude $(TOPDIR)/$(ARCH)_config.mk	# include architecture dependend rules
endif
ifdef	CPU
#//Ҳ����˵������uboot/cpu/arm920t.mk
sinclude $(TOPDIR)/cpu/$(CPU)/config.mk	# include  CPU	specific rules
endif
ifdef	SOC
#//Ҳ����˵������uboot/cpu/arm920t/s3c24x0/config.mk
sinclude $(TOPDIR)/cpu/$(CPU)/$(SOC)/config.mk	# include  SoC	specific rules
endif
ifdef	VENDOR   			֮ǰ����ΪNULL��ִ��else
BOARDDIR = $(VENDOR)/$(BOARD)
else
BOARDDIR = $(BOARD)
endif
ifdef	BOARD
#//Ҳ����˵������uboot/board/100ask24x0/config.mk���򿪸��ļ�
#//����ôһ�䣺TEXT_BASE = 0x33F80000   ����˵���ӵ�ַΪ0x33F80000
sinclude $(TOPDIR)/board/$(BOARDDIR)/config.mk	# include board specific rules
endif	
�����¿�����ôһ�䣺
LDSCRIPT := $(TOPDIR)/board/$(BOARDDIR)/u-boot.lds	
���ǿ��Ե�֪���ӽű�lds�ļ���uboot/board/100ask24x0/u-boot.lds	

�鿴���ӽű�	
*/
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
/*OUTPUT_FORMAT("elf32-arm", "elf32-arm", "elf32-arm")*/
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text      :
	{
	  cpu/arm920t/start.o	(.text)
          board/100ask24x0/boot_init.o (.text)
	  *(.text)
	}

	. = ALIGN(4);
	.rodata : { *(.rodata) }

	. = ALIGN(4);
	.data : { *(.data) }

	. = ALIGN(4);
	.got : { *(.got) }

	. = .;
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
_start��cpu/arm920t/start.S�У�һ��һ�εķ��������£�
*/ 
.globl _start
_start:	b       reset
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq
/*
 * the actual reset code ����SVCģʽ
 */
reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0
/* turn off the watchdog */
# define pWTCON		0x53000000
# define INTMOD     0X4A000004
# define INTMSK		0x4A000008	/* Interupt-Controller base addresses */
# define INTSUBMSK	0x4A00001C
# define CLKDIVN	0x4C000014	/* clock divisor register */
#if defined(CONFIG_S3C2400) || defined(CONFIG_S3C2410)
	ldr     r0, =pWTCON
	mov     r1, #0x0
	str     r1, [r0]
	/*
	 * mask all IRQs by setting all bits in the INTMR - default
	 * default�����ж�
	 */
	mov	r1, #0xffffffff
	ldr	r0, =INTMSK
	str	r1, [r0]
# if defined(CONFIG_S3C2410)
	ldr	r1, =0x3ff
	ldr	r0, =INTSUBMSK
	str	r1, [r0]
# endif
/* ����ʱ��, ʹ�û�� */
#define S3C2440_MPLL_400MHZ     ((0x5c<<12)|(0x01<<4)|(0x01))
#define S3C2440_UPLL_48MHZ      ((0x38<<12)|(0x02<<4)|(0x02))
#define S3C2440_CLKDIV          (0x05) // | (1<<3))    /* FCLK:HCLK:PCLK = 1:4:8, UCLK = UPLL/2 */

	ldr r1, =CLKDIVN
	mov r2, #S3C2440_CLKDIV
	str r2, [r1]

	mrc p15, 0, r1, c1, c0, 0		// read ctrl register 
	orr r1, r1, #0xc0000000 		// Asynchronous	
	mcr p15, 0, r1, c1, c0, 0		// write ctrl register

    ldr r0,=LOCKTIME
    ldr r1,=0xffffff
    str r1,[r0]
    // delay
    mov     r0, #0x200
1:  subs    r0, r0, #1
    bne     1b

    // Configure MPLL   ����PLLʱ�����������Ϊ400Mʱ�ӣ������ٷ�Ƶ
    ldr r0,=MPLLCON          
    ldr r1,=S3C2440_MPLL_400MHZ
    str r1,[r0]
    // delay
    mov     r0, #0x200
1:  subs    r0, r0, #1
    bne     1b

    //Configure UPLL		2440�и�PLL��MPLL��UPLL
    ldr     r0, =UPLLCON          
    ldr     r1, =S3C2440_UPLL_48MHZ
    str     r1, [r0]
    // delay
    mov     r0, #0x200
1:  subs    r0, r0, #1
    bne     1b
    
/* 2. ���� GSTATUS2[1]�ж��Ǹ�λ���ǻ��� */	
	ldr r0, =GSTATUS2
	ldr r1, [r0]
	tst r1, #(1<<1)  /* r1 & (1<<1) */
	bne wake_up	
	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 *���������Ҵ�ʱ��������RAM�ϣ���Ҫϵͳ�ײ��ʼ��
	 */
	adr	r0, _start		/* r0 <- current position of code   */
	/*
	TEXT_BASE������u-boot-1.1.6/board/100ask24x0/config.mk��,Ϊ0x33F80000
	*/
	ldr	r1, _TEXT_BASE		/* test if we run from flash or RAM */
	cmp     r0, r1                  /* don't reloc during debug         */
	blne	cpu_init_crit
	/*
	cpu_init_crit->
		cpu_init_crit:
		/*
		 * flush v4 I/D caches
		 * /
		mov	r0, #0
		mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache * /
		mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB * /
	
		/*
		 * disable MMU stuff and caches
		 * /
		mrc	p15, 0, r0, c1, c0, 0
		bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
		bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
		orr	r0, r0, #0x00000002	@ set bit 2 (A) Align
		orr	r0, r0, #0x00001000	@ set bit 12 (I) I-Cache
		mcr	p15, 0, r0, c1, c0, 0
	
		/*
		 * before relocating, we have to setup RAM timing
		 * because memory timing is board-dependend, you will
		 * find a lowlevel_init.S in your board directory.
		 * /
		mov	ip, lr
		bl	lowlevel_init->
			/ *
			.globl lowlevel_init
			lowlevel_init:				
				ldr     r0, =SMRDATA
				ldr	r1, _TEXT_BASE
				sub	r0, r0, r1
				ldr	r1, =BWSCON	/ * Bus Width Status Controller * /
				add     r2, r0, #13*4
			0:
				ldr     r3, [r0], #4
				str     r3, [r1], #4
				cmp     r2, r0
				bne     0b
				/ * everything is fine now * /
				mov	pc, lr
							
				.ltorg
			/ * the literal pools origin * /
			SMRDATA:
			    .word (0+(B1_BWSCON<<4)+(B2_BWSCON<<8)+(B3_BWSCON<<12)+(B4_BWSCON<<16)+(B5_BWSCON<<20)+(B6_BWSCON<<24)+(B7_BWSCON<<28)) 
			    .word ((B0_Tacs<<13)+(B0_Tcos<<11)+(B0_Tacc<<8)+(B0_Tcoh<<6)+(B0_Tah<<4)+(B0_Tacp<<2)+(B0_PMC))
			    .word ((B1_Tacs<<13)+(B1_Tcos<<11)+(B1_Tacc<<8)+(B1_Tcoh<<6)+(B1_Tah<<4)+(B1_Tacp<<2)+(B1_PMC))
			    .word ((B2_Tacs<<13)+(B2_Tcos<<11)+(B2_Tacc<<8)+(B2_Tcoh<<6)+(B2_Tah<<4)+(B2_Tacp<<2)+(B2_PMC))
			    .word ((B3_Tacs<<13)+(B3_Tcos<<11)+(B3_Tacc<<8)+(B3_Tcoh<<6)+(B3_Tah<<4)+(B3_Tacp<<2)+(B3_PMC))
			    .word ((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC))
			    .word ((B5_Tacs<<13)+(B5_Tcos<<11)+(B5_Tacc<<8)+(B5_Tcoh<<6)+(B5_Tah<<4)+(B5_Tacp<<2)+(B5_PMC))
			    .word ((B6_MT<<15)+(B6_Trcd<<2)+(B6_SCAN))
			    .word ((B7_MT<<15)+(B7_Trcd<<2)+(B7_SCAN))
			    .word ((REFEN<<23)+(TREFMD<<22)+(Trp<<20)+(Trc<<18)+(Tchr<<16)+REFCNT)
			    .word 0xb1
			    .word 0x30
			    .word 0x30
			* /
		mov	lr, ip
		mov	pc, lr
	*/
	/* Set up the stack						    */
stack_setup:
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CFG_MALLOC_LEN	/* malloc area                      */
	sub	r0, r0, #CFG_GBL_DATA_SIZE /* bdinfo                        */
	
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ)
relocate:				/* relocate U-Boot to RAM	   �ض�λ����*/
	adr	r0, _start		/* r0 <- current position of code   */
	ldr	r1, _TEXT_BASE		/* test if we run from flash or RAM */
	cmp     r0, r1                  /* don't reloc during debug         */
	beq     clear_bss
	
	ldr	r2, _armboot_start
	ldr	r3, _bss_start
	sub	r2, r3, r2		/* r2 <- size of armboot            */
#if 1
	bl  CopyCode2Ram	/* r0: source, r1: dest, r2: size */
		/*
		 if (bBootFrmNORFlash())
    {		*�� NOR Flash���� *
        for (i = 0; i < size / 4; i++)
        {pdwDest[i] = pdwSrc[i];}return 0;
    }else
    {   *��ʼ��NAND Flash *
				nand_init_ll();->
					*����ʱ��*
	        s3c2440nand->NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
	        *ʹ��NAND Flash������, ��ʼ��ECC, ��ֹƬѡ*
	        s3c2440nand->NFCONT = (1<<4)|(1<<1)|(1<<0);
	        *��λNAND Flash*
					nand_reset();->
						s3c2440_nand_reset();->
							s3c2440_nand_select_chip();
					    s3c2440_write_cmd(0xff);  // ��λ����
					    s3c2440_wait_idle();
					    s3c2440_nand_deselect_chip();
        *�� NAND Flash���� *
        nand_read_ll_lp();->
        	 *ѡ��оƬ*
			     nand_select_chip();			
			     for(i=start_addr; i < (start_addr + size);) {
			      *����READ0���� *
			      write_cmd(0);			
			      * Write Address *
			      write_addr_lp(i);
				    write_cmd(0x30);
			      wait_idle();			
			      for(j=0; j < NAND_SECTOR_SIZE_LP; j++, i++) {
			           *buf = read_data();
			           buf++;
			      }
			     }			
			     *ȡ��Ƭѡ�ź� *
			     nand_deselect_chip();
		}
		*/
	
#else
clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment        */
	ldr	r1, _bss_end		/* stop here                        */
	mov 	r2, #0x00000000		/* clear                            */

clbss_l:str	r2, [r0]		/* clear loop...                    */
	add	r0, r0, #4
	cmp	r0, r1
	ble	clbss_l

SetLoadFlag:
	/* Set a global flag, PreLoadedONRAM */
	adr	r0, _start		/* r0 <- current position of code   */
	ldr	r1, _TEXT_BASE		/* test if we run from flash or RAM */
	cmp     r0, r1                  /* don't reloc during debug         */
	ldr r2, =PreLoadedONRAM
	mov r3, #1
	streq r3, [r2]

	ldr	pc, _start_armboot
_start_armboot:	.word start_armboot
/*----------------------------------------------------------------------------
��������lib_arm/board.c:start_armboot����
----------------------------------------------------------------------------*/
	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) 
	/*
	init_fnc_t *init_sequence[] = {
		board_init,->				//board/100ask24x0/100ask24x0.c
			/ * set up the I/O ports * /
			//* arch number of SMDK2440-Board *
       gd->bd->bi_arch_number = MACH_TYPE_S3C2440;
      //* adress of boot parameters *
    	gd->bd->bi_boot_params = 0x30000100;
		env_init,->					//common/env_nand.c
			//���������ENV_IS_EMBEDDED������������Ĵ���gd->env_addr��gd->env_valid
			gd->env_addr  = (ulong)&default_environment[0];->
				uchar default_environment[] = {
				#ifdef	CONFIG_BOOTARGS
					"bootargs="	CONFIG_BOOTARGS			"\0"
				#endif
				#ifdef	CONFIG_BOOTCOMMAND
					"bootcmd="	CONFIG_BOOTCOMMAND		"\0"
				#endif
				#ifdef	CONFIG_SERVERIP
					"serverip="	MK_STR(CONFIG_SERVERIP)		"\0"
				#endif
				#if defined(CONFIG_BAUDRATE) && (CONFIG_BAUDRATE >= 0)
					"baudrate="	MK_STR(CONFIG_BAUDRATE)		"\0"
				#endif
				.......................
				}
			gd->env_valid = 1;
		init_baudrate,
		...................
	}
	*/
#ifndef CFG_NO_FLASH
	/* configure available FLASH banks */
	size = flash_init ();->          //board/100ask24x0/flash.c ʶ���nor��С��nand����ʱΪ0
#endif 
	mem_malloc_init (_armboot_start - CFG_MALLOC_LEN);//��һ���ڴ��Ϸ���һЩ�ռ䣬��192�Ŀռ���ʵ�ֶѵĶѷź��ͷš�
	nand_init();->					//ʶ���nand��С��nor����ʱҲ��ʶ�����С�������ͻ�ʶ���������flash
		/*
		nand_init_chip()->
			board_nand_init(nand);->		//����nand_chip�ṹ��, �ṩ�ײ�Ĳ�������  cpu/arm920t/s3c24x0/nand_flash.c
				s3c24x0_nand_inithw();
				chip->IO_ADDR_R    = (void *)&s3c2440nand->NFDATA;
        chip->IO_ADDR_W    = (void *)&s3c2440nand->NFDATA;
        chip->hwcontrol    = s3c2440_nand_hwcontrol;
        chip->dev_ready    = s3c2440_nand_devready;
        chip->select_chip  = s3c2440_nand_select_chip;//nandflashѡ�еĻ���������ĳЩ�Ĵ������������ǲ���ʹ��Ĭ�ϵĺ���
        chip->options      = 0;
			nand_scan()->						//drivers/nand/nand_base.c
				this->cmdfunc = nand_command;			//���������������Ҳ�����������е�ַ(ҳ�ڵ�ַ)���е�ַ(��һҳ)
				this->select_chip = nand_select_chip;//���δ�ṩ�Զ���ĺ�����ʹ���Դ���Ĭ�ϵĺ���
				this->write_byte = busw ? nand_write_byte16 : nand_write_byte;
				this->write_byte = busw ? nand_write_byte16 : nand_write_byte;
				this->cmdfunc (mtd, NAND_CMD_READID, 0x00, -1);//�൱��ʹ��nand_command
					this->hwcontrol(mtd, NAND_CTL_SETCLE);  		 //ʹ��֮ǰ�Զ����s3c2440_nand_hwcontrol
					
				this->cmdfunc (mtd, NAND_CMD_READID, 0x00, -1);
				/ * Read manufacturer and device IDs * /
				nand_maf_id = this->read_byte(mtd);
				nand_dev_id = this->read_byte(mtd);
		*/
	env_relocate ();					//���������ĳ�ʼ��
		/*
			env_ptr = (env_t *)malloc (CFG_ENV_SIZE);  //Ϊ��������������ռ�
			DEBUGF ("%s[%d] malloced ENV at %p\n", __FUNCTION__,__LINE__,env_ptr);
			if (gd->env_valid == 0) {					//env_init�о�����env_valid��ֵ������flash�д��ڵĻ�������У�����
		#if defined(CONFIG_GTH)	|| defined(CFG_ENV_IS_NOWHERE)	/* Environment not changable * /
				puts ("Using default environment\n\n");
		#else
				//��ӡУ�������Ϣ
				puts ("*** Warning - bad CRC, using default environment\n\n");
				SHOW_BOOT_PROGRESS (-1);
		#endif
				//#define ENV_SIZE (CFG_ENV_SIZE - ENV_HEADER_SIZE)  ENV_SIZE��include/environment.h�ж���
				if (sizeof(default_environment) > ENV_SIZE)
				{
					puts ("*** Error - default environment is too large\n\n");
					return;
				}
		
				memset (env_ptr, 0, sizeof(env_t));//Ϊ���������洢������ռ�
				memcpy (env_ptr->data,
					default_environment,
					sizeof(default_environment));//������Ĭ�ϱ���������RAM�е�ָ������
		#ifdef CFG_REDUNDAND_ENVIRONMENT
				env_ptr->flags = 0xFF;
		#endif
				env_crc_update ();//����crc
				gd->env_valid = 1;
			}
			else {//����flash�д��ڵĻ�������У��ɹ�
				env_relocate_spec ();
				/ *�ú���ʵ���������ض�λ����,�ȴ�NAND flash�ж�ȡ��������,�����ȡ��* /
			}
			gd->env_addr = (ulong)&(env_ptr->data);//�������������׵�ַ(����crcͷ��)����ȫ�ֱ���gd->env_addr
		*/
	main_loop ();							//��������
	/////////////////////common/main.c:main_lopp()����/////////////////////////////////////////////////////
	if (!getenv("mtdparts"))				//����
  {
    run_command("mtdparts default", 0);
    /*
    ����mtdparts default����
    U_BOOT_CMD(
			mtdparts,	6,	0,	do_jffs2_mtdparts,
			"mtdparts- define flash/nand partitions\n",);
		����do_jffs2_mtdparts����
		do_jffs2_mtdparts()->
			setenv("mtdparts", (char *)mtdparts_default);//mtdparts_default = MTDPARTS_DEFAULT;
			Ȼ��������õ�mtdparts����������.
    */
  }
	s = getenv ("bootcmd");
	//nand read.jffs2 0x30007FC0 kernel; nand read.jffs2 32000000 device_tree; bootm 0x30007FC0 - 0x32000000
	//��nand flash������ں˶����ڴ����棬�����ں˵�ʱ��bootm 0x30007fc0-0x32000000
	//ִ��mtdparts defaultָ��
  U_BOOT_CMD(nand, 5, 1, do_nand,
    "nand    - NAND sub-system\n",
    "info                  - show available NAND devices\n"
    "nand device [dev]     - show or set current device\n"
    "nand read[.jffs2]     - addr off|partition size\n"
    "nand write[.jffs2]    - addr off|partiton size - read/write `size' bytes starting\n"
    "    at offset `off' to/from memory address `addr'\n");
  //����do_nand��������ʵ��nand read�����ں˺��豸����ȡ���ڴ���ָ����λ��
  //Ȼ����ȡbootm������ִ��ָ��
  U_BOOT_CMD(
	 	bootm,	CFG_MAXARGS,	1,	do_bootm,
	 	"bootm   - boot application image from memory\n",
	 	"[addr [arg ...]]\n    - boot application image stored in memory\n"
	 	"\tpassing arguments 'arg ...'; when booting a Linux kernel,\n"
	 	"\t'arg' can be the address of an initrd image\n"
	#ifdef CONFIG_OF_FLAT_TREE
		"\tWhen booting a Linux kernel which requires a flat device-tree\n"
		"\ta third argument is required which is the address of the of the\n"
		"\tdevice-tree blob. To boot that kernel without an initrd image,\n"
		"\tuse a '-' for the second argument. If you do not pass a third\n"
		"\ta bd_info struct will be passed instead\n"
	#endif
	);
	//����do_bootm��������ʵ�ֶ�ȡ�ں˲�����Ϣ
	//�ú����Ļ������þ��Ƿ�����kernel��ͷ����Ϣ�����Ա�����������kernelҪ����uboot��mkimage��image���ͷ����Ϣ��uimage����
	/*
		����ih_load;������kernel�Ļ�ַ�����Բ���ǰ���nand read 0x32000000 0x200000 0x300000;��0x32000000����������ַ��
	����ͨ���������ͷ����Ϣ����ȡ���յļ��ص�ַ���ڽ�code���Ƶ�ָ����ih_loadǰ�����жϸ�kernel�Ƿ���ѹ���ģ����û
	ѹ��ֱ��memmove ((void ) ntohl(hdr->ih_load), (uchar )data, len); ��ָ����ih_load���������ѹ���ľͽ�ѹ��ָ��
	ih_load����ͬʱ���ͷ����Ϣ��kernel����crcУ�� ����ih_hcrc ih_dcrc����У��ֵ��mkimage�������ɡ� 
		����do_bootm�����������ǽ�kernel������ѹ�������Ҫ�����Ƶ�ָ����ih_load������У�����ݵ������ԣ�
	������do_bootm_linux ����������
	*/
	do_bootm()
	{
		image_header_t *hdr = &header;
		
		s = getenv ("verify");
		verify = (s && (*s == 'n')) ? 0 : 1;
		addr = simple_strtoul(argv[1], NULL, 16);
		memmove (&header, (char *)addr, sizeof(image_header_t));
		data = (ulong)&header;
		len  = sizeof(image_header_t);

		checksum = ntohl(hdr->ih_hcrc);
		hdr->ih_hcrc = 0;
		if (crc32 (0, (uchar *)data, len) != checksum)
		{......................}
		
		/* for multi-file images we need the data part, too */
		print_image_hdr ((image_header_t *)addr);	
		data = addr + sizeof(image_header_t);
		len  = ntohl(hdr->ih_size);
		if (verify) {
			puts ("   Verifying Checksum ... ");
			if (crc32 (0, (uchar *)data, len) != ntohl(hdr->ih_dcrc)) {
				printf ("Bad Data CRC\n");
				SHOW_BOOT_PROGRESS (-3);
				return 1;
			}
			puts ("OK\n");
		}
		
		switch (hdr->ih_type) {
			case IH_TYPE_KERNEL:
				name = "Kernel Image";
				break;
			............
		}
		switch (hdr->ih_comp) {
			case IH_COMP_NONE:
				if(ntohl(hdr->ih_load) == data) {
					printf ("   XIP %s ... ", name);
				} else {
					memmove ((void *) ntohl(hdr->ih_load), (uchar *)data, len);
				}
				break;
			...............
		}
		switch (hdr->ih_type) {
			case IH_OS_LINUX:
	    	do_bootm_linux  (cmdtp, flag, argc, argv,
			     addr, len_ptr, verify);
	    break;
	    ................
		}
	}
/*----------------------------------------------------------------------------
��������lib_arm/armlinux.c:do_bootm_linux����
----------------------------------------------------------------------------*/	
//����do_bootm��������ʵ�������ںˡ��豸��
do_bootm_linux()
{
	void (*theKernel)(int zero, int arch, uint params);
	image_header_t *hdr = &header;
	bd_t *bd = gd->bd;
	char	*of_flat_tree = NULL;
	ulong	of_data = 0;

#ifdef CONFIG_CMDLINE_TAG
	char *commandline = getenv ("bootargs");
#endif

	theKernel = (void (*)(int, int, uint))ntohl(hdr->ih_ep);//ָ����ڵ�ַ

	/* s3c2440 for device tree, no initrd image used */
	if (argc == 4) {
		of_flat_tree = (char *) simple_strtoul(argv[3], NULL, 16);//��ȡ���ĸ�����Ҳ���豸����ַ

		if  (be32_to_cpu(*(ulong *)of_flat_tree) == OF_DT_HEADER) {
			printf ("\nStarting kernel with device tree at 0x%x...\n\n", of_flat_tree);

			cleanup_before_linux ();			
			theKernel (0, bd->bi_arch_number, of_flat_tree);		//�����ں�
					
		} else {
			printf("Bad magic of device tree at 0x%x!\n\n", of_flat_tree);
		}
		
	}
}
//��������ں˵��������ڶ�������һ����ʹ�õ��豸��֮���ò���������������Ϊ�豸����ַ





























