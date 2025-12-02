if(NOT ENABLE_RUNIN)
  return()
endif()

execute_process(
  COMMAND python3 ${PROJECT_SOURCE_DIR}/platforms/docker/parse.py
  OUTPUT_VARIABLE parse_result
)

string(REPLACE "\n" ";" parse_result "${parse_result}")

add_custom_target(run_in_list)

set(img_num 0)
foreach(line ${parse_result})
  string(REPLACE "#" ";" info "${line}")
  list(GET info 0 name)
  list(GET info 1 description)
  list(GET info 2 image)
  list(GET info 3 options)
  list(GET info 4 cmd)

  if("${name}" STREQUAL "list")
    message(FATAL_ERROR "Docker image name 'list' is not allowed")
  endif()

  add_custom_target(pull_${name}
    COMMAND bash -c "if ! docker image inspect ${image} > /dev/null 2>&1; then echo 'Pulling ${image}...' && docker pull ${image}; fi"
    COMMENT "Pulling Docker image '${name}' if needed"
    VERBATIM
  )
  add_custom_target(run_in_${name}
    COMMAND ${CMAKE_COMMAND} -E echo "Starting container: ${name} (based on image: ${image})"
    COMMAND docker run --rm -it --network host --privileged
      -v ${PROJECT_SOURCE_DIR}:${PROJECT_SOURCE_DIR}
      -v ${PROJECT_SOURCE_DIR}/build/docker_images/${name}:${PROJECT_SOURCE_DIR}/build
      -w ${PROJECT_SOURCE_DIR}/build ${options} ${image} ${cmd}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running in ${name} container"
    DEPENDS pull_${name}
    VERBATIM USES_TERMINAL
  )

  math(EXPR img_num "${img_num} + 1")
  add_custom_command(
    TARGET run_in_list
    COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --blue "${img_num}. run_in_${name}: ${description}"
    COMMAND ${CMAKE_COMMAND} -E echo "   image: ${image}"
    COMMAND ${CMAKE_COMMAND} -E echo "   options: ${options}"
    COMMAND ${CMAKE_COMMAND} -E echo "   cmd: ${cmd}"
    VERBATIM USES_TERMINAL
  )
endforeach()

add_custom_command(
  TARGET run_in_list PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --magenta --bold "Available compiler docker images, ${img_num} in total:"
  VERBATIM USES_TERMINAL
)
