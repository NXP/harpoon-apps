#Description: Harpoon FreeRTOS specific implentation; user_visible: True
include_guard(GLOBAL)
message("common_freertos component is included.")

mcux_add_source(
    SOURCES
    FreeRTOS_helper.c
    FreeRTOS_tick_config.c
    idle.c
    irq.c
    uart.c
)

mcux_add_include(
    INCLUDES
    .
)

mcux_remove_armgcc_configuration(CC "-Os")

mcux_add_armgcc_configuration(
    TARGETS ddr_debug
    AS "-DDEBUG"
    CC "-DDEBUG -g"
    CX "-DDEBUG -g"
    LD "-g"
)

mcux_add_armgcc_configuration(
    TARGETS ddr_release
    AS "-DNDEBUG"
    CC "-DNDEBUG"
    CX "-DNDEBUG"
)

# Add GUEST flag at application level
mcux_add_armgcc_configuration(
    AS "-DGUEST"
    CC "-DGUEST"
    CX "-DGUEST"
)

if (${core_id} STREQUAL "ca53")
    mcux_add_armgcc_configuration(
        AS "-mcpu=cortex-a53"
        CC "-mcpu=cortex-a53"
        CX "-mcpu=cortex-a53"
        LD "-mcpu=cortex-a53"
    )
elseif (${core_id} STREQUAL "ca55")
    mcux_add_armgcc_configuration(
        AS "-mcpu=cortex-a55"
        CC "-mcpu=cortex-a55"
        CX "-mcpu=cortex-a55"
        LD "-mcpu=cortex-a55"
    )
endif()

mcux_add_macro(
    CC " \
    -DFSL_RTOS_FREE_RTOS \
    -DFSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL \
    -DFSL_ETH_ENABLE_CACHE_CONTROL \
    -DPRINTF_FLOAT_ENABLE=1 \
    -DPRINTF_ADVANCED_ENABLE=1 \
")

# Remove -ffreestanding option, as we need "inttypes.h" for PRI64 defines
mcux_remove_configuration(CC "-ffreestanding")

if(${CONFIG_MCUX_HW_DEVICE_CORE} STREQUAL "MIMX8ML8_ca53")
    mcux_add_macro(
        CC "-DSDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL"
    )
elseif(${CONFIG_MCUX_HW_DEVICE_CORE} STREQUAL "MIMX8MM6_ca53")
    mcux_add_macro(
        CC "-DSDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1800000000UL"
    )
elseif(${CONFIG_MCUX_HW_DEVICE_CORE} STREQUAL "MIMX8MN6_ca53")
    mcux_add_macro(
        CC "-DSDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1600000000UL"
    )
elseif(${CONFIG_MCUX_HW_DEVICE_CORE} STREQUAL "MIMX9352_ca55")
    mcux_add_macro(
        CC "-DSDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1700000000UL"
    )
elseif(${CONFIG_MCUX_HW_DEVICE_CORE} STREQUAL "MIMX9596_ca55")
    mcux_add_macro(
        CC "-DSDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY=1700000000UL"
    )
endif()

if(CONFIG_USE_GENAVB)
    mcux_add_source(
        SOURCES
        stub.c
    )
endif()

set(DDR_LD_FILE "${harpoon_os_board_path}/armgcc_aarch64/${device}xxxxx_${core_id}_ddr_ram.ld")

mcux_remove_armgcc_linker_script(
    TARGETS ddr_debug ddr_release
    BASE_PATH ${SdkRootDirPath}
    LINKER ${device_root}/${soc_portfolio}/${soc_series}/${device}/gcc/${CONFIG_MCUX_TOOLCHAIN_LINKER_DEVICE_PREFIX}_ddr_ram.ld
)

mcux_add_armgcc_linker_script(
    TARGETS ddr_debug ddr_release
    BASE_PATH ${SdkRootDirPath}
    LINKER ${DDR_LD_FILE}
)
