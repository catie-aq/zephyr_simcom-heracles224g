/*
 * Copyright (C) 2021 metraTec GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SIMCOM_HERACLES224G_H
#define SIMCOM_HERACLES224G_H

#include <zephyr/kernel.h>
#include <ctype.h>
#include <inttypes.h>
#include <errno.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/init.h>
#include <string.h>

#include <zephyr/net/net_if.h>
#include <zephyr/net/net_offload.h>
#include <zephyr/net/socket_offload.h>

#include "modem_context.h"
#include "modem_cmd_handler.h"
#include "modem_iface_uart.h"
#include "modem_socket.h"

#define MDM_UART_NODE DT_INST_BUS(0)
#define MDM_UART_DEV DEVICE_DT_GET(MDM_UART_NODE)
#define MDM_MAX_DATA_LENGTH 1024
#define MDM_RECV_BUF_SIZE 1024
#define MDM_MAX_SOCKETS 5
#define MDM_BASE_SOCKET_NUM 0
#define MDM_RECV_MAX_BUF 30
#define BUF_ALLOC_TIMEOUT K_SECONDS(1)
#define MDM_CMD_TIMEOUT K_SECONDS(10)
#define MDM_REGISTRATION_TIMEOUT K_SECONDS(180)
#define MDM_CONNECT_TIMEOUT K_SECONDS(90)
#define MDM_PDP_TIMEOUT K_SECONDS(120)
#define MDM_DNS_TIMEOUT K_SECONDS(210)
#define MDM_BRING_UP_WIRELESS_CONNECTION K_SECONDS(60)
#define MDM_WAIT_FOR_RSSI_DELAY K_SECONDS(2)
#define MDM_WAIT_FOR_RSSI_COUNT 30
#define MDM_MAX_AUTOBAUD 5
#define MDM_MAX_CEREG_WAITS 40
#define MDM_MAX_CGATT_WAITS 120
#define MDM_BOOT_TRIES 4
#define MDM_GNSS_PARSER_MAX_LEN 128
#define MDM_APN CONFIG_MODEM_SIMCOM_HERACLES224G_APN
#define MDM_LTE_BANDS CONFIG_MODEM_SIMCOM_HERACLES224G_LTE_BANDS
#define RSSI_TIMEOUT_SECS 30
#define MDM_BASIC_CMD_TIMEOUT K_SECONDS(2)
#define MDM_ACTIVATE_PDP_CONTEXT_TIMEOUT K_SECONDS(5)

/*
 * Default length of modem data.
 */
#define MDM_MANUFACTURER_LENGTH 12
#define MDM_MODEL_LENGTH 16
#define MDM_REVISION_LENGTH 64
#define MDM_IMEI_LENGTH 16
#define MDM_IMSI_LENGTH 16
#define MDM_ICCID_LENGTH 32

enum heracles224g_state {
	HERACLES224G_STATE_INIT = 0,
	HERACLES224G_STATE_NETWORKING,
	HERACLES224G_STATE_GNSS,
	HERACLES224G_STATE_OFF,
};

/* Possible states of the ftp connection. */
enum heracles224g_ftp_connection_state {
	/* Not connected yet. */
	HERACLES224G_FTP_CONNECTION_STATE_INITIAL = 0,
	/* Connected and still data available. */
	HERACLES224G_FTP_CONNECTION_STATE_CONNECTED,
	/* All data transferred. */
	HERACLES224G_FTP_CONNECTION_STATE_FINISHED,
	/* Something went wrong. */
	HERACLES224G_FTP_CONNECTION_STATE_ERROR,
};

/*
 * Driver data.
 */
