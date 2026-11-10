# Zephyr `cellular_modem` sample with the Zest_Radio_Heracles224G

Configuration files to run the upstream Zephyr
[`samples/net/cellular_modem`](https://docs.zephyrproject.org/latest/samples/net/cellular_modem/README.html)
sample with the Zest_Radio_Heracles224G shield connected to the first Zest_Core port.

1. Set your operator APN (`CONFIG_MODEM_CELLULAR_APN`) and the network mode in
   `zest_radio_heracles224g.conf`.
2. Run `te_udp_echo.py` and `te_udp_receive.py` (from `zephyr/samples/net/cellular_modem/server`)
   on a host reachable from Internet, and set its name in
   `CONFIG_SAMPLE_CELLULAR_MODEM_ENDPOINT_HOSTNAME`.
3. Build (from the workspace root):

```shell
west build -p -b zest_core_nrf52832 --shield zest_radio_heracles224g zephyr/samples/net/cellular_modem -- \
  -DDTC_OVERLAY_FILE=$PWD/6tron/drivers/simcom_heracles224g/samples/zephyr_cellular_modem/zest_radio_heracles224g.overlay \
  -DEXTRA_CONF_FILE=$PWD/6tron/drivers/simcom_heracles224g/samples/zephyr_cellular_modem/zest_radio_heracles224g.conf
```

If the driver and the shield are not yet declared in the west manifest, also add:
`-DZEPHYR_EXTRA_MODULES="$PWD/6tron/drivers/simcom_heracles224g;$PWD/6tron/shields/zest_radio_heracles224g"`.

:warning: The UART runs at 115200 bauds without hardware flow control: the sample throughput test
may report dropped packets.
