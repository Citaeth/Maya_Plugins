cmake_minimum_required(VERSION 3.15)

PROJECT (CacheSequencer)

SET(MAYA_BASE_LOCATION "C:/Program Files/Autodesk")
SET(MAYA_VERSIONS
    "2025")

SET(CacheSequencerFiles
    cache_sequencer.h
    cache_sequencer.cpp
)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

######################################################################################## Plugins, Foreach in case of more than on version to support
FOREACH(MAYA_VERSION ${MAYA_VERSIONS})

    SET(MAYA_LOCATION "${MAYA_BASE_LOCATION}/Maya${MAYA_VERSION}")

    include_directories(${MAYA_LOCATION}/include)
    link_directories(${MAYA_LOCATION}/lib)

    add_library(CacheSequencer_${MAYA_VERSION} SHARED ${CacheSequencerFiles})

    target_link_libraries(CacheSequencer_${MAYA_VERSION} OpenMaya Foundation)

    set_target_properties(CacheSequencer_${MAYA_VERSION} PROPERTIES
        OUTPUT_NAME "CacheSequencer"
        PREFIX ""
        SUFFIX ".mll"
    )

	INSTALL(TARGETS CacheSequencer_${MAYA_VERSION}
    RUNTIME DESTINATION "${MAYA_LOCATION}/bin/plug-ins"
)
ENDFOREACH()