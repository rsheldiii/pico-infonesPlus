# pico-infonesPlus 
cmake_minimum_required(VERSION 3.13)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Add new build type RELEASENODEBUG
# usage
# cmake -DCMAKE_BUILD_TYPE=RELEASENODEBUG ..
message("* Adding build types...")
SET(GCC_RELEASENODEBUG_FLAGS "-O2 -DNDEBUG")
SET(CMAKE_ASM_FLAGS_RELEASENODEBUG
    "${GCC_RELEASENODEBUG_FLAGS}"
    CACHE STRING "Flags used by the ASM compiler during REL_O2 builds."
    FORCE )

SET(CMAKE_CXX_FLAGS_RELEASENODEBUG
    "${GCC_RELEASENODEBUG_FLAGS}"
    CACHE STRING "Flags used by the C++ compiler during REL_O2 builds."
    FORCE )
SET(CMAKE_C_FLAGS_RELEASENODEBUG
    "${GCC_RELEASENODEBUG_FLAGS}"
    CACHE STRING "Flags used by the C compiler during REL_O2 builds."
    FORCE )
SET(CMAKE_EXE_LINKER_FLAGS_RELEASENODEBUG
    ""
    CACHE STRING "Flags used for linking binaries during REL_O2 builds."
    FORCE )
SET(CMAKE_SHARED_LINKER_FLAGS_RELEASENODEBUG
    ""
    CACHE STRING "Flags used by the shared libraries linker during REL_O2 builds."
    FORCE )

MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_RELEASENODEBUG
    CMAKE_ASM_FLAGS_RELEASENODEBUG
    CMAKE_C_FLAGS_RELEASENODEBUG
    CMAKE_EXE_LINKER_FLAGS_RELEASENODEBUG
    CMAKE_SHARED_LINKER_FLAGS_RELEASENODEBUG )

IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Debug
       CACHE STRING "Choose the type of build : None Debug Release RelWithDebInfo MinSizeRel REL_O2."
       FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
message("* Current build type is : ${CMAKE_BUILD_TYPE}")

add_definitions( -DNDEBUG )
include(pico_sdk_import.cmake)

project(piconesPlus C CXX ASM)
pico_sdk_init()
# IF(EXISTS "${PICO_SDK_PATH}/lib/mbedtls")
#     message(FATAL_ERROR "\nPico SDK 1.5.0 or higher cannot be used to build this project.\n"
#                         "Remove build subdirectory and use SDK 1.4.0 to build this project.\n"
#                         "See https://github.com/fhoedemakers/pico-infonesPlus/issues/7")
# ENDIF()
add_subdirectory(drivers/fatfs)
add_subdirectory(drivers/sdcard)


# Screen configuration for this hardware
set(PICO_SCREENS_WIDTH "255" CACHE STRING "Screen width in pixels")
set(PICO_SCREENS_HEIGHT "240" CACHE STRING "Screen height in pixels") 
set(PICO_SCREENS_DOWNSAMPLING_FACTOR "199" CACHE STRING "Downsampling factor as percentage")

add_subdirectory(pico-screens)
# add_subdirectory(pico-screens)


if (NOT INFONES_PLUS_HW_CONFIG)
    set(INFONES_PLUS_HW_CONFIG 1 CACHE STRING "Select the hardware configuration for your board")       
endif()

if ( INFONES_PLUS_HW_CONFIG EQUAL 1 )
	# This default Config is for Pimoroni Pico DV Demo Base
	set(DVICONFIG "dviConfig_PimoroniDemoDVSock" CACHE STRING
	  "Select a default pin configuration from common_dvi_pin_configs.h")
    set(LED_GPIO_PIN "25" CACHE STRING "Select the GPIO pin for LED")
	set(SD_CS "22" CACHE STRING "Specify the Chip Select GPIO pin for the SD card")
	set(SD_SCK "5" CACHE STRING "Specify de Clock GPIO pin for the SD card")
	set(SD_MOSI "18" CACHE STRING "Select the Master Out Slave In GPIO pin for the SD card")
	set(SD_MISO "19" CACHE STRING "Select the Master In Slave Out GPIO pin for the SD card")
	set(NES_CLK "14" CACHE STRING "Select the Clock GPIO pin for NES controller")
	set(NES_DATA "15" CACHE STRING "Select the Data GPIO pin for NES controller")
	set(NES_LAT "16" CACHE STRING "Select the Latch GPIO pin for NES controller")
	set(WII_SDA "-1" CACHE STRING "Select the SDA GPIO pin for Wii Classic controller")
	set(WII_SCL "-1" CACHE STRING "Select the SCL GPIO pin for Wii Classic controller")
