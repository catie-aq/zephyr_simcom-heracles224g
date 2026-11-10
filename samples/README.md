# SIMCom Heracles 224G sample

This sample:

1. powers the modem (PWRKEY pulse) and brings up the PPP network interface;
2. waits for the network registration, the PPP link (L4 connected) and the DNS server;
3. prints the modem information (manufacturer, model, IMEI, IMSI, firmware);
4. resolves `CONFIG_SAMPLE_ECHO_HOSTNAME` and exchanges a message with a TCP echo server
   (`tcpbin.com:4242` by default).

Set your operator APN in `prj.conf` (`CONFIG_MODEM_CELLULAR_APN`).

The sample uses the Zest_Radio_Heracles224G shield (set in `CMakeLists.txt`), connected to the
first Zest_Core port (`ZEST_RADIO_HERACLES224G(1)` in `app.overlay`).

```shell
west build -p -b zest_core_stm32l4a6rg samples
west flash
```

If the driver and the shield are not yet declared in the west manifest, also add:
`-- -DZEPHYR_EXTRA_MODULES="<path>/simcom_heracles224g;<path>/zest_radio_heracles224g"`.

For a hardware-modified shield, use `--shield zest_radio_heracles224g_alt` and replace the macro by
`ZEST_RADIO_HERACLES224G_ALT(port, pwrkey, status, dtr)` in `app.overlay`.

The shell provides `modem` (cellular info) and `at` commands (raw AT commands on the CMUX user
pipe, DLCI 3).
