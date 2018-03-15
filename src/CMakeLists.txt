cmake_minimum_required(VERSION 3.8)

set (READERLIB_SOURCE
	rlib/common/reader.cpp
	rlib/common/cached_reader.cpp
	rlib/common/statistic_reader.cpp
	rlib/common/synthetic_reader.cpp
	rlib/common/exporter.cpp
	rlib/common/sample.cpp
	rlib/common/sensor.cpp

	rlib/android/meta_reader.cpp
	rlib/android/meta.cpp

	rlib/powerscale/psi.cpp
	rlib/powerscale/psi_reader.cpp

	rlib/keysight/dlog.cpp
	rlib/keysight/dlog_reader.cpp
	rlib/keysight/dlog_exporter.cpp

	rlib/grim/grim_data.cpp
	rlib/grim/grim_reader.cpp

	rlib/csv/csv_reader.cpp
	rlib/csv/csv_exporter.cpp

	rlib/svg/svg_exporter.cpp

	rlib/xml/xml_exporter.cpp
	rlib/xml/xml_reader.cpp

	rlib/remote/reader.cpp
)

include_directories("./" ${Boost_INCLUDE_DIRS})

include_directories(... )

add_library (${LIBNAME} SHARED ${READERLIB_SOURCE})
target_include_directories(${LIBNAME} PUBLIC "./")
target_link_libraries(${LIBNAME} ${Boost_LIBRARIES})

add_library (${STATIC_LIBNAME} STATIC ${READERLIB_SOURCE})
target_include_directories(${STATIC_LIBNAME} PUBLIC "./")
target_link_libraries(${STATIC_LIBNAME} ${Boost_LIBRARIES})
