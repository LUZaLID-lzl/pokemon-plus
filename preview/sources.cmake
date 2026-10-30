set(PP_PREVIEW_SOURCES)
foreach(source
 preview/preview.c preview/simulator_window.c preview/random_seed.c
 core/touch_input.c main/image_assets.c main/game_ui.c main/intro_view.c main/fishing_view.c main/hospital_view.c main/training_view.c
 main/pp_font_intro_24.c main/pp_font_intro_32.c main/pp_font_intro_20.c
 core/intro.c main/pp_font_14.c main/pp_clock_36.c main/pp_clock_38.c
 core/game.c core/training.c core/training_job.c core/moves.c core/home_behavior.c core/home_conflict.c core/scene_fx.c core/scene_motion.c main/scene_view.c main/scene_effect_view.c
 core/pedometer.c main/battle_view.c main/pp_font_12.c main/pp_font_shop_10.c main/pp_font_18.c
 main/pp_font_nav_14.c main/pp_font_title_22.c)
 list(APPEND PP_PREVIEW_SOURCES "${CMAKE_CURRENT_LIST_DIR}/../${source}")
endforeach()
