/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
 * ACPI table header, common to all ACPI tables.
 */
struct acpi_header {
	char     signature[4];
	uint32_t length;
	uint8_t  revision;
	uint8_t  checksum;
	char     oem_id[6];
	char     oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed));

/*
 * ACPI Root System Description Table. This table has a variable size with at
 * least one entry.
 */
struct acpi_rsdt {
	struct acpi_header header;
	uint32_t entry[1];
} __attribute__((packed));

/*
 * Multiple APIC Description Table (MADT).
 */

enum acpi_madt_type {
	ACPI_MADT_TYPE_IOAPIC = 1,
};

struct acpi_madt {
	struct acpi_header header;
	uint32_t apic_base;
	uint32_t flags;
} __attribute__((packed));

struct acpi_madt_header {
	uint8_t type;
	uint8_t length;
} __attribute__((packed));

struct acpi_madt_ioapic {
	struct acpi_madt_header header;
	uint8_t apic_id;
	uint8_t reserved;
	uint32_t address;
	uint32_t gsib;
} __attribute__((packed));

/*
 * DMA Remapping table (DMAR).
 */

enum acpi_dmar_type {
	ACPI_DMAR_TYPE_DRHD = 0,
	ACPI_DMAR_TYPE_RMRR = 1,
};

struct acpi_dmar {
	struct acpi_header header;
	uint8_t host_addr_width;
	uint8_t flags;
	uint8_t reserved[10];
} __attribute__((packed));

struct acpi_dmar_header {
	uint16_t type;
	uint16_t length;
} __attribute__((packed));

struct acpi_dmar_drhd {
	struct acpi_dmar_header header;
	uint8_t flags;
	uint8_t reserved;
	uint16_t segment;
	uint32_t reg_base[2];
} __attribute__((packed));

enum acpi_dmar_rmrr_devscope_type {
	ACPI_DMAR_SCOPE_TYPE_NOT_USED  = 0,
	ACPI_DMAR_SCOPE_TYPE_ENDPOINT  = 1,
	ACPI_DMAR_SCOPE_TYPE_BRIDGE    = 2,
	ACPI_DMAR_SCOPE_TYPE_IOAPIC    = 3,
	ACPI_DMAR_SCOPE_TYPE_HPET      = 4,
	ACPI_DMAR_SCOPE_TYPE_NAMESPACE = 5,
	ACPI_DMAR_SCOPE_TYPE_RESERVED  = 6,
};

struct acpi_dmar_rmrr_devscope {
	uint8_t  type;
	uint8_t  length;
	uint16_t reserved;
	uint8_t  enum_id;
	uint8_t  start_bus;
	struct {
		uint8_t dev;
		uint8_t fun;
	} path[1];
} __attribute__((packed));

struct acpi_dmar_rmrr {
	struct acpi_dmar_header header;
	uint16_t reserved;
	uint16_t segment;
	uint32_t reg_base[2];
	uint32_t reg_limit[2];
	struct acpi_dmar_rmrr_devscope devscope[1];
} __attribute__((packed));

extern bool acpi_parse_tables(void);
