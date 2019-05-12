SHELL:=/bin/bash		# needed by bashism here and in zephyr-env.sh
ZEPHYR_PROJECT_DIR:=$(HOME)/zephyrproject

all: build

define setup_zephyr_env
	pushd $(ZEPHYR_PROJECT_DIR)/zephyr > /dev/null; \
	source zephyr-env.sh; \
	popd > /dev/null
endef

build:
	@$(call setup_zephyr_env); ninja -C ../build

flash:
	@$(call setup_zephyr_env); ninja -C ../build flash

menuconfig:
	@$(call setup_zephyr_env); ninja -C ../build menuconfig
