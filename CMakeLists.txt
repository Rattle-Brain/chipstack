cmake_minimum_required(VERSION 3.14)

# Give files extracted by FetchContent URL downloads the extraction time as
# their timestamp, so a changed URL always triggers a rebuild (CMake >= 3.24).
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
project(chipstack)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    ftxui
    URL https://github.com/ArthurSonzogni/FTXUI/archive/refs/tags/v5.0.0.tar.gz
)
FetchContent_MakeAvailable(ftxui)

# Core game logic library (no UI)
add_library(chipstack_core
    src/core/cards/deck.cpp
    src/core/cards/card.cpp
    src/core/common/player.cpp
    src/core/common/crupier.cpp
    src/core/poker/poker_table.cpp
    src/core/poker/hand_evaluator.cpp
    src/core/poker/betting_round.cpp
    src/core/blackjack/blackjack_table.cpp
    src/ai/bot_player.cpp
    src/ai/hand_strength.cpp
    src/ai/thinking_delay.cpp
)
target_include_directories(chipstack_core PUBLIC src)

# Main executable
add_executable(chipstack
    src/main.cpp
    src/ui/tui_app.cpp
    src/ui/card_renderer.cpp
    src/ui/components/player_seat.cpp
    src/ui/layout/table_layout.cpp
    src/ui/screens/poker_screen.cpp
    src/ui/screens/blackjack_screen.cpp
)
target_include_directories(chipstack PRIVATE src)
target_link_libraries(chipstack
    PRIVATE chipstack_core
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
)

# Tests
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(chipstack_tests
        tests/card_test.cpp
        tests/deck_test.cpp
        tests/hand_evaluator_test.cpp
        tests/poker_table_test.cpp
        tests/blackjack_table_test.cpp
        tests/betting_round_test.cpp
        tests/bot_player_test.cpp
        tests/hand_strength_test.cpp
        tests/thinking_delay_test.cpp
    )
    target_link_libraries(chipstack_tests
        PRIVATE chipstack_core
        PRIVATE GTest::gtest_main
    )
    include(GoogleTest)
    gtest_discover_tests(chipstack_tests)
endif()
