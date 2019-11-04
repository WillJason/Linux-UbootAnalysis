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








































