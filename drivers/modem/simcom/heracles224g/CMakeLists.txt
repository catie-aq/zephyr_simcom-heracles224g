# Copyright (c) 2025 CATIE
# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_include_directories(${ZEPHYR_BASE}/subsys/net/ip)
zephyr_library_include_directories(${ZEPHYR_BASE}/subsys/net/lib/sockets)

zephyr_library_include_directories(${ZEPHYR_BASE}/drivers/modem/)

if (CONFIG_MODEM_HERACLES224G)
zephyr_library_sources(heracles224g.c)
endif()
