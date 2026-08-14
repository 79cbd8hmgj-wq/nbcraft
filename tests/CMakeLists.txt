add_executable(nbcraft-civilization-tests
    civilization/CivilizationModelTests.cpp
    ../source/world/civilization/CivilizationTypes.cpp
    ../source/world/civilization/Settlement.cpp
)

target_include_directories(nbcraft-civilization-tests PRIVATE ../source)
add_test(NAME nbcraft-civilization-tests COMMAND nbcraft-civilization-tests)
