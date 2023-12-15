/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

/*
 * Magic value to place in the multiboot2 header to identify as a multiboot2
 * compliant image.
 */
#define MULTIBOOT2_HEADER_MAGIC         0xE85250D6

/*
 * Magic value to expect on boot to identify the bootloader as multiboot2
 * compliant.
 */
#define MULTIBOOT2_BOOT_MAGIC           0x36d76289

/* Further definitions only apply the C code. */
#ifndef __ASM__

#include <stdbool.h>
#include <stdint.h>

/*
 * Boot info tags.
 */
enum multiboot2_info_tag {
	MULTIBOOT2_INFO_TAG_END                 = 0,
	MULTIBOOT2_INFO_TAG_COMMAND_LINE        = 1,
	MULTIBOOT2_INFO_TAG_BOOTLOADER_NAME     = 2,
	MULTIBOOT2_INFO_TAG_MODULE              = 3,
	MULTIBOOT2_INFO_TAG_MEMORY              = 4,
	MULTIBOOT2_INFO_TAG_BIOS_BOOT_DEVICE    = 5,
	MULTIBOOT2_INFO_TAG_MEMORY_MAP          = 6,
	MULTIBOOT2_INFO_TAG_VBE_INFO            = 7,
	MULTIBOOT2_INFO_TAG_FRAMEBUFFER_INFO    = 8,
	MULTIBOOT2_INFO_TAG_ELF_SYMBOLS         = 9,
	MULTIBOOT2_INFO_TAG_APM_TABLE           = 10,
	MULTIBOOT2_INFO_TAG_EFI32_SYSTEM_TABLE  = 11,
	MULTIBOOT2_INFO_TAG_EFI64_SYSTEM_TABLE  = 12,
	MULTIBOOT2_INFO_TAG_SMBIOS_TABLES       = 13,
	MULTIBOOT2_INFO_TAG_ACPI_OLD_RSDP       = 14,
	MULTIBOOT2_INFO_TAG_ACPI_NEW_RSDP       = 15,
	MULTIBOOT2_INFO_TAG_NETWORK_INFO        = 16,
	MULTIBOOT2_INFO_TAG_EFI_MEMORY_MAP      = 17,
	MULTIBOOT2_INFO_TAG_EFI_BOOTSVC_RUNNING = 18,
	MULTIBOOT2_INFO_TAG_EFI32_IMAGE_HANDLE  = 19,
	MULTIBOOT2_INFO_TAG_EFI64_IMAGE_HANDLE  = 20,
	MULTIBOOT2_INFO_TAG_LOAD_BASE_PADDR     = 21,
};

/*
 * Allowed types for all memory regions.
 */
enum multiboot2_mmap_type {
	MULTIBOOT2_MMAP_TYPE_USEABLE  = 1,
	MULTIBOOT2_MMAP_TYPE_RESERVED = 2,
	MULTIBOOT2_MMAP_TYPE_ACPI     = 3,
	MULTIBOOT2_MMAP_TYPE_ACPI_NVS = 4,
	MULTIBOOT2_MMAP_TYPE_BAD      = 5,
};

/*
 * Multiboot2 boot information structure header. This header is followed by an
 * array of multiboot2 tags.
 */
struct multiboot2_info_header {
	uint32_t total_size;
	uint32_t reserved;
} __attribute__((packed));

/*
 * Multiboot2 boot information tag header. This header is followed by tag
 * specific data.
 */
struct multiboot2_tag_header {
	uint32_t type;
	uint32_t size;
} __attribute__((packed));

/*
 * Multiboot2 memory map boot tag. This structure is followed by an array of
 * entries.
 */
struct multiboot2_tag_mmap {
	uint32_t entry_size;
	uint32_t entry_version;
} __attribute__((packed));

/*
 * Multiboot2 memory map entry structure. Each entry describes a memory region.
 */
struct multiboot2_tag_mmap_entry {
	uint64_t addr;
	uint64_t size;
	uint32_t type;
	uint32_t reserved;
} __attribute__((packed));

/*
 * Multiboot2 ACPI v1 RSDP boot tag. This structure is the entry point of the
 * ACPI v1 system description.
 */
struct multiboot2_tag_rsdp1 {
	char     signature[8];
	uint8_t  checksum;
	char     oemid[6];
	uint8_t  revision;
	uint32_t rsdt_address;
} __attribute__((packed));

/*
 * Multiboot2 ACPI v2 RSDP boot tag. This structure is the entry point of the
 * ACPI v2 system description.
 */
struct multiboot2_tag_rsdp2 {
	char     signature[8];
	uint8_t  checksum;
	char     oemid[6];
	uint8_t  revision;
	uint32_t rsdt_address;
	uint32_t length;
	uint64_t xsdt_address;
	uint8_t  extended_checksum;
	uint8_t  reserved[3];
} __attribute__((packed));

extern bool multiboot2_parse_info(uint32_t info_addr);

#endif
