/*
 *  Copyright (C) 2009-2010, Lars-Peter Clausen <lars@metafoo.de>
 *  Copyright (C) 2011, Maarten ter Huurne <maarten@treewalker.org>
 *  JZ4740 setup code
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General	 Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/clk-provider.h>
#include <linux/clocksource.h>




#include <linux/init.h>
#include <linux/io.h>
#include <linux/irqchip.h>
#include <linux/kernel.h>
#include <linux/libfdt.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>

#include <asm/bootinfo.h>
#include <asm/prom.h>

#include <asm/mach-jz4740/base.h>

//#include "reset.h"


#define JZ4740_EMC_SDRAM_CTRL 0x80


static void __init jz4740_detect_mem(void)
{
	void __iomem *jz_emc_base;
	u32 ctrl, bus, bank, rows, cols;
	phys_addr_t size;

	jz_emc_base = ioremap(JZ4740_EMC_BASE_ADDR, 0x100);
	ctrl = readl(jz_emc_base + JZ4740_EMC_SDRAM_CTRL);
	bus = 2 - ((ctrl >> 31) & 1);
	bank = 1 + ((ctrl >> 19) & 1);
	cols = 8 + ((ctrl >> 26) & 7);
	rows = 11 + ((ctrl >> 20) & 3);
	printk(KERN_DEBUG
		"SDRAM preconfigured: bus:%u bank:%u rows:%u cols:%u\n",
		bus, bank, rows, cols);
	iounmap(jz_emc_base);

	size = 1 << (bus + bank + cols + rows);
	add_memory_region(0, size, BOOT_MEM_RAM);
}

void __init plat_mem_setup(void)
{
	int offset;

	void __iomem *cpm_iobase = (void __iomem *)CKSEG1ADDR(0x10000000);

	/* ingenic mips cpu special */
	__asm__ (
		"li    $2, 0xa9000000 \n\t"
		"mtc0  $2, $5, 4      \n\t"
		"nop                  \n\t"
		::"r"(2));

	set_io_port_base(IO_BASE);
	/*Not have ioport*/
	ioport_resource.start	= 0x00000000;
	ioport_resource.end	= 0x00000000;
	iomem_resource.start	= 0x10000000;
	iomem_resource.end	= 0x1fffffff;

	/*x1000 cpu special*/
	writel( 0, cpm_iobase + 0x90);
	writel(16, cpm_iobase + 0x94);
	writel(24, cpm_iobase + 0x98);
	writel( 8, cpm_iobase + 0x9c);

//	jz4740_reset_init();
	__dt_setup_arch(__dtb_start);

	offset = fdt_path_offset(__dtb_start, "/memory");
	if (offset < 0)
		jz4740_detect_mem();
}

void __init device_tree_init(void)
{
	if (!initial_boot_params)
		return;

	unflatten_and_copy_device_tree();
}

static int __init populate_machine(void)
{
	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
	return 0;
}
arch_initcall(populate_machine);

void __init plat_time_init(void)
{
	of_clk_init(NULL);

	clocksource_probe();
}

const char *get_system_type(void)
{
	return "XBurst-Based";
}

void __init arch_init_irq(void)
{
	irqchip_init();
}
