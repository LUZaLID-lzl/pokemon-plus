idf_component_register(SRCS "rtc_clock.c" "../core/rtc_time.c" "battery_monitor.c" "../core/battery.c" "image_assets.c" "app_main.c" "display_port.c" "../core/touch_input.c" "game_ui.c" "intro_view.c" "fishing_view.c" "hospital_view.c" "training_view.c" "pp_font_intro_24.c" "pp_font_intro_32.c" "pp_font_intro_20.c" "../core/intro.c" "pp_font_14.c" "pp_clock_36.c" "pp_clock_38.c" "../core/game.c" "../core/training.c" "../core/training_job.c" "../core/moves.c" "../core/home_behavior.c" "../core/home_conflict.c" "../core/scene_fx.c" "../core/scene_motion.c" "scene_view.c" "scene_effect_view.c" "../core/pedometer.c" "battle_view.c" "pp_font_12.c" "pp_font_shop_10.c" "pp_font_18.c" "pp_font_nav_14.c" "pp_font_title_22.c"
 INCLUDE_DIRS "." "../core"
 REQUIRES esp_driver_i2c esp_partition spi_flash waveshare__esp32_s3_touch_lcd_1_85B nvs_flash esp_timer esp_wifi esp_netif esp_event esp_driver_usb_serial_jtag)

target_compile_definitions(${COMPONENT_LIB} PRIVATE PP_APP_VERSION="${PROJECT_VER}")

# Native images are directly addressable in Flash through a small path index.
add_custom_target(pp_builtin_assets ALL
 COMMAND ${PYTHON} ${PROJECT_DIR}/tools/prepare_builtin_assets.py
 BYPRODUCTS ${PROJECT_DIR}/build/assets.bin
 VERBATIM)
add_dependencies(flash pp_builtin_assets)
esptool_py_flash_to_partition(flash "assets" "${PROJECT_DIR}/build/assets.bin")