elseif ( INFONES_PLUS_HW_CONFIG EQUAL 2 )
	# --------------------------------------------------------------------
	# Alternate config for use with different SDcard reader and HDMI board
	# --------------------------------------------------------------------
	# Adafruit DVI Breakout For HDMI Source Devices https://www.adafruit.com/product/4984
	set(DVICONFIG "dviConfig_PicoDVISock" CACHE STRING
	  "Select a default pin configuration from common_dvi_pin_configs.h")
    set(LED_GPIO_PIN "25" CACHE STRING "Select the GPIO pin for LED")
	# Adafruit Micro-SD breakout board+ https://www.adafruit.com/product/254 
	set(SD_CS "5" CACHE STRING "Specify the Chip Select GPIO pin for the SD card")
	set(SD_SCK "2" CACHE STRING "Specify de Clock GPIO pin for the SD card")
	set(SD_MOSI "3" CACHE STRING "Select the Master Out Slave In GPIO pin for the SD card")
	set(SD_MISO "4" CACHE STRING "Select the Master In Slave Out GPIO pin for the SD card")
	set(NES_CLK "6" CACHE STRING "Select the Clock GPIO pin for NES controller")
	set(NES_DATA "7" CACHE STRING "Select the Data GPIO pin for NES controller")
	set(NES_LAT "8" CACHE STRING "Select the Latch GPIO pin for NES controller")
	set(WII_SDA "-1" CACHE STRING "Select the SDA GPIO pin for Wii Classic controller")
	set(WII_SCL "-1" CACHE STRING "Select the SCL GPIO pin for Wii Classic controller")
elseif ( INFONES_PLUS_HW_CONFIG EQUAL 3 )
	# --------------------------------------------------------------------
	# Alternate config for use with Adafruit Feather RP2040 DVI + SD Wing
	# --------------------------------------------------------------------
	set(DVICONFIG "dviConfig_AdafruitFeatherDVI" CACHE STRING
	  "Select a default pin configuration from common_dvi_pin_configs.h")
    set(LED_GPIO_PIN "13" CACHE STRING "Select the GPIO pin for LED")
	set(SD_CS "10" CACHE STRING "Specify the Chip Select GPIO pin for the SD card")
	set(SD_SCK "14" CACHE STRING "Specify de Clock GPIO pin for the SD card")
	set(SD_MOSI "15" CACHE STRING "Select the Master Out Slave In GPIO pin for the SD card")
	set(SD_MISO "8" CACHE STRING "Select the Master In Slave Out GPIO pin for the SD card")
	set(NES_CLK "5" CACHE STRING "Select the Clock GPIO pin for NES controller")
	set(NES_DATA "6" CACHE STRING "Select the Data GPIO pin for NES controller")
	set(NES_LAT "9" CACHE STRING "Select the Latch GPIO pin for NES controller")
	set(WII_SDA "2" CACHE STRING "Select the SDA GPIO pin for Wii Classic controller")
	set(WII_SCL "3" CACHE STRING "Select the SCL GPIO pin for Wii Classic controller")
elseif ( INFONES_PLUS_HW_CONFIG EQUAL 4 )
    # --------------------------------------------------------------------
	# Alternate config for use with Waveshare RP2040-PiZero
	# --------------------------------------------------------------------
	set(DVICONFIG "dviConfig_WaveShareRp2040" CACHE STRING
    "Select a default pin configuration from common_dvi_pin_configs.h")
    set(LED_GPIO_PIN "-1" CACHE STRING "Select the GPIO pin for LED")
    set(SD_CS "21" CACHE STRING "Specify the Chip Select GPIO pin for the SD card")
    set(SD_SCK "18" CACHE STRING "Specify de Clock GPIO pin for the SD card")
    set(SD_MOSI "19" CACHE STRING "Select the Master Out Slave In GPIO pin for the SD card")
    set(SD_MISO "20" CACHE STRING "Select the Master In Slave Out GPIO pin for the SD card")
    set(NES_CLK "5" CACHE STRING "Select the Clock GPIO pin for NES controller")
    set(NES_DATA "6" CACHE STRING "Select the Data GPIO pin for NES controller")
    set(NES_LAT "9" CACHE STRING "Select the Latch GPIO pin for NES controller")
    set(WII_SDA "-1" CACHE STRING "Select the SDA GPIO pin for Wii Classic controller")
    set(WII_SCL "-1" CACHE STRING "Select the SCL GPIO pin for Wii Classic controller") 
endif ( )

# --------------------------------------------------------------------
message("HDMI board type     : ${DVICONFIG}")
message("SD card CS          : ${SD_CS}")
message("SD card SCK         : ${SD_SCK}")
message("SD card MOSI        : ${SD_MOSI}")
message("SD card MISO        : ${SD_MISO}")
message("NES controller CLK  : ${NES_CLK}")
message("NES controller DATA : ${NES_DATA}")
message("NES controller LAT  : ${NES_LAT}")
message("Wii controller SDA  : ${WII_SDA}")
message("Wii controller SCL  : ${WII_SCL}")
message("LED pin             : ${LED_GPIO_PIN}")

