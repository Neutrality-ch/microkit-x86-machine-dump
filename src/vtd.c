/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include "serial.h"
#include "sysinfo.h"
#include "vtd.h"

#define VTD_CAP_REG       0x08
#define VTD_SAGAW         8
#define VTD_SAGAW_2_LEVEL 0x01
#define VTD_SAGAW_3_LEVEL 0x02
#define VTD_SAGAW_4_LEVEL 0x04
#define VTD_SAGAW_5_LEVEL 0x08
#define VTD_SAGAW_6_LEVEL 0x10

/*
 * Read a 32-bit register from a specific IOMMU. Note while paging is enabled
 * we assume a 1:1 virtual mapping so we can use physical memory addresses.
 */
static inline uint32_t vtd_read32(uint32_t drhu_id, uint32_t offset)
{
    return *(volatile uint32_t *)(sysinfo.drhu.addr[drhu_id] + offset);
}

/*
 * Scan the Intel VT-d stuff.
 */
bool vtd_scan(void)
{
	/* Early exit if there's no IOMMU. */
	if (sysinfo.drhu.count == 0)
		return true;

	/* Code shamelessly taken from seL4 intel-vtd.c */
	uint32_t aw_bitmask = 0xffffffff;
	for (int i = 0; i < sysinfo.drhu.count; i++)
		aw_bitmask &= vtd_read32(i, VTD_CAP_REG) >> VTD_SAGAW;

	/* Populate the sysinfo structure. */
	if (aw_bitmask & VTD_SAGAW_3_LEVEL)
		sysinfo.vtd.num_iopt_levels = 3;
	else if (aw_bitmask & VTD_SAGAW_4_LEVEL)
		sysinfo.vtd.num_iopt_levels = 4;
	else if (aw_bitmask & VTD_SAGAW_5_LEVEL)
		sysinfo.vtd.num_iopt_levels = 5;
	else if (aw_bitmask & VTD_SAGAW_6_LEVEL)
		sysinfo.vtd.num_iopt_levels = 6;
	else if (aw_bitmask & VTD_SAGAW_2_LEVEL)
		sysinfo.vtd.num_iopt_levels = 2;
	else {
		serial_puts("[X] Error: IOMMU: mismatch of supported number of PT levels between IOMMUs\n");
		return false;
	}

	return true;
}
