# Copyright (c) 2025 CATIE
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

if(NOT DEFINED SHIELD)
  set(SHIELD zest_radio_heracles224g)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(simcom_heracles224g)

target_sources(app PRIVATE src/main.c)
