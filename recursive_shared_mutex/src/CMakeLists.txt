set (RSMTX_SOURCES
        recursive_shared_mutex.cpp
        )

add_library (dts_rsmtx ${RSMTX_HEADERS} ${RSMTX_SOURCES})