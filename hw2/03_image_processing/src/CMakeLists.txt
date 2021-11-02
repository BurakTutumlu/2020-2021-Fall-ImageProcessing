project(ceng391_02T CXX)

set(CMAKE_BUILD_TYPE Release)

include(FindPNG)
include_directories(${PNG_INCLUDE_DIR})
link_directories(${PNG_LIBRARY_DIR})

add_executable(image-test image.cc image_test.cc)
target_link_libraries(image-test ${PNG_LIBRARIES})

add_executable(process-per-pixel image.cc process_per_pixel.cc)
target_link_libraries(process-per-pixel ${PNG_LIBRARIES})

add_executable(filter-image image.cc filter_image.cc)
target_link_libraries(filter-image ${PNG_LIBRARIES})

add_executable(rotate-image image.cc rotate_image.cc)
target_link_libraries(rotate-image ${PNG_LIBRARIES})
