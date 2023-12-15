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
	serial_puts("\n-----BEGIN MACHINE FILE BLOCK-----\n");
	serial_puts("{\n");

	/* Memory regions. */
	serial_puts("    \"memory\": [\n");
	for (int i = 0; i < sysinfo.memory.count; i++) {
		serial_puts("        {\n");
		serial_puts("            \"base\": ");
		serial_put_uint64(sysinfo.memory.list[i].addr);
		serial_puts(",\n");
		serial_puts("            \"size\": ");
		serial_put_uint64(sysinfo.memory.list[i].size);
		serial_puts("\n");
		if (i + 1 == sysinfo.memory.count)
			serial_puts("        }\n");
		else
			serial_puts("        },\n");
	}
	serial_puts("    ],\n");

	/* Kernel devices. */
	serial_puts("    \"kdevs\": [\n");
	serial_puts("        {\n");
	serial_puts("            \"name\": \"apic\",\n");
	serial_puts("            \"base\": ");
	serial_put_uint64(sysinfo.apic.addr);
	serial_puts(",\n");
	serial_puts("            \"size\": 4096\n");
	serial_puts("        }");
	for (int i = 0; i < sysinfo.ioapic.count; i++) {
		serial_puts(",\n        {\n");
		serial_puts("            \"name\": \"ioapic.");
		serial_put_uint64(i);
		serial_puts("\",\n");
		serial_puts("            \"base\": ");
		serial_put_uint64(sysinfo.ioapic.addr[i]);
		serial_puts(",\n            \"size\": 4096\n");
		serial_puts("        }");
	}
	for (int i = 0; i < sysinfo.drhu.count; i++) {
		serial_puts(",\n        {\n");
		serial_puts("            \"name\": \"drhu.");
		serial_put_uint64(i);
		serial_puts("\",\n");
		serial_puts("            \"base\": ");
		serial_put_uint64(sysinfo.drhu.addr[i]);
		serial_puts(",\n            \"size\": 4096\n");
		serial_puts("        }");
	}
	serial_puts("\n    ],\n");

	/* RMRRs. */
	serial_puts("    \"rmrrs\": [\n");
	if (sysinfo.rmrr.count > 0) {
		for (int i = 0; i < sysinfo.rmrr.count; i++) {
			serial_puts("        {\n");
			serial_puts("            \"devid\": ");
			serial_put_uint64(sysinfo.rmrr.list[i].devid);
			serial_puts(",\n");
			serial_puts("            \"base\":  ");
			serial_put_uint64(sysinfo.rmrr.list[i].addr);
			serial_puts(",\n");
			serial_puts("            \"limit\": ");
			serial_put_uint64(sysinfo.rmrr.list[i].limit);
			serial_puts("\n");
			if (i + 1 == sysinfo.rmrr.count)
				serial_puts("        }\n");
			else
				serial_puts("        },\n");
		}
	}
	serial_puts("    ],\n");

	/* Bootinfo. */
	serial_puts("    \"bootinfo\": {\n");
	serial_puts("        \"numIOPTLevels\": ");
	serial_put_uint64(sysinfo.vtd.num_iopt_levels);
	serial_puts("\n    }\n");

	serial_puts("}\n");
	serial_puts("-----END MACHINE FILE BLOCK-----\n");
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
