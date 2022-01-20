SET(CMAKE_ASM_FLAGS_DEBUG " \
    -DDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -DGUEST \
    -g \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -std=gnu99 \
")
SET(CMAKE_ASM_FLAGS_RELEASE " \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -DGUEST \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -std=gnu99 \
")
SET(CMAKE_ASM_FLAGS_DDR_DEBUG " \
    -DDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -DGUEST \
    -g \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -std=gnu99 \
")
SET(CMAKE_ASM_FLAGS_DDR_RELEASE " \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -DGUEST \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -std=gnu99 \
")
SET(CMAKE_ASM_FLAGS_RAM_RELEASE " \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -DGUEST \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -std=gnu99 \
")
SET(CMAKE_ASM_FLAGS_FLASH_DEBUG " \
    -DDEBUG \
    -DFLASH_TARGET \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -DGUEST \
    -g \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -std=gnu99 \
")
SET(CMAKE_C_FLAGS_DEBUG " \
    -DDEBUG \
    -DCPU_MIMX8MN6DVTJZ_ca53 \
    -DGUEST \
    -DFSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DFSL_RTOS_FREE_RTOS \
    -g \
    -O0 \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -std=gnu99 \
    ${app_extra_flags} \
")
SET(CMAKE_C_FLAGS_RELEASE " \
    -DNDEBUG \
    -DCPU_MIMX8MN6DVTJZ_ca53 \
    -DGUEST \
    -DFSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DFSL_RTOS_FREE_RTOS \
    -Os \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -std=gnu99 \
    ${app_extra_flags} \
")
SET(CMAKE_C_FLAGS_DDR_DEBUG " \
    -DDEBUG \
    -DCPU_MIMX8MN6DVTJZ_ca53 \
    -DGUEST \
    -DFSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DFSL_RTOS_FREE_RTOS \
    -g \
    -O0 \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -std=gnu99 \
    ${app_extra_flags} \
")
SET(CMAKE_C_FLAGS_DDR_RELEASE " \
    -DNDEBUG \
    -DCPU_MIMX8MN6DVTJZ_ca53 \
    -DGUEST \
    -DFSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DFSL_RTOS_FREE_RTOS \
    -Os \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -std=gnu99 \
    ${app_extra_flags} \
")
SET(CMAKE_C_FLAGS_FLASH_DEBUG " \
    -DDEBUG \
    -DFLASH_TARGET \
    -DCPU_MIMX8MN6DVTJZ_ca53 \
    -DGUEST \
    -DFSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DFSL_RTOS_FREE_RTOS \
    -g \
    -O0 \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -std=gnu99 \
    ${app_extra_flags} \
")
SET(CMAKE_CXX_FLAGS_DEBUG " \
    -DDEBUG \
    -DCPU_MIMX8MN6DVTJZ_ca53 \
    -DGUEST \
    -DFSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL \
    -DSERIAL_PORT_TYPE_UART=1 \
    -g \
    -O0 \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -fno-rtti \
    -fno-exceptions \
")
SET(CMAKE_CXX_FLAGS_RELEASE " \
    -DNDEBUG \
    -DCPU_MIMX8MN6DVTJZ_ca53 \
    -DGUEST \
    -DFSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL \
    -DSERIAL_PORT_TYPE_UART=1 \
    -Os \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -fno-rtti \
    -fno-exceptions \
")
SET(CMAKE_CXX_FLAGS_DDR_DEBUG " \
    -DDEBUG \
    -DCPU_MIMX8MN6DVTJZ_ca53 \
    -DGUEST \
    -DFSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL \
    -DSERIAL_PORT_TYPE_UART=1 \
    -g \
    -O0 \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -fno-rtti \
    -fno-exceptions \
")
SET(CMAKE_CXX_FLAGS_DDR_RELEASE " \
    -DNDEBUG \
    -DCPU_MIMX8MN6DVTJZ_ca53 \
    -DGUEST \
    -DFSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL \
    -DSERIAL_PORT_TYPE_UART=1 \
    -Os \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -fno-rtti \
    -fno-exceptions \
")
SET(CMAKE_CXX_FLAGS_RAM_RELEASE " \
    -DNDEBUG \
    -DCPU_MIMX8MN6DVTJZ_ca53 \
    -DGUEST \
    -DFSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL \
    -DSERIAL_PORT_TYPE_UART=1 \
    -Os \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    -Wno-address-of-packed-member \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -fno-rtti \
    -fno-exceptions \
")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG " \
    -g \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    --specs=nosys.specs \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -Xlinker \
    --gc-sections \
    -Xlinker \
    -static \
    -Xlinker \
    -z \
    -Xlinker \
    muldefs \
    -Xlinker \
    -Map=output.map \
    -T${BoardPath}/armgcc_aarch64/MIMX8MN6xxxxx_ca53_ram.ld -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE " \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    --specs=nosys.specs \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -Xlinker \
    --gc-sections \
    -Xlinker \
    -static \
    -Xlinker \
    -z \
    -Xlinker \
    muldefs \
    -Xlinker \
    -Map=output.map \
    -T${BoardPath}/armgcc_aarch64/MIMX8MN6xxxxx_ca53_ram.ld -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_DDR_DEBUG " \
    -g \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    --specs=nosys.specs \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -Xlinker \
    --gc-sections \
    -Xlinker \
    -static \
    -Xlinker \
    -z \
    -Xlinker \
    muldefs \
    -Xlinker \
    -Map=output.map \
    -T${BoardPath}/armgcc_aarch64/MIMX8MN6xxxxx_ca53_ddr_ram.ld -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_DDR_RELEASE " \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    --specs=nosys.specs \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -Xlinker \
    --gc-sections \
    -Xlinker \
    -static \
    -Xlinker \
    -z \
    -Xlinker \
    muldefs \
    -Xlinker \
    -Map=output.map \
    -T${BoardPath}/armgcc_aarch64/MIMX8MN6xxxxx_ca53_ddr_ram.ld -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_RAM_RELEASE " \
    -march=armv8.1-a \
    -mtune=cortex-a53 \
    -Wall \
    --specs=nosys.specs \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -Xlinker \
    --gc-sections \
    -Xlinker \
    -static \
    -Xlinker \
    -z \
    -Xlinker \
    muldefs \
    -Xlinker \
    -Map=output.map \
    -T${BoardPath}/armgcc_aarch64/MIMX8MN6xxxxx_ca53_ram.ld -static \
")
