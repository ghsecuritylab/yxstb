include_directories(
	${PROJECT_SOURCE_DIR}/EC2108_C27/platform/$ENV{PLATFORM}/include_priv
	)
	

aux_source_directory( decode  platform_spec_SRCS )
aux_source_directory( input  platform_spec_SRCS )
aux_source_directory( upgrade  platform_spec_SRCS )
###uncompress: removed by teddy at 2011-4-11 14:55:48
#aux_source_directory( uncompress  platform_spec_SRCS )	
aux_source_directory( pvrfile  platform_spec_SRCS )
aux_source_directory( mid_depend  platform_spec_SRCS )

if( ENABLE_Debug_Build )
	add_definitions( -DDEBUG_BUILD )
endif( ENABLE_Debug_Build )
	
if( ENABLE_Include_DLNA )
	add_definitions( -DINCLUDE_DLNA )
endif( ENABLE_Include_DLNA )	

if( ENABLE_IGMPV3 )
	add_definitions( -DENABLE_IGMPV3 )
endif( ENABLE_IGMPV3 )	

if( ENABLE_IPV6 )
	add_definitions( -DENABLE_IPV6 )
endif( ENABLE_IPV6 )	

if( ENABLE_Include_DVBS_Tuner )
	add_definitions( -DINCLUDE_DVBS_TUNER )
	include_directories( ../../ipstb/src/porting/tuner/include )
endif( ENABLE_Include_DVBS_Tuner )

aux_source_directory( mid  platform_spec_SRCS )

add_library( platform_depends STATIC 
	${platform_spec_SRCS}
	)

#MESSAGE("${CMAKE_CURRENT_SOURCE_DIR}: platform_spec_SRCS=${platform_spec_SRCS}" )

