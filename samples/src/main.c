/*
 * Copyright (c) 2026 CATIE
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/cellular.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/socket.h>

LOG_MODULE_REGISTER(heracles224g_sample, LOG_LEVEL_INF);

#define L4_EVENT_MASK                                                                              \
	(NET_EVENT_L4_CONNECTED | NET_EVENT_L4_DISCONNECTED | NET_EVENT_DNS_SERVER_ADD)
#define L4_CONNECTED BIT(0)
#define L4_DNS_ADDED BIT(1)

#define ECHO_MESSAGE "Hello from Zest_Radio_Heracles224G!"

static const struct device *modem = DEVICE_DT_GET(DT_ALIAS(modem));
static struct net_if *ppp_iface;

K_EVENT_DEFINE(l4_event);

static void l4_event_handler(uint64_t event, struct net_if *iface, void *info, size_t info_length,
			     void *user_data)
{
	ARG_UNUSED(info);
	ARG_UNUSED(info_length);
	ARG_UNUSED(user_data);

	if (iface != ppp_iface) {
		return;
	}

	switch (event) {
	case NET_EVENT_L4_CONNECTED:
		k_event_post(&l4_event, L4_CONNECTED);
		break;
	case NET_EVENT_DNS_SERVER_ADD:
		k_event_post(&l4_event, L4_DNS_ADDED);
		break;
	case NET_EVENT_L4_DISCONNECTED:
		k_event_set(&l4_event, 0);
		break;
	default:
		break;
	}
}

NET_MGMT_REGISTER_EVENT_HANDLER(l4_events, L4_EVENT_MASK, l4_event_handler, NULL);

static void print_modem_info(void)
{
	static const struct {
		enum cellular_modem_info_type type;
		const char *name;
	} infos[] = {
		{CELLULAR_MODEM_INFO_MANUFACTURER, "Manufacturer"},
		{CELLULAR_MODEM_INFO_MODEL_ID, "Model"},
		{CELLULAR_MODEM_INFO_FW_VERSION, "Firmware"},
		{CELLULAR_MODEM_INFO_IMEI, "IMEI"},
		{CELLULAR_MODEM_INFO_SIM_IMSI, "IMSI"},
	};
	char buffer[64];
	int16_t rssi;

	for (size_t i = 0; i < ARRAY_SIZE(infos); i++) {
		if (cellular_get_modem_info(modem, infos[i].type, buffer, sizeof(buffer)) == 0) {
			LOG_INF("%s: %s", infos[i].name, buffer);
		}
	}

	if (cellular_get_signal(modem, CELLULAR_SIGNAL_RSSI, &rssi) == 0) {
		LOG_INF("RSSI: %d dBm", rssi);
	}
}

static int tcp_echo(void)
{
	struct zsock_addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};
	struct zsock_addrinfo *res;
	char port[8];
	char rx[sizeof(ECHO_MESSAGE)];
	size_t received = 0;
	int sock;
	int ret;

	snprintk(port, sizeof(port), "%d", CONFIG_SAMPLE_ECHO_PORT);

	LOG_INF("Resolving %s", CONFIG_SAMPLE_ECHO_HOSTNAME);
	ret = zsock_getaddrinfo(CONFIG_SAMPLE_ECHO_HOSTNAME, port, &hints, &res);
	if (ret != 0) {
		LOG_ERR("DNS resolution failed (%d)", ret);
		return -EHOSTUNREACH;
	}

	sock = zsock_socket(res->ai_family, res->ai_socktype, IPPROTO_TCP);
	if (sock < 0) {
		LOG_ERR("Failed to create socket (%d)", errno);
		zsock_freeaddrinfo(res);
		return -errno;
	}

	LOG_INF("Connecting to %s:%d", CONFIG_SAMPLE_ECHO_HOSTNAME, CONFIG_SAMPLE_ECHO_PORT);
	ret = zsock_connect(sock, res->ai_addr, res->ai_addrlen);
	zsock_freeaddrinfo(res);
	if (ret < 0) {
		LOG_ERR("Failed to connect (%d)", errno);
		goto out;
	}

	ret = zsock_send(sock, ECHO_MESSAGE, strlen(ECHO_MESSAGE), 0);
	if (ret < 0) {
		LOG_ERR("Failed to send (%d)", errno);
		goto out;
	}

	while (received < strlen(ECHO_MESSAGE)) {
		ret = zsock_recv(sock, &rx[received], strlen(ECHO_MESSAGE) - received, 0);
		if (ret <= 0) {
			LOG_ERR("Failed to receive (%d)", ret < 0 ? errno : 0);
			ret = -EIO;
			goto out;
		}
		received += ret;
	}
	rx[received] = '\0';

	LOG_INF("Echo received: \"%s\"", rx);
	ret = strcmp(rx, ECHO_MESSAGE) == 0 ? 0 : -EBADMSG;

out:
	zsock_close(sock);
	return ret;
}

int main(void)
{
	uint32_t events;
	int ret;

	if (!device_is_ready(modem)) {
		LOG_ERR("Modem device not ready");
		return -ENODEV;
	}

	ppp_iface = net_if_get_first_by_type(&NET_L2_GET_NAME(PPP));
	if (ppp_iface == NULL) {
		LOG_ERR("No PPP interface");
		return -ENODEV;
	}

	LOG_INF("Bring up network interface (powering modem)");
	ret = net_if_up(ppp_iface);
	if (ret < 0) {
		LOG_ERR("Failed to bring up network interface (%d)", ret);
		return ret;
	}

	LOG_INF("Waiting for network connection");
	events = k_event_wait(&l4_event, L4_CONNECTED, false, K_SECONDS(180));
	if (!(events & L4_CONNECTED)) {
		LOG_ERR("Network not connected in time");
		return -ETIMEDOUT;
	}

	events = k_event_wait(&l4_event, L4_DNS_ADDED, false, K_SECONDS(10));
	if (!(events & L4_DNS_ADDED)) {
		LOG_WRN("No DNS server received from the network");
	}

	print_modem_info();

	ret = tcp_echo();
	LOG_INF("TCP echo test %s", ret == 0 ? "succeeded" : "failed");

	return 0;
}
