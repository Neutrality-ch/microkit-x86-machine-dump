/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <stdint.h>
#include "config.h"

/**
 * System information structure.
 */
struct sysinfo {

	/* Memory regions. */
	struct {
		uint32_t count;
		struct {
			uint64_t addr;
			uint64_t size;
		} list[MAX_MEMORY_REGIONS];
	} memory;

	/* ACPI Root System Description Table. */
	struct {
		uint32_t addr;
	} rsdt;

	/* APIC. */
	struct {
		uint64_t addr;
	} apic;

	/* I/O APICs. */
	struct {
		uint32_t count;
		uint32_t addr[MAX_NUM_IOAPICS];
	} ioapic;

	/* DMA Remapping Hardware Units. */
	struct {
		uint32_t count;
		uint32_t addr[MAX_NUM_DRHUS];
	} drhu;

	/* Reserved memory Region Reporting structures. */
	struct {
		uint32_t count;
		struct {
			uint32_t addr;
			uint32_t limit;
			uint32_t devid;
		} list[MAX_NUM_RMRRS];
	} rmrr;

	/* VT-d. */
	struct {
		uint32_t num_iopt_levels;
	} vtd;
};

/* Global system information data. */
extern struct sysinfo sysinfo;
