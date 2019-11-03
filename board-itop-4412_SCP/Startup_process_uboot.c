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





































































