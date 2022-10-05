if (Qt${QT_VERSION_MAJOR}HttpServer_FOUND)

	target_compile_definitions(Remote PRIVATE QT_HTTPSERVER)

	set(HTTP_INSTALL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Remote/web/dist")
	add_custom_target(RemoteClientCode
		COMMAND npm run installbuild
		WORKING_DIRECTORY ${HTTP_INSTALL_SRC_DIR})
	if (openiA_USE_IDE_FOLDERS)
		SET_PROPERTY(TARGET RemoteClientCode PROPERTY FOLDER "Modules")
	endif()
	add_dependencies(Remote RemoteClientCode)
	foreach (cfg ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER "${cfg}" CFG)
		set(HTTP_DST_SUBDIR "RemoteClient")
		set(HTTP_INSTALL_DST_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG}}/${HTTP_DST_SUBDIR}")
		if (NOT EXISTS "${HTTP_INSTALL_DST_DIR}")
			file(COPY "${HTTP_INSTALL_SRC_DIR}/" DESTINATION "${HTTP_INSTALL_DST_DIR}")
		endif()
	endforeach()
	install(DIRECTORY "${HTTP_INSTALL_SRC_DIR}/" DESTINATION ${HTTP_DST_SUBDIR})
else()
	message(WARNING "Qt HttpServer is not available! You can still run the Remote Render Server, but you will have to serve the 'RemoteClient' folder separately, e.g. via webpack!")
endif()
