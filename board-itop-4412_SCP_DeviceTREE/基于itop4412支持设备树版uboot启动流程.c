/*
�����κγ�����ں�����������ʱ�����ģ�uboot������������ӽű������ġ�uboot��armv7���ӽű�Ĭ��Ŀ¼Ϊarch/arm/cpu/u-boot.lds����������������ļ�����CONFIG_SYS_LDSCRIPT��ָ����
��ڵ�ַҲ���������������ģ��������ļ��п�����CONFIG_SYS_TEXT_BASEָ����������ڱ���ʱ����ld��������ѡ��-Ttext��
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
 
 
 
 
 
 
 