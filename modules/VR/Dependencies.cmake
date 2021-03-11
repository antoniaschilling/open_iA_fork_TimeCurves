SET (DEPENDENCIES_LIBRARIES
	${OPENVR_LIBRARY}
	iAguibase
	iAobjectvis
	iAqthelper
)
IF ( (VTK_MAJOR_VERSION LESS 9 AND vtkRenderingOpenVR_LOADED) OR
	 (VTK_MAJOR_VERSION GREATER 8 AND RenderingOpenVR IN_LIST VTK_AVAILABLE_COMPONENTS) )
	IF (VTK_MAJOR_VERSION LESS 9)
		FIND_PACKAGE(OpenVR)
		# HINTS ${OPENVR_PATH} # leads to OpenVR not being found at all...?
	ENDIF()
ELSE()
	MESSAGE(STATUS "OpenVR SDK: Not found.")
ENDIF()

IF (VTK_MAJOR_VERSION LESS 9)
	SET( DEPENDENCIES_CMAKE
		vtkRenderingOpenVR_LOADED
	)
ELSE()
	if (NOT RenderingOpenVR IN_LIST VTK_AVAILABLE_COMPONENTS)
		MESSAGE(WARNING "OpenVR not available in VTK 9! Please enable RenderingOpenVR module!")
	endif()
ENDIF()

SET( DEPENDENCIES_INCLUDE_DIRS
	${OPENVR_INCLUDE_DIR}
)
SET( DEPENDENCIES_COMPILE_DEFINITIONS
	OPENVR_VERSION_MAJOR=${OPENVR_VERSION_MAJOR} OPENVR_VERSION_MINOR=${OPENVR_VERSION_MINOR} OPENVR_VERSION_BUILD=${OPENVR_VERSION_BUILD}
)
