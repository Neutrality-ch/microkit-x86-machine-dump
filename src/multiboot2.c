/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include "multiboot2.h"
#include "serial.h"
#include "sysinfo.h"

/* x86 high memory begins at 1MB. */
#define HIGHMEM_BASE_ADDR       0x100000

/*
 * Round a number up to the next 64-bit boundary.
 */
static uint32_t roundup64(uint32_t n)
{
	if (n & 7)
		n = (n & ~7) + 8;
	return n;
}

/*
 * Parse the multiboot2 memory map tag.
 */
static bool parse_mmap_tag(struct multiboot2_tag_header *tag_header)
{
	struct multiboot2_tag_mmap *tag = (void *) (tag_header + 1);
	uint32_t saddr = (uint32_t) (tag + 1);
	uint32_t eaddr = (uint32_t) tag + tag_header->size;

	serial_puts("[*] Multiboot2 memory map tag found\n");

	/* Walk the list of memory map entries. */
	for (uint32_t addr = saddr; addr < eaddr; addr += tag->entry_size) {
		struct multiboot2_tag_mmap_entry *m = (struct multiboot2_tag_mmap_entry *) addr;

		/* Ignore unusable or low memory (below 1MB). */
		if (m->type != MULTIBOOT2_MMAP_TYPE_USEABLE || m->addr < HIGHMEM_BASE_ADDR)
			continue;

		serial_printf("[*] Found a usable memory area at %X size %X\n", m->addr, m->size);

		/* Populate the sysinfo structure. */
		if (sysinfo.memory.count == MAX_MEMORY_REGIONS) {
			serial_puts("[X] Error: too many memory regions provided by boot loader!\n");
			return false;
		}
		sysinfo.memory.list[sysinfo.memory.count].addr = m->addr;
		sysinfo.memory.list[sysinfo.memory.count].size = m->size;
		sysinfo.memory.count++;
	}

	return true;
}

/*
 * Parse the multiboot2 ACPI 1.0 RSDP tag.
 */
static bool parse_rsdp1_tag(struct multiboot2_tag_header *tag_header)
{
	struct multiboot2_tag_rsdp1 *tag = (void *) (tag_header + 1);

	serial_puts("[*] Multiboot2 ACPI 1.0 RSDP tag found\n");
	sysinfo.rsdt.addr = tag->rsdt_address;
	return true;
}

/*
 * Parse the multiboot2 ACPI 2.0 RSDP tag.
 */
static bool parse_rsdp2_tag(struct multiboot2_tag_header *tag_header)
{
	struct multiboot2_tag_rsdp2 *tag = (struct multiboot2_tag_rsdp2 *) (tag_header + 1);

	serial_puts("[*] Multiboot2 ACPI 2.0 RSDP tag found\n");
	sysinfo.rsdt.addr = tag->rsdt_address;
	return true;
}

/*
 * Parse the multiboot2 info structure.
 */
bool multiboot2_parse_info(uint32_t info_addr)
{
	struct multiboot2_info_header *info_header = (void *) info_addr;
	uint32_t tag_addr    = info_addr + sizeof (*info_header);
	uint32_t tag_endaddr = info_addr + info_header->total_size;
	bool mmap_found = false;
	bool acpi_found = false;

	/* Walk the list of multiboot info tags. */
	while (tag_addr < tag_endaddr) {
		struct multiboot2_tag_header *tag_header = (void *) tag_addr;

		/* Break on the end tag. */
		if (tag_header->type == MULTIBOOT2_INFO_TAG_END)
			break;

		/* Parse the memory map tag. */
		if (tag_header->type == MULTIBOOT2_INFO_TAG_MEMORY_MAP) {
			if (parse_mmap_tag(tag_header) == false)
				return false;
			mmap_found = true;
		}

		/* Parse the ACPI 1.0 RSDP tags. */
		if (tag_header->type == MULTIBOOT2_INFO_TAG_ACPI_OLD_RSDP) {
			if (parse_rsdp1_tag(tag_header) == false)
				return false;
			acpi_found = true;
		}

		/* Parse the ACPI 2.0 RSDP tags. */
		if (tag_header->type == MULTIBOOT2_INFO_TAG_ACPI_NEW_RSDP) {
			if (parse_rsdp2_tag(tag_header) == false)
				return false;
			acpi_found = true;
		}

		/* Continue to the next tag. */
		tag_addr = roundup64(tag_addr + tag_header->size);
	}

	/* Make sure that we found what we were looking for. */
	if (!mmap_found) {
		serial_puts("[X] Error: multiboot2 memory map tag missing!\n");
		return false;
	}
	if (!acpi_found) {
		serial_puts("[X] Error: multiboot ACPI tag missing!\n");
		return false;
	}

	return true;
}
