if HAVE_REDLAND

NULL=

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST=$(srcdir)/*.xml $(srcdir)/*.rdf
noinst_PROGRAMS = rdfexample1 rdfexample2 rdfexample3 rdfexample4 \
	makeCyclicRDF rdfassert \
	rdfsimplequery

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@                                   \
	@REDLAND_CFLAGS@                                \
	@CFLAGS@

rdfexample1_SOURCES = rdfexample1.cpp
rdfexample1_LDADD   = -lpopt @LIBFERRIS_LA@ 
rdfexample1_LDFLAGS = @GLIB_LIBS@ @REDLAND_LIBS@

rdfexample2_SOURCES = rdfexample2.cpp
rdfexample2_LDADD   = -lpopt @LIBFERRIS_LA@ 
rdfexample2_LDFLAGS = @GLIB_LIBS@ @REDLAND_LIBS@

rdfexample3_SOURCES = rdfexample3.cpp
rdfexample3_LDADD   = -lpopt @LIBFERRIS_LA@ 
rdfexample3_LDFLAGS = @GLIB_LIBS@ @REDLAND_LIBS@

rdfexample4_SOURCES = rdfexample4.cpp
rdfexample4_LDADD   = -lpopt @LIBFERRIS_LA@ 
rdfexample4_LDFLAGS = @GLIB_LIBS@ @REDLAND_LIBS@

makeCyclicRDF_SOURCES = makeCyclicRDF.cpp
makeCyclicRDF_LDADD   = -lpopt @LIBFERRIS_LA@ 
makeCyclicRDF_LDFLAGS = @GLIB_LIBS@ @REDLAND_LIBS@

rdfassert_SOURCES = rdfassert.cpp
rdfassert_LDADD   = -lpopt @LIBFERRIS_LA@ 
rdfassert_LDFLAGS = @GLIB_LIBS@ @REDLAND_LIBS@

rdfsimplequery_SOURCES = rdfsimplequery.cpp
rdfsimplequery_LDADD   = -lpopt @LIBFERRIS_LA@ 
rdfsimplequery_LDFLAGS = @GLIB_LIBS@ @REDLAND_LIBS@

endif


