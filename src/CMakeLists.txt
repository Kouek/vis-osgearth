set(TARGET_NAME "scivis")

# Pack all glsl files into C-style symbols, avoid path-not-found problems
function(pack_glsl_to_c PREFIX_PATH)
	file(GLOB SHADER_SRCS "${PREFIX_PATH}/*.glsl")
	foreach(SHADER_SRC ${SHADER_SRCS})
		get_filename_component(FILE_NAME ${SHADER_SRC} NAME_WLE)
		set(NEW_FILE_PATH "${PREFIX_PATH}/generated/${FILE_NAME}.h")

		file(STRINGS ${SHADER_SRC} SHADER_SRC_CODES)
		file(
			WRITE
			${NEW_FILE_PATH}
			"static inline const char* ${FILE_NAME} = \"\\n\"\n"
		)
		foreach(SHADER_SRC_CODE ${SHADER_SRC_CODES})
			file(
				APPEND
				${NEW_FILE_PATH}
				"\"${SHADER_SRC_CODE}\\n\"\n"
			)
		endforeach()
		file(
			APPEND
			${NEW_FILE_PATH}
			";"
		)
	endforeach()
endfunction(pack_glsl_to_c)

pack_glsl_to_c("${CMAKE_CURRENT_LIST_DIR}/scalar_viser/shaders")

add_library(
	${TARGET_NAME}
	INTERFACE
)
target_include_directories(
	${TARGET_NAME}
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}
)
