/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include "acpi.h"
#include "serial.h"
#include "sysinfo.h"

/*
 * Parse MADT I/O APIC entries.
 */
static bool parse_madt_ioapic(struct acpi_madt_ioapic *ioapic)
{
	/* Populate the sysinfo structure. */
	if (sysinfo.ioapic.count == MAX_NUM_IOAPICS) {
		serial_puts("[!] Warning: too many IOAPICs, ignoring.\n");
		return true;
	}
	sysinfo.ioapic.addr[sysinfo.ioapic.count++] = ioapic->address;

	serial_puts("[*] I/O APIC #");
	serial_put_uint64(sysinfo.ioapic.count);
	serial_puts(" found\n");

	return true;
}

/*
 * Parse the Multiple APIC Description Table (MADT).
 */
static bool parse_madt(struct acpi_madt *madt)
{
	serial_puts("[*] ACPI MADT table found\n");

	/* Save the base address of the APIC. */
	sysinfo.apic.addr = madt->apic_base;

	/* Walk the list of entries. */
	struct acpi_madt_header *header = (void *) (madt + 1);
	while ((char *) header < (char *) madt + madt->header.length) {

		/* Catch I/O APICs. */
		if (header->type == ACPI_MADT_TYPE_IOAPIC &&
		    !parse_madt_ioapic((struct acpi_madt_ioapic *) header))
			return false;

		/* Jump to the next entry. */
		header = (void *) ((char *) header + header->length);
	}

	return true;
}

/*
 * Parse DMAR DRHD entries.
 */
static bool parse_dmar_drhd(struct acpi_dmar_drhd *drhd)
{
	/* Populate the sysinfo structure. */
	if (sysinfo.drhu.count == MAX_NUM_DRHUS) {
		serial_puts("[X] Error: too many DRHUs, please raise the limit\n");
		return false;
	}
	sysinfo.drhu.addr[sysinfo.drhu.count++] = drhd->reg_base[0];

	serial_puts("[*] DRHU #");
	serial_put_uint64(sysinfo.drhu.count);
	serial_puts(" found\n");

	return true;
}

/*
 * Parse DMAR RMRR entries.
 */
static bool parse_dmar_rmrr(struct acpi_dmar_rmrr *rmrr)
{
	serial_puts("[*] ACPI: DMAR: RMRR table found\n");

	/* Validate the entry. */
	if (rmrr->reg_base[1] != 0 || rmrr->reg_limit[1] != 0) {
		serial_puts("[!] Warning: ACPI: RMRR device above 4GiB, disabling IOMMU support\n");
		sysinfo.drhu.count = 0;
		sysinfo.rmrr.count = 0;
		return true;
	}

	/* Walk the list of device scopes. */
	int devmax = (rmrr->header.length - sizeof (*rmrr)) / sizeof (rmrr->devscope[0]);
	for (int i = 0; i <= devmax; i++) {
		struct acpi_dmar_rmrr_devscope *devscope = &rmrr->devscope[i];

		/* Check the device scope type. */
		if (devscope->type != ACPI_DMAR_SCOPE_TYPE_ENDPOINT) {
			serial_puts("[!] Warning: ACPI: RMRR device scope: type not supported, disabling IOMMU support\n");
			sysinfo.drhu.count = 0;
			sysinfo.rmrr.count = 0;
			return true;
		}

		/* Check the device scope size. */
		if (devscope->length != sizeof (*devscope)) {
			serial_puts("[!] Warning: ACPI: RMRR device scope: bridges not supported, disabling IOMMU support\n");
			sysinfo.drhu.count = 0;
			sysinfo.rmrr.count = 0;
			return true;
		}

		/* Populate the sysinfo structure. */
		if (sysinfo.rmrr.count == MAX_NUM_RMRRS) {
			serial_puts("[X] Error: too many RMRRs, please raise the limit\n");
			return false;
		}
		sysinfo.rmrr.list[sysinfo.rmrr.count].addr  = rmrr->reg_base[0];
		sysinfo.rmrr.list[sysinfo.rmrr.count].limit = rmrr->reg_limit[0];
		sysinfo.rmrr.list[sysinfo.rmrr.count].devid = 0 \
			| (devscope->start_bus   << 8)          \
			| (devscope->path[0].dev << 3)          \
			| (devscope->path[0].fun << 0);
		sysinfo.rmrr.count++;
	}

	return true;
}

/*
 * Parse the Intel DMA Remapping table (DMAR).
 */
static bool parse_dmar(struct acpi_dmar *dmar)
{
	serial_puts("[*] ACPI DMAR table found\n");

	/* Walk the list of entries. */
	struct acpi_dmar_header *header = (void *) (dmar + 1);
	while ((char *) header < (char *) dmar + dmar->header.length) {

		/* Catch DHRDs. */
		if (header->type == ACPI_DMAR_TYPE_DRHD &&
		    !parse_dmar_drhd((struct acpi_dmar_drhd *) header))
			return false;

		/* Catch RMRRs. */
		if (header->type == ACPI_DMAR_TYPE_RMRR &&
		    !parse_dmar_rmrr((struct acpi_dmar_rmrr *) header))
			return false;

		/* Jump to the next entry. */
		header = (void *) ((char *) header + header->length);
	}
	return true;
}

/*
 * Parse the ACPI tables.
 */
bool acpi_parse_tables(void)
{
	struct acpi_rsdt *rsdt = (void *) sysinfo.rsdt.addr;
	bool madt_found = false;
	bool dmar_found = false;

	/* Walk the list of ACPI system description tables. */
	int nentries = (rsdt->header.length - sizeof (rsdt->header)) / 4;
	for (int i = 0; i < nentries; i++) {
		struct acpi_header *header = (void *) rsdt->entry[i];

		/* Parse the MADT table. */
		if (!memcmp(header->signature, "APIC", 4)) {
			if (parse_madt((struct acpi_madt *) header))
				madt_found = true;
			continue;
		}

		/* Parse the DMAR table. */
		if (!memcmp(header->signature, "DMAR", 4)) {
			if (parse_dmar((struct acpi_dmar *) header))
				dmar_found = true;
			continue;
		}
	}

	/* Make sure that the tables were found. */
	if (!madt_found) {
		serial_puts("[X] Error: ACPI MADT table not found!\n");
		return false;
	}
	if (!dmar_found) {
		serial_puts("[X] Error: ACPI DMAR table not found!\n");
		return false;
	}
	return true;
}
