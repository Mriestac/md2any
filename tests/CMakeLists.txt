add_executable(test_smoke
    test_smoke.cpp
)

target_link_libraries(test_smoke
    PRIVATE
        Qt6::Core
        Qt6::Test
)

add_test(NAME test_smoke COMMAND test_smoke)

add_executable(test_models
    test_models.cpp
)

target_link_libraries(test_models
    PRIVATE
        md2any_core
        Qt6::Core
        Qt6::Test
)

add_test(NAME test_models COMMAND test_models)

add_executable(test_pandoc_runner
    test_pandoc_runner.cpp
)

add_executable(fake_pandoc
    fake_pandoc.cpp
)

target_link_libraries(fake_pandoc
    PRIVATE
        Qt6::Core
)

target_link_libraries(test_pandoc_runner
    PRIVATE
        md2any_core
        Qt6::Core
        Qt6::Test
)

add_dependencies(test_pandoc_runner fake_pandoc)

add_test(NAME test_pandoc_runner COMMAND test_pandoc_runner)

add_executable(test_pandoc_formats
    test_pandoc_formats.cpp
)

target_link_libraries(test_pandoc_formats
    PRIVATE
        md2any_core
        Qt6::Core
        Qt6::Test
)

add_dependencies(test_pandoc_formats fake_pandoc)

add_test(NAME test_pandoc_formats COMMAND test_pandoc_formats)

add_executable(test_conversion_process
    test_conversion_process.cpp
)

target_link_libraries(test_conversion_process
    PRIVATE
        md2any_core
        Qt6::Core
        Qt6::Test
)

add_dependencies(test_conversion_process fake_pandoc)

add_test(NAME test_conversion_process COMMAND test_conversion_process)

add_executable(test_app_controller
    test_app_controller.cpp
    ../src/app_controller.cpp
)

target_include_directories(test_app_controller
    PRIVATE
        ../src
)

target_link_libraries(test_app_controller
    PRIVATE
        md2any_core
        Qt6::Core
        Qt6::Gui
        Qt6::Test
)

add_dependencies(test_app_controller fake_pandoc)

add_test(NAME test_app_controller COMMAND test_app_controller)

add_executable(test_settings_store
    test_settings_store.cpp
)

target_link_libraries(test_settings_store
    PRIVATE
        md2any_core
        Qt6::Core
        Qt6::Test
)

add_test(NAME test_settings_store COMMAND test_settings_store)