add_executable(piconesPlus
    main.cpp
    hid_app.cpp
    gamepad.cpp
    nespad.cpp
    wiipad.cpp
    menu.cpp
    RomLister.cpp
    FrensHelpers.cpp
    pico-screens/screens/shared.c
    pico-screens/screen.c
    # pico-screens/screens/lilygo_ttgo.c
    # pico-screens/screens/st7789_240_135.c
    # pico-screens/screens/ssd1306_70_40.cpp
    # pico-screens/screens/ssd1306_70_40_i2c.c
    pico-screens/screens/st7735_128_128.c
)

pico_set_program_name(piconesPlus "piconesPlus")
pico_set_program_version(piconesPlus "0.8")

pico_enable_stdio_uart(piconesPlus 0)
pico_enable_stdio_usb(piconesPlus 0)

# tinyusb
set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_DEBUG_LEVEL 1)
include(${PICO_TINYUSB_PATH}/hw/bsp/${FAMILY}/family.cmake)
family_configure_target(piconesPlus)

target_compile_definitions(piconesPlus PRIVATE
    DVICONFIG=${DVICONFIG}
    SDCARD_PIN_SPI0_CS=${SD_CS}
    SDCARD_PIN_SPI0_SCK=${SD_SCK}
    SDCARD_PIN_SPI0_MOSI=${SD_MOSI}
    SDCARD_PIN_SPI0_MISO=${SD_MISO}
    SDCARD_PIO=pio1
    SDCARD_PIO_SM=0
    NES_PIN_CLK=${NES_CLK}
    NES_PIN_DATA=${NES_DATA}
    NES_PIN_LAT=${NES_LAT}
    WII_PIN_SDA=${WII_SDA}
    WII_PIN_SCL=${WII_SCL}
    LED_GPIO_PIN=${LED_GPIO_PIN}

    ST7735_128_128=1
    # Remove hard-coded DOWNSAMPLING_FACTOR - now defined through pico-screens
    # DOWNSAMPLING_FACTOR_OUT_OF_100=199

    # hagl-hal stuff
    
    HAGL_HAL_USE_SINGLE_BUFFER=1

    # screen-specific

    MIPI_DISPLAY_PIN_DC=1
    MIPI_DISPLAY_PIN_CS=5
    MIPI_DISPLAY_PIN_CLK=2 # 18
    MIPI_DISPLAY_PIN_MOSI=3 # 19
    MIPI_DISPLAY_PIN_RST=0
    MIPI_DISPLAY_PIN_BL=22
    MIPI_DISPLAY_PIN_MISO=-1

    MIPI_DISPLAY_SPI_PORT=spi0
    MIPI_DISPLAY_SPI_CLOCK_SPEED_HZ=62500000
    MIPI_DISPLAY_PIXEL_FORMAT=MIPI_DCS_PIXEL_FORMAT_16BIT

    # MIPI_DISPLAY_ADDRESS_MODE=MIPI_DCS_ADDRESS_MODE_BGR|MIPI_DCS_ADDRESS_MODE_MIRROR_Y|MIPI_DCS_ADDRESS_MODE_MIRROR_X
    # MIPI_DISPLAY_ADDRESS_MODE=MIPI_DCS_ADDRESS_MODE_BGR|MIPI_DCS_ADDRESS_MODE_SWAP_XY|MIPI_DCS_ADDRESS_MODE_MIRROR_Y # this was for 160x80
    MIPI_DISPLAY_ADDRESS_MODE=MIPI_DCS_ADDRESS_MODE_BGR|MIPI_DCS_ADDRESS_MODE_MIRROR_Y|MIPI_DCS_ADDRESS_MODE_MIRROR_X
    
    # MIPI_DISPLAY_WIDTH=160
    # MIPI_DISPLAY_HEIGHT=80
    # MIPI_DISPLAY_OFFSET_X=0
    # MIPI_DISPLAY_OFFSET_Y=26

    MIPI_DISPLAY_WIDTH=128
    MIPI_DISPLAY_HEIGHT=128
    MIPI_DISPLAY_OFFSET_X=2
    MIPI_DISPLAY_OFFSET_Y=1

    MIPI_DISPLAY_INVERT=1
)
target_link_libraries(piconesPlus PRIVATE
    pico_stdlib
    pico_multicore
    hardware_dma
    hardware_pio
    hardware_i2c
    hardware_interp
    hardware_timer
    hardware_clocks
    hardware_pwm
    hardware_flash
    dvi
    util
    infones
    tinyusb_host
    sdcard
    fatfs
    pico-screens
    )

    target_include_directories(piconesPlus
    PRIVATE
        pico_lib
        infones
        assets
        include
        .
    )

    add_subdirectory(pico_lib/dvi)
    add_subdirectory(pico_lib/util)
    add_subdirectory(infones)

    pico_add_extra_outputs(piconesPlus)



