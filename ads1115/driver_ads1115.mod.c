#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x92997ed8, "_printk" },
	{ 0x41453899, "_dev_err" },
	{ 0x6776710, "__register_chrdev" },
	{ 0x61907396, "__class_create" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xd43e80f0, "device_create" },
	{ 0x3a8831fe, "class_destroy" },
	{ 0x36948ecd, "i2c_register_driver" },
	{ 0xec85562e, "i2c_del_driver" },
	{ 0x8c7fb26f, "device_destroy" },
	{ 0x25303d98, "class_unregister" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0x4a23c383, "i2c_smbus_write_word_data" },
	{ 0x5f754e5a, "memset" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xb10bd345, "i2c_transfer_buffer_flags" },
	{ 0x2547772f, "i2c_smbus_read_i2c_block_data" },
	{ 0x97255bdf, "strlen" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0xc84d16dc, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("i2c:ads1115");

MODULE_INFO(srcversion, "A34243D06CB7D290C5C966F");
