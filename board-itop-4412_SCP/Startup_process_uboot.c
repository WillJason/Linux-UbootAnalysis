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





































































