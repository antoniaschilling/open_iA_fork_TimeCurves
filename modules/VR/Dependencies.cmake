
FIND_PACKAGE(OpenVR REQUIRED)

MESSAGE(STATUS "OpenVR: ${OPENVR_ROOT_DIR}")

SET( DEPENDENCIES_CMAKE
	vtkRenderingOpenVR_LOADED
)

SET( DEPENDENCIES_LIBRARIES
	${OPENVR_LIBRARY}
)