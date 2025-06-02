if(NOT ENABLE_RUNIN)
  return()
endif()

execute_process(
  COMMAND python3 ${PROJECT_SOURCE_DIR}/platforms/docker/parse.py
  OUTPUT_VARIABLE parse_result
)

string(REPLACE "\n" ";" parse_result "${parse_result}")

foreach(line ${parse_result})
  string(REPLACE "#" ";" info "${line}")
  list(GET info 0 name)
  list(GET info 1 image)
  list(GET info 2 options)
  list(GET info 3 cmd)

  add_custom_target(pull_${name}
    COMMAND bash -c "if ! docker image inspect ${image} > /dev/null 2>&1; then echo 'Pulling ${image}...' && docker pull ${image}; fi"
    COMMENT "Pulling Docker image '${name}' if needed"
    VERBATIM
  )
  set(docker_command "docker run --rm -it \
    -v ${PROJECT_SOURCE_DIR}:${PROJECT_SOURCE_DIR} \
    -v ${PROJECT_SOURCE_DIR}/build/docker_images/${name}:${PROJECT_SOURCE_DIR}/build \
    -w ${PROJECT_SOURCE_DIR} ${options} ${image} ${cmd}"
  )
  add_custom_target(run_in_${name}
    COMMAND ${CMAKE_COMMAND} -E echo "Starting container: ${name} (based on image: ${image})"
    COMMAND bash -c ${docker_command}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running in ${name} container"
    DEPENDS pull_${name}
    VERBATIM
  )
endforeach()

add_custom_target(run_in_list)

foreach(line ${parse_result})
  string(REPLACE "#" ";" info "${line}")
  list(GET info 0 name)
  list(GET info 1 image)
  
  add_custom_command(
    TARGET run_in_list
    COMMAND ${CMAKE_COMMAND} -E echo "  run_in_${name} - image: ${image}"
    VERBATIM
  )
endforeach()

add_custom_command(
  TARGET run_in_list
  PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Available container targets:"
  COMMENT "List of available run_in targets"
  VERBATIM
)
