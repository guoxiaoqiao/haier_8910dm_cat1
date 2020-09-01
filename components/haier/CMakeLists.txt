# Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
# All rights reserved.
#
# This software is supplied "AS IS" without any warranties.
# RDA assumes no responsibility or liability for the use of the software,
# conveys no license or title under any patent, copyright, or mask work
# right to the product. RDA reserves the right to make changes in the
# software without notification.  RDA also make no representation or
# warranty that such application will be suitable for the specified use
# without further testing or modification.

configure_file(include/haier_config.h.in ${out_inc_dir}/haier_config.h)

set(target haier)
add_app_libraries($<TARGET_FILE:${target}>)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
#target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_ZKDB)
target_include_directories(${target} PUBLIC include)
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${target} PRIVATE kernel driver calclib ats cfw net nanopb fs service diag)

aux_source_directory(./src DIR_SRCS)
message("haier app ${DIR_SRCS}")
target_sources(${target} PRIVATE
	${DIR_SRCS}
)

include(core.cmake)

relative_glob(srcs include/*.h src/*.c src/*.h)
beautify_c_code(${target} ${srcs})
