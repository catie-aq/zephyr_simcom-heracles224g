# SIMCom Heracles 224G Cellular Modem Driver

Zephyr OS driver for the [SIMCom Heracles 224G](https://www.simcom.com/) GSM / LTE Cat-M1 / NB-IoT
modem.

The driver is a vendor extension of the Zephyr generic cellular modem framework
(`CONFIG_MODEM_CELLULAR`): the modem is configured with AT commands, the UART is multiplexed with
CMUX (`AT+CMUX`) and the data link uses PPP (`ATD*99***<cid>#`). The modem is therefore exposed as a
native Zephyr network interface (sockets, DNS, TLS, MQTT, ... from the Zephyr network stack).

## Devicetree

```dts
&uart0 {
	status = "okay";
	current-speed = <115200>;

	modem: modem {
		compatible = "simcom,heracles224g";
		status = "okay";
		mdm-power-gpios = <&gpioa 0 GPIO_ACTIVE_HIGH>;  /* drives PWRKEY low when active */
		mdm-status-gpios = <&gpioa 1 GPIO_ACTIVE_HIGH>; /* optional, STATUS pin */
	};
};
```

## Configuration

```
CONFIG_NETWORKING=y
CONFIG_NET_L2_PPP=y
CONFIG_MODEM=y
CONFIG_MODEM_CELLULAR=y
CONFIG_MODEM_CELLULAR_APN="your.apn"
```

The driver is enabled automatically (`CONFIG_MODEM_SIMCOM_HERACLES224G`) when a
`simcom,heracles224g` node is enabled in the devicetree.

### Network mode

The radio access technology can be selected at modem initialization (both values are saved by
the modem in non-volatile memory). By default the modem settings are left unchanged.

| Kconfig choice | AT command |
|---|---|
| `CONFIG_MODEM_SIMCOM_HERACLES224G_NETWORK_MODE_AUTO` | `AT+CNMP=2` |
| `CONFIG_MODEM_SIMCOM_HERACLES224G_NETWORK_MODE_GSM` | `AT+CNMP=13` |
| `CONFIG_MODEM_SIMCOM_HERACLES224G_NETWORK_MODE_LTE` | `AT+CNMP=38` |
| `CONFIG_MODEM_SIMCOM_HERACLES224G_NETWORK_MODE_GSM_LTE` | `AT+CNMP=51` |
| `CONFIG_MODEM_SIMCOM_HERACLES224G_LTE_TECH_CATM` | `AT+CMNB=1` |
| `CONFIG_MODEM_SIMCOM_HERACLES224G_LTE_TECH_NBIOT` | `AT+CMNB=2` |
| `CONFIG_MODEM_SIMCOM_HERACLES224G_LTE_TECH_CATM_NBIOT` | `AT+CMNB=3` |

Example, LTE Cat-M1 / NB-IoT only:

```
CONFIG_MODEM_SIMCOM_HERACLES224G_NETWORK_MODE_LTE=y
CONFIG_MODEM_SIMCOM_HERACLES224G_LTE_TECH_CATM_NBIOT=y
```

## Sample

See [samples](samples): it powers the modem, waits for the PPP link, resolves a host name and
exchanges data with a TCP echo server.

The upstream Zephyr `samples/net/cellular_modem` sample can also be used with the
Zest_Radio_Heracles224G shield: see [samples/zephyr_cellular_modem](samples/zephyr_cellular_modem).
