set( PROJECT "bd9xx" )
set( TARGET_NAME snark_navigation_trimble_${PROJECT} )
file( GLOB includes *.h)
file( GLOB sources *.cpp )
source_group( ${TARGET_NAME} FILES ${sources} ${includes} )
add_library( ${TARGET_NAME} ${sources} ${includes} )
set_target_properties( ${TARGET_NAME} PROPERTIES ${snark_LIBRARY_PROPERTIES} )
target_link_libraries( ${TARGET_NAME} snark_math ${snark_ALL_EXTERNAL_LIBRARIES} )
install( FILES ${includes} DESTINATION ${snark_INSTALL_INCLUDE_DIR}/navigation/trimble/${PROJECT} )
add_subdirectory( packets )