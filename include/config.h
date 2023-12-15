/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

/*
 * Maximum number of memory regions to accept from the multiboot information
 * structure. This is an arbitrary limit and exceeding it will throw an error.
 */
#define MAX_MEMORY_REGIONS      16

/*
 * Maximum number of I/O APICs to register. This is set to 1 to match what seL4
 * does. All additional I/O APICs will be ignored (with a warning).
 */
#define MAX_NUM_IOAPICS         1

/*
 * Maximum number of DMA Remapping Hardware units. This is an arbitrary limit
 * and exceeding it will throw an error.
 */
#define MAX_NUM_DRHUS           16

/*
 * Maximum number of Reserved memory Region Reporting structures. This is an
 * arbitrary limit and exceeding it will throw an error.
 */
#define MAX_NUM_RMRRS           16
