add_executable(nbcraft-civilization-tests
    civilization/CivilizationModelTests.cpp
    ../source/world/civilization/CivilizationTypes.cpp
    ../source/world/civilization/Settlement.cpp
)

target_include_directories(nbcraft-civilization-tests PRIVATE ../source)
add_test(NAME nbcraft-civilization-tests COMMAND nbcraft-civilization-tests)

add_executable(nbcraft-civilization-manager-tests
    civilization/CivilizationManagerTests.cpp
    ../source/world/civilization/CivilizationTypes.cpp
    ../source/world/civilization/Settlement.cpp
    ../source/world/civilization/CivilizationManager.cpp
)

target_include_directories(nbcraft-civilization-manager-tests PRIVATE ../source)
add_test(NAME nbcraft-civilization-manager-tests COMMAND nbcraft-civilization-manager-tests)
