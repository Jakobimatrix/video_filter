add_executable(light_trail src/light_trail.cpp)

target_link_libraries(light_trail
    PRIVATE video_filter)
