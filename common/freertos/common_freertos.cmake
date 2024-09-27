#Description: Harpoon FreeRTOS specific implentation; user_visible: True
include_guard(GLOBAL)
message("common_freertos component is included.")


target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS_helper.c
    ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS_tick_config.c
    ${CMAKE_CURRENT_LIST_DIR}/idle.c
    ${CMAKE_CURRENT_LIST_DIR}/irq.c
    ${CMAKE_CURRENT_LIST_DIR}/uart.c
)

if(CONFIG_USE_GENAVB)
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/stub.c
    )
endif()

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)
