cmake_minimum_required(VERSION 3.8)

include_directories (
	"../src"
	"./"
)

function(add_test_helper test_name exec_name text_cpp)

	add_executable (${exec_name} ${text_cpp})
	target_link_libraries (
		${exec_name}
		readerlib
	)

	add_test (NAME ${test_name} COMMAND ${exec_name})

endfunction(add_test_helper)

add_test_helper ("READERLIB_EXPORT_XML"  "readerlib_test_export_xml"  "./export/xml_test.cpp")
add_test_helper ("READERLIB_EXPORT_CSV"  "readerlib_test_export_csv"  "./export/csv_test.cpp")
add_test_helper ("READERLIB_EXPORT_SVG"  "readerlib_test_export_svg"  "./export/svg_test.cpp")
add_test_helper ("READERLIB_EXPORT_DLOG" "readerlib_test_export_dlog" "./export/dlog_test.cpp")

add_test_helper ("READERLIB_EXPORT_XML_RESOLUTION_5"  "readerlib_test_export_xml_r5"  "./export/xml_r5_test.cpp")
add_test_helper ("READERLIB_EXPORT_CSV_RESOLUTION_5"  "readerlib_test_export_csv_r5"  "./export/csv_r5_test.cpp")
add_test_helper ("READERLIB_EXPORT_SVG_RESOLUTION_5"  "readerlib_test_export_svg_r5"  "./export/svg_r5_test.cpp")
add_test_helper ("READERLIB_EXPORT_DLOG_RESOLUTION_5" "readerlib_test_export_dlog_r5" "./export/dlog_r5_test.cpp")

add_test_helper ("READERLIB_READER_CSV"   "readerlib_test_reader_csv"   "./reader/csv_test.cpp")
add_test_helper ("READERLIB_READER_XML"   "readerlib_test_reader_xml"   "./reader/xml_test.cpp")

add_test_helper ("READERLIB_READER_CSV_RESOLUTION_5"   "readerlib_test_reader_csv_r5"   "./reader/csv_r5_test.cpp")
add_test_helper ("READERLIB_READER_XML_RESOLUTION_5"   "readerlib_test_reader_xml_r5"   "./reader/xml_r5_test.cpp")
