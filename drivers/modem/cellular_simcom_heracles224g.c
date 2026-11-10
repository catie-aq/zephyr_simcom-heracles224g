/*
 * Copyright (c) 2026 CATIE
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/modem/modem_cellular.h>

#define DT_DRV_COMPAT simcom_heracles224g

MODEM_CELLULAR_COMMON_CHAT_MATCHES();

MODEM_CHAT_MATCH_DEFINE(powerdown_match, "NORMAL POWER DOWN", "", NULL);

MODEM_CHAT_MATCHES_DEFINE(simcom_heracles224g_unsol, MODEM_CELLULAR_COMMON_UNSOL_MATCHES,
			  MODEM_CHAT_MATCH("RDY", "", modem_cellular_chat_on_modem_ready));

#define HERACLES224G_CNMP "AT+CNMP=" STRINGIFY(CONFIG_MODEM_SIMCOM_HERACLES224G_CNMP_VALUE)
#define HERACLES224G_CMNB "AT+CMNB=" STRINGIFY(CONFIG_MODEM_SIMCOM_HERACLES224G_CMNB_VALUE)

#define HERACLES224G_CNMP_CMD                                                                      \
	IF_ENABLED(CONFIG_MODEM_SIMCOM_HERACLES224G_SET_NETWORK_MODE,                              \
		   (MODEM_CHAT_SCRIPT_CMD_RESP(HERACLES224G_CNMP, ok_match), ))

#define HERACLES224G_CMNB_CMD                                                                      \
	IF_ENABLED(CONFIG_MODEM_SIMCOM_HERACLES224G_SET_LTE_TECH,                                  \
		   (MODEM_CHAT_SCRIPT_CMD_RESP(HERACLES224G_CMNB, ok_match), ))

/* clang-format off */
MODEM_CHAT_SCRIPT_CMDS_DEFINE(
	simcom_heracles224g_init_chat_script_cmds,
	MODEM_CHAT_SCRIPT_CMD_RESP_NONE("AT", 100), MODEM_CHAT_SCRIPT_CMD_RESP_NONE("AT", 100),
	MODEM_CHAT_SCRIPT_CMD_RESP_NONE("AT", 100), MODEM_CHAT_SCRIPT_CMD_RESP_NONE("AT", 100),
	MODEM_CHAT_SCRIPT_CMD_RESP("ATE0", ok_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("AT+CFUN=4", ok_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("AT+CMEE=1", ok_match),
	HERACLES224G_CNMP_CMD
	HERACLES224G_CMNB_CMD
	MODEM_CHAT_SCRIPT_CMD_RESP("AT+CREG=1", ok_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("AT+CGREG=1", ok_match),
	MODEM_CHAT_SCRIPT_CMD_RESP_MULT("AT+CEREG=1", allow_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("AT+CREG?", ok_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("AT+CGREG?", ok_match),
	MODEM_CHAT_SCRIPT_CMD_RESP_MULT("AT+CEREG?", allow_match),
	MODEM_CHAT_SCRIPT_CMD_RESP_MULT("AT+CGMI", cgmi_match),
	MODEM_CHAT_SCRIPT_CMD_RESP_MULT("AT+CGMR", cgmr_match),
	MODEM_CHAT_SCRIPT_CMD_RESP_MULT("AT+CGSN", imei_match),
	MODEM_CHAT_SCRIPT_CMD_RESP_MULT("AT+CGMM", cgmm_match),
	MODEM_CHAT_SCRIPT_CMD_RESP_MULT("AT+CIMI", cimi_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("AT+CMUX=0,0,5," STRINGIFY(CONFIG_MODEM_CMUX_MTU), ok_match));
/* clang-format on */

MODEM_CHAT_SCRIPT_DEFINE(simcom_heracles224g_init_chat_script,
			 simcom_heracles224g_init_chat_script_cmds, abort_matches,
			 modem_cellular_chat_callback_handler, 10);

MODEM_CHAT_SCRIPT_CMDS_DEFINE(simcom_heracles224g_dial_chat_script_cmds,
			      MODEM_CHAT_SCRIPT_CMD_RESP_MULT(
				      "AT+CGACT=0," STRINGIFY(CONFIG_MODEM_CELLULAR_PDP_CONTEXT_ID),
							      allow_match),
				      MODEM_CHAT_SCRIPT_CMD_RESP("AT+CFUN=1", ok_match),
				      MODEM_CHAT_SCRIPT_CMD_RESP_NONE("ATD*99***" STRINGIFY(CONFIG_MODEM_CELLULAR_PDP_CONTEXT_ID) "#", 0));

MODEM_CHAT_SCRIPT_DEFINE(simcom_heracles224g_dial_chat_script,
			 simcom_heracles224g_dial_chat_script_cmds, dial_abort_matches,
			 modem_cellular_chat_callback_handler, 10);

MODEM_CHAT_SCRIPT_CMDS_DEFINE(simcom_heracles224g_periodic_chat_script_cmds,
			      MODEM_CHAT_SCRIPT_CMD_RESP("AT+CREG?", ok_match),
			      MODEM_CHAT_SCRIPT_CMD_RESP("AT+CGREG?", ok_match),
			      MODEM_CHAT_SCRIPT_CMD_RESP_MULT("AT+CEREG?", allow_match));

MODEM_CHAT_SCRIPT_DEFINE(simcom_heracles224g_periodic_chat_script,
			 simcom_heracles224g_periodic_chat_script_cmds, abort_matches,
			 modem_cellular_chat_callback_handler, 4);

MODEM_CHAT_SCRIPT_CMDS_DEFINE(simcom_heracles224g_shutdown_chat_script_cmds,
			      MODEM_CHAT_SCRIPT_CMD_RESP("AT+CPOWD=1", powerdown_match));

MODEM_CHAT_SCRIPT_DEFINE(simcom_heracles224g_shutdown_chat_script,
			 simcom_heracles224g_shutdown_chat_script_cmds, abort_matches,
			 modem_cellular_chat_callback_handler, 5);

static const struct modem_cellular_vendor_config simcom_heracles224g_vendor = {
	/* clang-format off */
	.scripts = {
		.init = &simcom_heracles224g_init_chat_script,
		.dial = &simcom_heracles224g_dial_chat_script,
		.periodic = &simcom_heracles224g_periodic_chat_script,
		.shutdown = &simcom_heracles224g_shutdown_chat_script,
	},
	.unsol_matches = {
		.matches = simcom_heracles224g_unsol,
		.size = ARRAY_SIZE(simcom_heracles224g_unsol),
	},
	/* clang-format on */
	.chat_delimiter = "\r",
	.chat_filter = "\n",
	.power_pulse_duration_ms = 1600,
	.reset_pulse_duration_ms = 100,
	.startup_time_ms = 10000,
	.shutdown_time_ms = 5000,
};

#define MODEM_CELLULAR_DEVICE_SIMCOM_HERACLES224G(inst)                                            \
	MODEM_DT_INST_PPP_DEFINE(inst, MODEM_CELLULAR_INST_NAME(ppp, inst), NULL, 1500, 64);       \
                                                                                                   \
	static struct modem_cellular_data MODEM_CELLULAR_INST_NAME(data, inst);                    \
                                                                                                   \
	MODEM_CELLULAR_DEFINE_AND_INIT_USER_PIPES(inst, (user_pipe_0, 3))                          \
                                                                                                   \
	MODEM_CELLULAR_DEFINE_INSTANCE(inst, &simcom_heracles224g_vendor, NULL)

DT_INST_FOREACH_STATUS_OKAY(MODEM_CELLULAR_DEVICE_SIMCOM_HERACLES224G)