struct heracles224g_data {
	/*
	 * Network interface of the sim module.
	 */
	struct net_if *netif;
	uint8_t mac_addr[6];
	/*
	 * Uart interface of the modem.
	 */
	struct modem_iface_uart_data iface_data;
	uint8_t iface_rb_buf[MDM_MAX_DATA_LENGTH];
	/*
	 * Modem command handler.
	 */
	struct modem_cmd_handler_data cmd_handler_data;
	uint8_t cmd_match_buf[MDM_RECV_BUF_SIZE + 1];
	/*
	 * Modem socket data.
	 */
	struct modem_socket_config socket_config;
	struct modem_socket sockets[MDM_MAX_SOCKETS];
	/*
	 * Current state of the modem.
	 */
	enum heracles224g_state state;
	/*
	 * RSSI work
	 */
	struct k_work_delayable rssi_query_work;
	/*
	 * Information over the modem.
	 */
	char mdm_manufacturer[MDM_MANUFACTURER_LENGTH];
	char mdm_model[MDM_MODEL_LENGTH];
	char mdm_revision[MDM_REVISION_LENGTH];
	char mdm_imei[MDM_IMEI_LENGTH];
#if defined(CONFIG_MODEM_SIM_NUMBERS)
	char mdm_imsi[MDM_IMSI_LENGTH];
	char mdm_iccid[MDM_ICCID_LENGTH];
#endif /* #if defined(CONFIG_MODEM_SIM_NUMBERS) */
	int mdm_rssi;
	/*
	 * Current operating socket and statistics.
	 */
	int current_sock_fd;
	int current_sock_written;
	/*
	 * Network registration of the modem.
	 */
	uint8_t mdm_registration;
	/*
	 * Whether gprs is attached or detached.
	 */
	uint8_t mdm_cgatt;
	/*
	 * local ip address.
	 */
	char mdm_ip_addr[16];
	/*
	 * If the sim card is ready or not.
	 */
	bool cpin_ready;
	/*
	 * Flag if the PDP context is active.
	 */
	bool pdp_active;
	/*
	 * Flag if the PDP context is configured.
	 */
	bool pdp_configured;
	/* SMS buffer structure provided by read. */
	struct heracles224g_sms_buffer *sms_buffer;
	/* Position in the sms buffer. */
	uint8_t sms_buffer_pos;
	/* Ftp related variables. */
	struct {
		/* User buffer for ftp data. */
		char *read_buffer;
		/* Length of the read buffer/number of bytes read. */
		size_t nread;
		/* State of the ftp connection. */
		enum heracles224g_ftp_connection_state state;
	} ftp;
	/*
	 * Semaphore(s).
	 */
	struct k_sem sem_response;
	struct k_sem sem_tx_ready;
	struct k_sem sem_dns;
	struct k_sem sem_ftp;
};

/*
 * Socket read callback data.
 */
struct socket_read_data {
	char *recv_buf;
	size_t recv_buf_len;
	struct sockaddr *recv_addr;
	uint16_t recv_read_len;
};




// ------------------------------------------------------



#define HERACLES224G_GNSS_DATA_UTC_LEN 20
#define HERACLES224G_SMS_MAX_LEN 160

struct heracles224g_gnss_data {
	/**
	 * Whether gnss is powered or not.
	 */
	bool run_status;
	/**
	 * Whether fix is acquired or not.
	 */
	bool fix_status;
	/**
	 * UTC in format yyyyMMddhhmmss.sss
	 */
	char utc[HERACLES224G_GNSS_DATA_UTC_LEN];
	/**
	 * Latitude in 10^-7 degree.
	 */
	int32_t lat;
	/**
	 * Longitude in 10^-7 degree.
	 */
	int32_t lon;
	/**
	 * Altitude in mm.
	 */
	int32_t alt;
	/**
	 * Horizontal dilution of precision in 10^-2.
	 */
	uint16_t hdop;
	/**
	 * Course over ground un 10^-2 degree.
	 */
	uint16_t cog;
	/**
	 * Speed in 10^-1 km/h.
	 */
	uint16_t kmh;
};

/**
 * Possible sms states in memory.
 */
enum heracles224g_sms_stat {
	HERACLES224G_SMS_STAT_REC_UNREAD = 0,
	HERACLES224G_SMS_STAT_REC_READ,
	HERACLES224G_SMS_STAT_STO_UNSENT,
	HERACLES224G_SMS_STAT_STO_SENT,
	HERACLES224G_SMS_STAT_ALL,
};

/**
 * Possible ftp return codes.
 */
