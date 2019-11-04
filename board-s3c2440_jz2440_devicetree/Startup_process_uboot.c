/*
先来看看配置过程：
执行make 100ask24x0_config   
*打开顶层Makefile查找目标和依赖
100ask24x0_config	:	unconfig
	@$(MKCONFIG) $(@:_config=) arm arm920t 100ask24x0 NULL s3c24x0

MKCONFIG	:= $(SRCTREE)/mkconfig
其实执行make 100ask24x0_config的配置命令就相当于执行了
mkconfig 100ask24x0 arm arm920t 100ask24x0 NULL s3c24x0	这条指令
*打开mkconfig文件来看看配置:	
echo "ARCH   = $2" >  config.mk
echo "CPU    = $3" >> config.mk
echo "BOARD  = $4" >> config.mk
[ "$5" ] && [ "$5" != "NULL" ] && echo "VENDOR = $5" >> config.mk
[ "$6" ] && [ "$6" != "NULL" ] && echo "SOC    = $6" >> config.mk	
也就是说ARCH="arm" CPU="arm920t" BOARD="100ask24x0" VENDOR=NULL SOC="s3c24x0" 
	
*顶层的Makefile里面还有其他语句	
# load other configuration
include $(TOPDIR)/config.mk	    这句包含了顶层的config.mk
*打开顶层的config.mk文件，有这么几句
ifdef	ARCH
#//也就是说包含了uboot/arm_config.mk
sinclude $(TOPDIR)/$(ARCH)_config.mk	# include architecture dependend rules
endif
ifdef	CPU
#//也就是说包含了uboot/cpu/arm920t.mk
sinclude $(TOPDIR)/cpu/$(CPU)/config.mk	# include  CPU	specific rules
endif
ifdef	SOC
#//也就是说包含了uboot/cpu/arm920t/s3c24x0/config.mk
sinclude $(TOPDIR)/cpu/$(CPU)/$(SOC)/config.mk	# include  SoC	specific rules
endif
ifdef	VENDOR   			之前定义为NULL，执行else
BOARDDIR = $(VENDOR)/$(BOARD)
else
BOARDDIR = $(BOARD)
endif
ifdef	BOARD
#//也就是说包含了uboot/board/100ask24x0/config.mk，打开该文件
#//有这么一句：TEXT_BASE = 0x33F80000   就是说链接地址为0x33F80000
sinclude $(TOPDIR)/board/$(BOARDDIR)/config.mk	# include board specific rules
endif	
再往下看有这么一句：
LDSCRIPT := $(TOPDIR)/board/$(BOARDDIR)/u-boot.lds	
我们可以得知链接脚本lds文件是uboot/board/100ask24x0/u-boot.lds	

查看链接脚本	
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
链接脚本中这些宏的定义在linkage.h中，看字面意思也明白，程序的入口是在_start.，后面是text段，data段等。
_start在cpu/arm920t/start.S中，一段一段的分析，如下：
*/ 








































