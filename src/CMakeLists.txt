add_executable(Leap_And_Land
        main.c
        common.c
        menu.c
        game.c
        utils/input.c
        utils/audio.c
        utils/display.c
        kun.c
        help.c)

target_include_directories(Leap_And_Land PRIVATE
        ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(Leap_And_Land PRIVATE
        PkgConfig::SDL2
        PkgConfig::SDL2_IMAGE
        PkgConfig::SDL2_MIXER
        PkgConfig::SDL2_TTF)

if (NOT WIN32)
    target_link_libraries(Leap_And_Land PRIVATE m)
endif ()
