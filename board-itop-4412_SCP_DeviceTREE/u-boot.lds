OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")#;定义3种输出文件的格式。
若有命令行选项－EB（大端），则使用第二个输出格式，有命令行指定－EL（小端），则使用第三个格式。
否则使用默认的default输出格式。/*指定输出可执行文件是elf格式,32位ARM指令,小端格式*/
OUTPUT_ARCH(arm)#设置输出文件的体系架构。
ENTRY(_start)
SECTIONS
{
		/*
     * 指定可执行image文件的全局入口点，通常这个地址都放在ROM(flash)0x0位置。
     * 必须使编译器知道这个地址，通常都是修改此处来完成.
     */
 . = 0x00000000;
 . = ALIGN(4);#表示代码段从0x00000000开始加载，然后后面的.rodata,.data,.bss段都分别加在其后，并
 且后面的每个段的起始地址是按照4个字节对齐的。
 .text :
 {
 	//映像文件复制起始地址
  *(.__image_copy_start)//__image_copy_start 的值为当前地址，即 0X00000000 
  *(.vectors)
  arch/arm/cpu/armv7/start.o (.text*) #启动start文件
  *(.text*)
 }
 . = ALIGN(4);
 .rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }//指定只读数据段
 . = ALIGN(4);
 .data : {
  *(.data*)
 }
 . = ALIGN(4);
 . = .;
 . = ALIGN(4);
 .u_boot_list : {
  KEEP(*(SORT(.u_boot_list*)));/*.data段结束后，紧接着存放u-boot自有的一些function，
  例如u-boot command等*/
 }
 . = ALIGN(4);
 .image_copy_end :/*至此，u-boot需要自拷贝的内容结束，总结一下，包括代码段，数据段，以及u_boot_list*/
 {
  *(.__image_copy_end)
 }
 .rel_dyn_start :/*在老的uboot中，如果我们想要uboot启动后把自己拷贝到内存中的某个地方，
 只要把要拷贝的地址写给TEXT_BASE即可，然后boot启动后就会把自己拷贝到TEXT_BASE内的地址处
 运行，在拷贝之前的代码都是相对的，不能出现绝对的跳转，否则会跑飞。在新版的uboot里
 （2013.07），TEXT_BASE的含义改变了。它表示用户要把这段代码加载到哪里，通常是通过串口等工
 具。然后搬移的时候由uboot自己计算一个地址来进行搬移。新版的uboot采用了动态链接技术，
 在lds文件中有__rel_dyn_start和__rel_dyn_end，这两个符号之间的区域存放着动态链接符号，
 只要给这里面的符号加上一定的偏移，拷贝到内存中代码的后面相应的位置处，就可以在绝对跳转
 中找到正确的函数。*/
 {
  *(.__rel_dyn_start)
 }
 .rel.dyn : {
  *(.rel*)/*动态链接符存放在的段*/
 }
 .rel_dyn_end :
 {
  *(.__rel_dyn_end)/*动态链接符段结束*/
 }
 .end :
 {
  *(.__end)
 }
 _image_binary_end = .;/*bin文件结束*/
 /*
     * Deprecated: this MMU section is used by pxa at present but
     * should not be used by new boards/CPUs.
     */
 . = ALIGN(4096);
 .mmutable : {
  *(.mmutable)
 }
  /*
     * Compiler-generated __bss_start and __bss_end, see arch/arm/lib/bss.c
     * __bss_base and __bss_limit are for linker only (overlay ordering)
     */
 .bss_start __rel_dyn_start (OVERLAY) : {
  KEEP(*(.__bss_start));
  __bss_base = .;
 }
 .bss __bss_base (OVERLAY) : {
  *(.bss*)
   . = ALIGN(4);
   __bss_limit = .;
 }
 .bss_end __bss_limit (OVERLAY) : {
  KEEP(*(.__bss_end));
 }
 .dynsym _image_binary_end : { *(.dynsym) }
 .dynbss : { *(.dynbss) }
 .dynstr : { *(.dynstr*) }
 .dynamic : { *(.dynamic*) }
 .plt : { *(.plt*) }
 .interp : { *(.interp*) }
 .gnu.hash : { *(.gnu.hash) }
 .gnu : { *(.gnu*) }
 .ARM.exidx : { *(.ARM.exidx*) }
 .gnu.linkonce.armexidx : { *(.gnu.linkonce.armexidx.*) }
}
