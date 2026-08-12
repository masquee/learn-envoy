cmake_minimum_required(VERSION 3.20)

project(learn_envoy_cpp_examples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(absl CONFIG REQUIRED)

function(add_learning_example target source)
  add_executable(${target} ${source})
  target_compile_options(${target} PRIVATE
    $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall;-Wextra;-Wpedantic>
  )
  add_test(NAME ${target} COMMAND ${target})
endfunction()

enable_testing()

add_learning_example(status_or 01_status_or.cc)
target_link_libraries(status_or PRIVATE absl::statusor absl::strings)

add_learning_example(string_view 02_string_view.cc)
target_link_libraries(string_view PRIVATE absl::strings)

add_learning_example(optional 03_optional.cc)

add_learning_example(span 04_span.cc)
target_link_libraries(span PRIVATE absl::span)

add_learning_example(flat_hash_map 05_flat_hash_map.cc)
target_link_libraries(flat_hash_map PRIVATE absl::flat_hash_map absl::strings)

add_learning_example(strings 06_strings.cc)
target_link_libraries(strings PRIVATE absl::strings)

add_learning_example(cleanup 07_cleanup.cc)
target_link_libraries(cleanup PRIVATE absl::cleanup)

add_learning_example(smart_pointers 08_smart_pointers.cc)

add_learning_example(lambdas_callbacks 09_lambdas_callbacks.cc)

add_learning_example(variant_basics 10_variant_basics.cc)

add_learning_example(variant_visit 11_variant_visit.cc)

add_learning_example(variant_overloaded 12_variant_overloaded.cc)

add_learning_example(inlined_vector 13_inlined_vector.cc)
target_link_libraries(inlined_vector PRIVATE absl::inlined_vector)
