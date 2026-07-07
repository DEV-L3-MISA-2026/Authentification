cmake_minimum_required(VERSION 3.28)

project(FacialAuthentification VERSION 1.0 LANGUAGES CXX)
find_package(OpenCV 4 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(PQXX REQUIRED IMPORTED_TARGET libpqxx)
pkg_check_modules(SNDFILE REQUIRED sndfile)
pkg_check_modules(SAMPLERATE REQUIRED samplerate)
pkg_check_modules(RNNOISE REQUIRED rnnoise)

set(ONNXRUNTIME_DIR "/opt/onnxruntime-linux-x64-1.16.3")

include_directories(
    ${SNDFILE_INCLUDE_DIRS}
    ${SAMPLERATE_INCLUDE_DIRS}
    ${ONNXRUNTIME_DIR}/include
)


link_directories(
    ${SNDFILE_LIBRARY_DIRS}
    ${SAMPLERATE_LIBRARY_DIRS}
    ${ONNXRUNTIME_DIR}/lib
)

add_executable(app 
    main.cpp
    src/face_api_server.cpp
    src/FaceDetector.cpp
    src/FaceRecognizer.cpp
    src/utility.cpp
    src/FacesModel.cpp
    src/AudioProcessor.cpp
    src/SpeechProcessor.cpp
    src/Repository/AuthRepository.cpp
    src/MemoryAudioFile.cpp
    src/Vad.cpp
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    /usr/include
)

target_compile_definitions(app PRIVATE CROW_USE_BOOST)

target_link_libraries(app
    PRIVATE  ${OpenCV_LIBS}
        PkgConfig::PQXX
        ${SNDFILE_LIBRARIES}
        ${SAMPLERATE_LIBRARIES}
        onnxruntime
        ${RNNOISE_LIBRARIES}
)