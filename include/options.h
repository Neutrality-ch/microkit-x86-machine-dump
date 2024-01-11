/*
 * Copyright 2024, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <stdint.h>

/*
 * Program options.
 */
struct options {

	/* Serial port to use. */
	uint16_t serial_port;

	/* What to do on exit. */
	enum {
		OPTION_ON_EXIT_HANG     = 0,
		OPTION_ON_EXIT_REBOOT   = 1,
		OPTION_ON_EXIT_SHUTDOWN = 2,
	} on_exit;
};

/* Global program option data. */
extern struct options options;
