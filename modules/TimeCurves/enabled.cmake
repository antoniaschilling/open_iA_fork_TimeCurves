TARGET_COMPILE_DEFINITIONS(TimeCurves PRIVATE TAPKEE_EIGEN_INCLUDE_FILE=<${TAPKEE_EIGEN_INCLUDE_FILE}>)

if (openiA_TESTING_ENABLED)
	#get_filename_component(CoreSrcDir "../libs/base" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

	add_executable(TimeCurvesTest TimeCurves/iATimeCurvesTest.cpp)
	target_link_libraries(TimeCurvesTest PRIVATE Qt${QT_VERSION_MAJOR}::Core)
	target_include_directories(TimeCurvesTest PRIVATE 
		${CMAKE_CURRENT_SOURCE_DIR}/../libs/base 
		${CMAKE_CURRENT_BINARY_DIR}/../libs
		${CMAKE_CURRENT_BINARY_DIR}           # for _export.h
		${TAPKEE_INCLUDE_DIR}
	)
	target_compile_definitions(TimeCurvesTest PRIVATE TAPKEE_EIGEN_INCLUDE_FILE=<${TAPKEE_EIGEN_INCLUDE_FILE}>)
	add_test(NAME TimeCurvesTest COMMAND TimeCurvesTest)
	if (MSVC)
		string(REGEX REPLACE "/" "\\\\" QT_WIN_DLL_DIR ${QT_LIB_DIR})
		set_tests_properties(TimeCurvesTest PROPERTIES ENVIRONMENT "PATH=${QT_WIN_DLL_DIR};$ENV{PATH};CTEST_OUTPUT_ON_FAILURE=1")
		set_target_properties(TimeCurvesTest PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=${QT_WIN_DLL_DIR};$ENV{PATH}")
	else()
		target_compile_options(TimeCurvesTest PRIVATE -fPIC)
	endif()
	if (openiA_USE_IDE_FOLDERS)
		set_property(TARGET TimeCurvesTest PROPERTY FOLDER "Tests")
	endif()
endif()