enum heracles224g_ftp_rc {
	/* Operation finished correctly. */
	HERACLES224G_FTP_RC_OK = 0,
	/* Session finished. */
	HERACLES224G_FTP_RC_FINISHED,
	/* An error occurred. */
	HERACLES224G_FTP_RC_ERROR,
};

/**
 * Buffer structure for sms.
 */
struct heracles224g_sms {
	/* First octet of the sms. */
	uint8_t first_octet;
	/* Message protocol identifier. */
	uint8_t tp_pid;
	/* Status of the sms in memory. */
	enum heracles224g_sms_stat stat;
	/* Index of the sms in memory. */
	uint16_t index;
	/* Time the sms was received. */
	struct {
		uint8_t year;
		uint8_t month;
		uint8_t day;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
		uint8_t timezone;
	} time;
	/* Buffered sms. */
	char data[HERACLES224G_SMS_MAX_LEN + 1];
	/* Length of the sms in buffer. */
	uint8_t data_len;
};

/**
 * Buffer structure for sms reads.
 */
struct heracles224g_sms_buffer {
	/* sms structures to read to. */
	struct heracles224g_sms *sms;
	/* Number of sms structures. */
	uint8_t nsms;
};

/**
 * @brief Power on the Sim7080.
 *
 * @return 0 on success. Otherwise -1 is returned.
 */
int mdm_heracles224g_power_on(void);

/**
 * @brief Power off the Sim7080.
 *
 * @return 0 on success. Otherwise -1 is returned.
 */
int mdm_heracles224g_power_off(void);

/**
 * @brief Starts the modem in network operation mode.
 *
 * @return 0 on success. Otherwise <0 is returned.
 */
int mdm_heracles224g_start_network(void);

/**
 * @brief Starts the modem in gnss operation mode.
 *
 * @return 0 on success. Otherwise <0 is returned.
 */
int mdm_heracles224g_start_gnss(void);

/**
 * @brief Query gnss position form the modem.
 *
 * @return 0 on success. If no fix is acquired yet -EAGAIN is returned.
 *         Otherwise <0 is returned.
 */
int mdm_heracles224g_query_gnss(struct heracles224g_gnss_data *data);

/**
 * Get the heracles224g manufacturer.
 */
const char *mdm_heracles224g_get_manufacturer(void);

/**
 * Get the heracles224g model information.
 */
const char *mdm_heracles224g_get_model(void);

/**
 * Get the heracles224g revision.
 */
const char *mdm_heracles224g_get_revision(void);

/**
 * Get the heracles224g imei number.
 */
const char *mdm_heracles224g_get_imei(void);

/**
 * Read sms from sim module.
 *
 * @param buffer Buffer structure for sms.
 * @return Number of sms read on success. Otherwise -1 is returned.
 *
 * @note The buffer structure needs to be initialized to
 * the size of the sms buffer. When this function finishes
 * successful, nsms will be set to the number of sms read.
 * If the whole structure is filled a subsequent read may
 * be needed.
 */
int mdm_heracles224g_read_sms(struct heracles224g_sms_buffer *buffer);

/**
 * Delete a sms at a given index.
 *
 * @param index The index of the sms in memory.
 * @return 0 on success. Otherwise -1 is returned.
 */
int mdm_heracles224g_delete_sms(uint16_t index);

/**
 * Start a ftp get session.
 *
 * @param server The ftp servers address.
 * @param user User name for the ftp server.
 * @param passwd Password for the ftp user.
 * @param file File to be downloaded.
 * @param path Path to the file on the server.
 * @return 0 if the session was started. Otherwise -1 is returned.
 */
int mdm_heracles224g_ftp_get_start(const char *server, const char *user, const char *passwd,
				  const char *file, const char *path);

/**
 * Read data from a ftp get session.
 *
 * @param dst The destination buffer.
 * @param size Initialize to the size of dst. Gets set to the number
 *             of bytes actually read.
 * @return According heracles224g_ftp_rc.
 */
int mdm_heracles224g_ftp_get_read(char *dst, size_t *size);

#endif /* SIMCOM_HERACLES224G_H */
