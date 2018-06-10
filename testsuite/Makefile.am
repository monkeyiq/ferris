NULL=
SUBDIRS = \
	config                      \
	ferris.test                 \
	sampledata expected_output  

EXTRA_DIST = ut_outofproc_notification_simpleclient_output.txt \
	ut_outofproc_notification_simpleclient_output_3.txt    \
	ut_outofproc_notification_med_output.txt \
	pythontest.py

noinst_PROGRAMS = \
	ut_index_compression          \
	ut_manybasetoone_viewcontext  \
	ut_fca                        \
	ut_inheritea                  \
	ut_ParseRelativeTimeString    \
	ut_ffilter                    \
	ut_ferrisdom                  \
	ut_xslt                       \
	ut_medallion                  \
	ut_cache                      \
	ut_outofproc_notification     \
	ut_config                     \
	ut_fuzzymedallion	      \
	ut_memory_manager

INCLUDES = @GLIB_CFLAGS@ @CFLAGS@

ut_memory_manager_SOURCES = ut_memory_manager.cpp
ut_memory_manager_LDADD   = -lpopt @LIBFERRIS_LA@ 
ut_memory_manager_LDFLAGS = $(GLIB_LIBS) 


ut_index_compression_SOURCES = ut_index_compression.cpp
ut_index_compression_LDADD   = -lpopt @LIBFERRIS_LA@ 
ut_index_compression_LDFLAGS = $(GLIB_LIBS) 

ut_manybasetoone_viewcontext_SOURCES = ut_manybasetoone_viewcontext.cpp
ut_manybasetoone_viewcontext_LDADD   = -lpopt @LIBFERRIS_LA@ 
ut_manybasetoone_viewcontext_LDFLAGS = $(GLIB_LIBS) 

ut_fca_SOURCES = ut_fca.cpp
ut_fca_LDADD   = -lpopt @LIBFERRIS_LA@ 
ut_fca_LDFLAGS = $(GLIB_LIBS) 

ut_inheritea_SOURCES = ut_inheritea.cpp
ut_inheritea_LDADD   = -lpopt @LIBFERRIS_LA@ 
ut_inheritea_LDFLAGS = $(GLIB_LIBS) 

ut_ParseRelativeTimeString_SOURCES = ut_ParseRelativeTimeString.cpp
ut_ParseRelativeTimeString_LDADD   = -lpopt @LIBFERRIS_LA@ 
ut_ParseRelativeTimeString_LDFLAGS = $(GLIB_LIBS) 

ut_ffilter_SOURCES = ut_ffilter.cpp
ut_ffilter_LDADD   = -lpopt @LIBFERRIS_LA@ 
ut_ffilter_LDFLAGS = $(GLIB_LIBS) 

ut_ferrisdom_SOURCES = ut_ferrisdom.cpp
ut_ferrisdom_LDADD   = -lpopt @LIBFERRIS_LA@ 
ut_ferrisdom_LDFLAGS = $(GLIB_LIBS) 

ut_xslt_SOURCES = ut_xslt.cpp
ut_xslt_LDADD   = -lpopt @LIBFERRIS_LA@ 
ut_xslt_LDFLAGS = $(GLIB_LIBS) 

ut_medallion_SOURCES = ut_medallion.cpp
ut_medallion_LDADD   = -lpopt @LIBFERRIS_LA@ 
ut_medallion_LDFLAGS = $(GLIB_LIBS) 

ut_cache_SOURCES = ut_cache.cpp
ut_cache_LDADD   = -lpopt @LIBFERRIS_LA@ 
ut_cache_LDFLAGS = $(GLIB_LIBS) 

ut_outofproc_notification_SOURCES = ut_outofproc_notification.cpp
ut_outofproc_notification_LDADD   = -lpopt @LIBFERRIS_LA@ 
ut_outofproc_notification_LDFLAGS = $(GLIB_LIBS) 

ut_config_SOURCES = ut_config.cpp
ut_config_LDADD   = -lpopt @LIBFERRIS_LA@ 
ut_config_LDFLAGS = $(GLIB_LIBS) 

ut_fuzzymedallion_SOURCES = ut_fuzzymedallion.cpp
ut_fuzzymedallion_LDADD   = -lpopt @LIBFERRIS_LA@ 
ut_fuzzymedallion_LDFLAGS = $(GLIB_LIBS) 
