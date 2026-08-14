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

add_executable(nbcraft-civilization-persistence-tests
    civilization/CivilizationPersistenceTests.cpp
)

target_include_directories(nbcraft-civilization-persistence-tests PRIVATE ../source)
target_link_libraries(nbcraft-civilization-persistence-tests PRIVATE nbcraft-core)
add_test(NAME nbcraft-civilization-persistence-tests COMMAND nbcraft-civilization-persistence-tests)

add_executable(nbcraft-civilization-file-storage-tests
    civilization/CivilizationFileStorageTests.cpp
)

target_include_directories(nbcraft-civilization-file-storage-tests PRIVATE ../source)
target_link_libraries(nbcraft-civilization-file-storage-tests PRIVATE nbcraft-core)
add_test(NAME nbcraft-civilization-file-storage-tests COMMAND nbcraft-civilization-file-storage-tests)
