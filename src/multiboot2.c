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
#include "utils.h"

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
 * Find and return a multiboot2 info tag.
 */
struct multiboot2_tag_header *multiboot2_find_tag(uint32_t info_addr, uint32_t type)
{
	struct multiboot2_info_header *info_header = (void *) info_addr;
	uint32_t tag_addr    = info_addr + sizeof (*info_header);
	uint32_t tag_endaddr = info_addr + info_header->total_size;

	/* Walk the list of tags. */
	while (tag_addr < tag_endaddr) {
		struct multiboot2_tag_header *tag_header = (void *) tag_addr;

		/* Return the tag if found. */
		if (tag_header->type == type)
			return tag_header;

		/* Break on the end tag. */
		if (tag_header->type == MULTIBOOT2_INFO_TAG_END)
			break;

		/* Continue to the next tag. */
		tag_addr = roundup64(tag_addr + tag_header->size);
	}

	/* Not found. */
	return NULL;
}

/*
 * Split a string into space-separated tokens and return the first token. The
 * input string is modified in place to insert string terminating 0 bytes, and
 * the string pointer is moved to the end of the token on success to be called
 * iteratively.
 */
static char *get_token(char **string)
{
	/* Strip any leading space. */
	char *token = *string;
	while (*token == ' ')
		token++;

	/* Jump past the token and zero-terminate it. */
	char *end = token;
	while (*end) {
		if (*end == ' ') {
			*end++ = 0;
			break;
		}
		end++;
	}

	/* Return NULL when there aren't any more tokens. */
	if (end == token)
		return NULL;

	/* Otherwise return the token and bump the string pointer. */
	*string = end;
	return token;
}

/*
 * Split a string formatted as "key=value" into its key and value components.
 * The value component is optional and NULL is returned if not found. The
 * string is modified in place to insert string terminating 0 bytes.
 */
static void get_token_key_value(char *string, char **key, char **value)
{
	*key = string;
	while (*string) {
		if (*string == '=') {
			*string = 0;
			*value = ++string;
			return;
		}
		string++;
	}
	*value = NULL;
	return;
}

/*
 * Parse the multiboot2 command line tag.
 */
static bool parse_command_line(struct multiboot2_tag_header *tag_header)
{
	char *cmdline = (char *) (tag_header + 1);
	unsigned size = tag_header->size - sizeof (*tag_header);

	/* It's ok to be left blank. */
	if (!size || *cmdline == '\0')
		return true;

	/* Force 0-termination. */
	cmdline[size - 1] = '\0';
	serial_printf("[*] Multiboot2 command line: %s\n", cmdline);

	/* Parse all tokens. */
	char *token;
	while ((token = get_token(&cmdline))) {
		char *key, *value;
		get_token_key_value(token, &key, &value);

		/* Option: serial port. */
		if (!strcmp(key, "serial")) {
			uint64_t val = 0;

			if (value && strtou64(value, &val) && val < 0x3ff) {
				serial_printf("[*] Switching to serial port %X\n", val);
				options.serial_port = (uint16_t) val;
				serial_init();
				continue;
			} else {
				serial_printf("[X] Cannot parse serial port option\n");
				return false;
			}
		}

		/* Option: on_exit. */
		if (!strcmp(key, "on_exit")) {
			if (value) {
				if (!strcmp(value, "hang")) {
					options.on_exit = OPTION_ON_EXIT_HANG;
					continue;
				}
				if (!strcmp(value, "reboot")) {
					options.on_exit = OPTION_ON_EXIT_REBOOT;
					continue;
				}
				if (!strcmp(value, "shutdown")) {
					options.on_exit = OPTION_ON_EXIT_SHUTDOWN;
					continue;
				}
			}
			serial_printf("[X] Cannot parse on_exit option\n");
			return false;
		}

		/* Unknown option. */
		serial_printf("[X] Invalid command line option: %s\n", key);
		return false;
	}

	return true;
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
	struct multiboot2_tag_header *tag;

	/* Find the command line first. */
	if ((tag = multiboot2_find_tag(info_addr, MULTIBOOT2_INFO_TAG_COMMAND_LINE))) {
		if (!parse_command_line(tag))
			return false;
	}

	/* Find and parse the memory map tag. */
	if ((tag = multiboot2_find_tag(info_addr, MULTIBOOT2_INFO_TAG_MEMORY_MAP))) {
		if (!parse_mmap_tag(tag))
			return false;
	} else {
		serial_puts("[X] Error: multiboot2 memory map tag missing!\n");
		return false;
	}

	/* Find and parse an ACPI RSDP tags. */
	if ((tag = multiboot2_find_tag(info_addr, MULTIBOOT2_INFO_TAG_ACPI_NEW_RSDP))) {
		if (!parse_rsdp2_tag(tag))
			return false;
	} else if ((tag = multiboot2_find_tag(info_addr, MULTIBOOT2_INFO_TAG_ACPI_OLD_RSDP))) {
		if (!parse_rsdp1_tag(tag))
			return false;
	} else {
		serial_puts("[X] Error: multiboot ACPI tag missing!\n");
		return false;
	}

	return true;
}
