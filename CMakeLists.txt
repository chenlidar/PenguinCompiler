project(PenguinCompiler)
cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_COMPILER "clang++-10")
file(GLOB_RECURSE SRC "src/*.cpp")
add_compile_options(-O2 -g -std=c++11)
include_directories(src)
include_directories(test)
add_executable(test-asm ${SRC})

file(GLOB_RECURSE FUNCTESTCASE "test/functional/*.sy")
file(GLOB_RECURSE PERFTESTCASE "test/performance/*.sy")

add_custom_target(
	func
	DEPENDS test-asm
)

add_custom_target(
	perf
	DEPENDS test-asm
)

add_custom_target(
	test
	DEPENDS func perf
)

set(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

foreach(file ${FUNCTESTCASE})
	file(RELATIVE_PATH FUNCTESTCASE_REL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/test/functional ${file})
	string(REGEX REPLACE "([0-9_a-zA-Z\-]*).(sy)" "\\1" FUNCTEST_NAME ${FUNCTESTCASE_REL_FILE})
	add_custom_target(
		${FUNCTEST_NAME}
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test/script/test-functional.sh test_single ${FUNCTEST_NAME}
		DEPENDS test-asm
	)
	add_dependencies(func ${FUNCTEST_NAME})
endforeach(file ${FUNCTESTCASE})

foreach(file ${PERFTESTCASE})
	file(RELATIVE_PATH PERFTESTCASE_REL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/test/performance ${file})
	string(REGEX REPLACE "([0-9_a-zA-Z\-]*).(sy)" "\\1" PERFTEST_NAME ${PERFTESTCASE_REL_FILE})
	add_custom_target(
		${PERFTEST_NAME}
		COMMAND	${CMAKE_CURRENT_SOURCE_DIR}/test/script/perf.sh test_single ${PERFTEST_NAME}
		DEPENDS test-asm
	)
	add_dependencies(perf ${PERFTEST_NAME})
endforeach(file ${PERFTESTCASE})