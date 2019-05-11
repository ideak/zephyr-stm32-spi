stm32_spi_check_env()
{
	if [ -z "${ZEPHYR_BASE+x}" ]; then
		echo -e \
"ZEPHYR_BASE is not set.\n"\
"Please source zephyr-env.sh from your <Zephyr project dir>/zephyr directory"

		return 1
	fi

	if [ ! -d src ]; then
		echo -e \
"Please source this file from your Zephyr app base dir"

		return 1
	fi

	if [ -d build ]; then
		echo -e \
"The build directory already exist, remove it first if you want reconfigure it"

		return 1
	fi

	if [ -e build ]; then
		echo -e \
"You use 'build' as something else than your build dir"

		return 1
	fi

	return 0
}

stm32_spi_do_setup()
{
	local err

	export CROSS_COMPILE=/usr/bin/arm-none-eabi-
	export ZEPHYR_TOOLCHAIN_VARIANT=cross-compile

	mkdir build
	cd build

	cmake -GNinja -DBOARD=nucleo_f411re ..
	err=$?

	cd ..

	if [ $err -ne 0 ]; then
		echo Removing build
		rm -r build
	fi

	return $err
}

if stm32_spi_check_env && stm32_spi_do_setup; then
	echo -e \
"Setup complete, you can now do:\n"\
"\tcd build\n"\
"\tninja\n"\
"\tninja flash"
fi
