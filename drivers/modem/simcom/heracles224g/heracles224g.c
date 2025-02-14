/*
 * Copyright (c) 2025, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT simcom_heracles224g

#include <zephyr/kernel.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>

#include "heracles224g.h"

LOG_MODULE_REGISTER(HERACLES224G, CONFIG_MODEM_LOG_LEVEL);

struct heracles224g_config {
};

struct heracles224g_data {
};

static int heracles224g_attr_set(const struct device *dev, enum modem_channel chan,
				 enum modem_attribute attr, const struct modem_value *val)
{
	return 0;
}

static int heracles224g_sample_fetch(const struct device *dev, enum modem_channel chan)
{
	struct heracles224g_data *data = dev->data;
	const struct heracles224g_config *config = dev->config;

	return 0;
}

static int heracles224g_channel_get(const struct device *dev, enum modem_channel chan,
				    struct modem_value *val)
{
	struct heracles224g_data *data = dev->data;

	// TODO: Update val with the modem value
	val->val1 = 0;
	val->val2 = 0;

	return 0;
}

static int heracles224g_init(const struct device *dev)
{
	const struct heracles224g_config *config = dev->config;
	struct heracles224g_data *data = dev->data;

	return 0;
}

static const struct modem_driver_api heracles224g_driver_api = {
	.attr_set = heracles224g_attr_set,
	.sample_fetch = heracles224g_sample_fetch,
	.channel_get = heracles224g_channel_get,
};

#define HERACLES224G_INIT(n)                                                                       \
	static struct heracles224g_config heracles224g_config_##n = {};                            \
	static struct heracles224g_data heracles224g_data_##n;                                     \
	DEVICE_DT_INST_DEFINE(n, heracles224g_init, NULL, &heracles224g_data_##n,                  \
			      &heracles224g_config_##n, POST_KERNEL, CONFIG_MODEM_INIT_PRIORITY,   \
			      &heracles224g_driver_api);

DT_INST_FOREACH_STATUS_OKAY(HERACLES224G_INIT)
