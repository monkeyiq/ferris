
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = ferris-rdf-smush-new-url ferris-rdf-smush-union-urls ferris-myrdf-query \
	ferris-rdf-detach-by-regex ferris-myrdf-compact ferris-myrdf-inference

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@SOPRANO_CFLAGS@                                \
	@CFLAGS@

ferris_rdf_smush_new_url_SOURCES = ferris-rdf-smush-new-url.cpp
ferris_rdf_smush_new_url_LDADD = -lpopt @LIBFERRIS_LA@ 
ferris_rdf_smush_new_url_LDFLAGS = @SOPRANO_LIBS@

ferris_rdf_smush_union_urls_SOURCES = ferris-rdf-smush-union-urls.cpp
ferris_rdf_smush_union_urls_LDADD = -lpopt @LIBFERRIS_LA@ 
ferris_rdf_smush_union_urls_LDFLAGS = @SOPRANO_LIBS@

ferris_myrdf_query_SOURCES = ferris-myrdf-query.cpp
ferris_myrdf_query_LDADD = -lpopt @LIBFERRIS_LA@ 
ferris_myrdf_query_LDFLAGS = @SOPRANO_LIBS@

ferris_rdf_detach_by_regex_SOURCES = ferris-rdf-detach-by-regex.cpp
ferris_rdf_detach_by_regex_LDADD = -lpopt @LIBFERRIS_LA@ 
ferris_rdf_detach_by_regex_LDFLAGS = @SOPRANO_LIBS@

ferris_myrdf_compact_SOURCES = ferris-myrdf-compact.cpp
ferris_myrdf_compact_LDADD = -lpopt @LIBFERRIS_LA@ 
ferris_myrdf_compact_LDFLAGS = @SOPRANO_LIBS@

ferris_myrdf_inference_SOURCES = ferris-myrdf-inference.cpp
ferris_myrdf_inference_LDADD = -lpopt @LIBFERRIS_LA@ 
ferris_myrdf_inference_LDFLAGS = @SOPRANO_LIBS@

