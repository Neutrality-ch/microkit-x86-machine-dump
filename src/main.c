/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include "acpi.h"
#include "multiboot2.h"
#include "serial.h"
#include "sysinfo.h"
#include "utils.h"
#include "vtd.h"

/* Global system information structure. */
struct sysinfo sysinfo;

/*
 * Output the machine file in json format over the serial line.
 */
static void dump_machine_file(void)
{
	/* Opening tag. */
	serial_puts("\n"
	            "-----BEGIN MACHINE FILE BLOCK-----\n"
	            "{\n");

	/* Memory regions. */
	serial_puts("    \"memory\": [\n");
	for (uint32_t i = 0; i < sysinfo.memory.count; i++) {
		serial_printf("        {\n"
		              "            \"base\": %D,\n"
		              "            \"size\": %D\n"
	                      "        }%s\n",
		              sysinfo.memory.list[i].addr,
		              sysinfo.memory.list[i].size,
		              i + 1 == sysinfo.memory.count ? "" : ",");
	}
	serial_puts("    ],\n");

	/* Kernel devices. */
	serial_printf("    \"kdevs\": [\n"
	              "        {\n"
	              "            \"name\": \"apic\",\n"
	              "            \"base\": %D,\n"
	              "            \"size\": 4096\n"
	              "        }",
	              sysinfo.apic.addr);
	for (uint32_t i = 0; i < sysinfo.ioapic.count; i++) {
		serial_printf(",\n"
		              "        {\n"
		              "            \"name\": \"ioapic.%d\",\n"
		              "            \"base\": %d,\n"
		              "            \"size\": 4096\n"
		              "        }",
		              i, sysinfo.ioapic.addr[i]);
	}

	for (uint32_t i = 0; i < sysinfo.drhu.count; i++) {
		serial_printf(",\n"
		              "        {\n"
		              "            \"name\": \"drhu.%d\",\n"
		              "            \"base\": %d,\n"
		              "            \"size\": 4096\n"
		              "        }",
		              i, sysinfo.drhu.addr[i]);
	}
	serial_puts("\n    ],\n");

	/* RMRRs. */
	serial_puts("    \"rmrrs\": [\n");
	if (sysinfo.rmrr.count > 0) {
		for (int i = 0; i < sysinfo.rmrr.count; i++) {
			serial_printf("        {\n"
			              "            \"devid\": %d,\n"
			              "            \"base\":  %d,\n"
			              "            \"limit\": %d\n"
			              "        }%s\n",
			              sysinfo.rmrr.list[i].devid,
			              sysinfo.rmrr.list[i].addr,
			              sysinfo.rmrr.list[i].limit,
			              i + 1 == sysinfo.rmrr.count ? "" : ",");
		}
	}
	serial_puts("    ],\n");

	/* Bootinfo. */
	serial_printf("    \"bootinfo\": {\n"
	              "        \"numIOPTLevels\": %d\n"
	              "    }\n",
	              sysinfo.vtd.num_iopt_levels);

	/* Closing tag. */
	serial_puts("}\n"
	            "-----END MACHINE FILE BLOCK-----\n");
}

/*
 * Discover the hardware and print the machine file.
 */
void main(uint32_t multiboot_magic, uint32_t multiboot_info)
{
	memset((char *) &sysinfo, 0, sizeof (sysinfo));
	serial_init();

	/* Parse the multiboot info structure. */
	if (multiboot_magic == MULTIBOOT2_BOOT_MAGIC) {
		serial_puts("[*] Multiboot2 boot loader detected\n");
		if (multiboot2_parse_info(multiboot_info) == false)
			return;
	} else {
		serial_puts("[X] Error: unsupported boot loader (not multiboot2 compliant)!\n");
		return;
	}

	/* Parse the ACPI tables. */
	if (acpi_parse_tables() == false)
		return;

	/* Look up the number of VT-D IOPT levels. */
	if (vtd_scan() == false)
		return;

	/* Output the json file. */
	dump_machine_file();
}
