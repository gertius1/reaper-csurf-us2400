add_library(
    reaper_plugin OBJECT
    csurf_main.cpp
    csurf_us2400.cpp
    csurf_us2400_helpoverlay.cpp
    csurf_us2400.cpp
    CSurf_US2400_stripoverlay.cpp
    csurf_utils.cpp
)
target_sources(reaper_plugin PRIVATE)

target_include_directories(
  reaper_plugin SYSTEM PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/vendor 
  ${CMAKE_SOURCE_DIR}/vendor/reaper-sdk/sdk)

target_link_libraries(${PROJECT_NAME} reaper_plugin)

if(NOT WIN32)
  find_package(SWELL REQUIRED)
  
  add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/res.rc_mac_dlg
    COMMAND ${SWELL_RESGEN} ${CMAKE_CURRENT_SOURCE_DIR}/res.rc)

  add_custom_target( generate
    ALL ${SWELL_RESGEN} ${CMAKE_CURRENT_SOURCE_DIR}/res.rc
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res.rc)
endif()

if(WIN32)
  target_compile_options(reaper_plugin PUBLIC
    /W3
    /WX
    )
   target_compile_definitions(reaper_plugin PUBLIC NOMINMAX)
   target_link_libraries(reaper_plugin winmm)
elseif(APPLE)
  target_compile_options(reaper_plugin PUBLIC
    -fsigned-char -fobjc-arc -Wextra -Wno-error=deprecated-declarations
  )
else()
  target_compile_options(reaper_plugin PUBLIC
    -fsigned-char -Werror -lX11
  )
endif()

find_package(WDL REQUIRED)
target_link_libraries(reaper_plugin WDL::WDL)

if(NOT WIN32)
  target_link_libraries(reaper_plugin SWELL::swell)
endif()