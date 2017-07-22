deps_config := \
	/home/lieven/esp/esp-idf/components/app_trace/Kconfig \
	/home/lieven/workspace/limero-steer/components/arduino-esp32/Kconfig \
	/home/lieven/esp/esp-idf/components/aws_iot/Kconfig \
	/home/lieven/esp/esp-idf/components/bt/Kconfig \
	/home/lieven/esp/esp-idf/components/esp32/Kconfig \
	/home/lieven/esp/esp-idf/components/ethernet/Kconfig \
	/home/lieven/esp/esp-idf/components/fatfs/Kconfig \
	/home/lieven/esp/esp-idf/components/freertos/Kconfig \
	/home/lieven/esp/esp-idf/components/log/Kconfig \
	/home/lieven/esp/esp-idf/components/lwip/Kconfig \
	/home/lieven/esp/esp-idf/components/mbedtls/Kconfig \
	/home/lieven/esp/esp-idf/components/openssl/Kconfig \
	/home/lieven/esp/esp-idf/components/spi_flash/Kconfig \
	/home/lieven/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/lieven/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/lieven/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/lieven/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
