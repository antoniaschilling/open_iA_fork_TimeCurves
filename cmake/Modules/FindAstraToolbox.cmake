# Try to find ASTRA Toolbox
#
# The following variables are set:
#    - ASTRA_TOOLBOX_FOUND           true if ASTRA Toolbox found, false otherwise
#    - ASTRA_TOOLBOX_INCLUDE_DIRS    the include directory
#    - ASTRA_TOOLBOX_LIBRARIES       the libraries to link to

find_package( PackageHandleStandardArgs )

set(ASTRA_TOOLBOX_DIR
	"${ASTRA_TOOLBOX_DIR}"
	CACHE
	PATH
	"Path to search for the ASTRA Toolbox"
)
SET (ASTRA_TOOLBOX_LIBRARIES "ASTRA_TOOLBOX_LIBRARIES-NOTFOUND")
SET (ASTRA_TOOLBOX_INCLUDE_DIRS "ASTRA_TOOLBOX_INCLUDE_DIRS-NOTFOUND")

# ToDo: Fix to include correct library for both Debug and Release build!
find_library(ASTRA_TOOLBOX_LIBRARIES AstraCuda64D
    PATHS ${ASTRA_TOOLBOX_DIR}/bin/x64/Release_CUDA
    PATHS ${ASTRA_TOOLBOX_DIR}/bin/x64/Debug_CUDA
)

find_path(ASTRA_INCLUDE_DIR astra/Algorithm.h
	PATHS ${ASTRA_TOOLBOX_DIR}/include
)

find_path(BOOST_INCLUDE_DIR boost/shared_ptr.hpp
	PATHS ${ASTRA_TOOLBOX_DIR}/lib/include)

SET (ASTRA_TOOLBOX_INCLUDE_DIRS ${ASTRA_INCLUDE_DIR} ${BOOST_INCLUDE_DIR})

IF (ASTRA_TOOLBOX_LIBRARIES AND ASTRA_TOOLBOX_INCLUDE_DIRS)
	SET (ASTRA_TOOLBOX_FOUND TRUE)
ELSE()
	SET (ASTRA_TOOLBOX_FOUND FALSE)
ENDIF ()

IF (AstraToolbox_FIND_REQUIRED AND NOT ASTRA_TOOLBOX_FOUND)
	MESSAGE(SEND_ERROR "Astra Toolbox not found, but it is specified to be required! Please set ASTRA_TOOLBOX_DIR!")
ENDIF()
