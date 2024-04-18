message("aem-manager component is included")
include(${GenAVBPath}/apps/common/aem-manager/aem-audio-entities.cmake)

set(AEM_MANAGER_RTOS_TARGET ${MCUX_SDK_PROJECT_NAME})
include(${GenAVBPath}/apps/rtos/aem-manager/aem-manager-rtos.cmake)

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AEM_ENTITIES}
    ${AEM_MANAGER_HELPERS_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/aem_manager.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AEM_MANAGER_HELPERS_HEADER_DIR}
)
