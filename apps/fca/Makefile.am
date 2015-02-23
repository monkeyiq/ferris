
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

if HAVE_GTKEXTRA2
GTKEXTRA_APPS = gf-create-fca-scale-numeric
endif

bin_PROGRAMS = ferris-create-fca-tree ferris-lattice-from-cfi        \
	ferris-create-fca-scale-urls ferris-create-fca-scale-numeric \
	ferris-create-fca-scale-numeric-ordinal                      \
	$(GTKEXTRA_APPS)                                             \
	ferris-create-fca-scale-splice-in-ffitler                    \
	ferris-create-fca-scale-nominal                              \
	ferris-export-concept-lattice-to-toscanaj                    \
	ferris-export-formal-context-to-burmeister


INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@                                   \
	@LIBPQXX_CFLAGS@                                \
	@SOPRANO_CFLAGS@                                \
	@LIBGISTMIQ_CFLAGS@                             \
	@CFLAGS@

noinst_LTLIBRARIES = libferrisfcascaling.la
libferrisfcascaling_la_SOURCES = libferrisfcascaling.cpp libferrisfcascaling.hh


ferris_create_fca_tree_SOURCES = ferris-create-fca-tree.cpp
ferris_create_fca_tree_LDADD = -lpopt @LIBFERRIS_LA@
ferris_create_fca_tree_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @LIBPQXX_LIBS@ libferrisfcascaling.la

ferris_lattice_from_cfi_SOURCES = ferris-lattice-from-cfi.cpp
ferris_lattice_from_cfi_LDADD = -lpopt @LIBFERRIS_LA@
ferris_lattice_from_cfi_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @LIBPQXX_LIBS@ @LIBGISTMIQ_LIBS@

ferris_create_fca_scale_urls_SOURCES = ferris-create-fca-scale-urls.cpp
ferris_create_fca_scale_urls_LDADD = -lpopt @LIBFERRIS_LA@
ferris_create_fca_scale_urls_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @LIBPQXX_LIBS@ @SOPRANO_LIBS@ libferrisfcascaling.la

ferris_create_fca_scale_numeric_SOURCES = ferris-create-fca-scale-numeric.cpp
ferris_create_fca_scale_numeric_LDADD = -lpopt @LIBFERRIS_LA@
ferris_create_fca_scale_numeric_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @LIBPQXX_LIBS@ libferrisfcascaling.la

ferris_create_fca_scale_numeric_ordinal_SOURCES = ferris-create-fca-scale-numeric-ordinal.cpp
ferris_create_fca_scale_numeric_ordinal_LDADD = -lpopt @LIBFERRIS_LA@
ferris_create_fca_scale_numeric_ordinal_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @LIBPQXX_LIBS@ libferrisfcascaling.la

ferris_create_fca_scale_splice_in_ffitler_SOURCES = ferris-create-fca-scale-splice-in-ffitler.cpp
ferris_create_fca_scale_splice_in_ffitler_LDADD = -lpopt @LIBFERRIS_LA@
ferris_create_fca_scale_splice_in_ffitler_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @LIBPQXX_LIBS@ libferrisfcascaling.la


gf_create_fca_scale_numeric_SOURCES = gf-create-fca-scale-numeric.cpp
gf_create_fca_scale_numeric_CXXFLAGS = $(INCLUDES) @GTK_CFLAGS@ @GTKEXTRA2_CFLAGS@
gf_create_fca_scale_numeric_LDADD = -lpopt @LIBFERRIS_LA@ @LIBFERRISUI_LA@
gf_create_fca_scale_numeric_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @LIBPQXX_LIBS@ \
	@GTK_LIBS@ @GTKEXTRA2_LIBS@ libferrisfcascaling.la


ferris_create_fca_scale_nominal_SOURCES = ferris-create-fca-scale-nominal.cpp
ferris_create_fca_scale_nominal_LDADD = -lpopt @LIBFERRIS_LA@
ferris_create_fca_scale_nominal_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @LIBPQXX_LIBS@ libferrisfcascaling.la


ferris_export_concept_lattice_to_toscanaj_SOURCES = ferris-export-concept-lattice-to-toscanaj.cpp
ferris_export_concept_lattice_to_toscanaj_LDADD = -lpopt @LIBFERRIS_LA@
ferris_export_concept_lattice_to_toscanaj_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @LIBPQXX_LIBS@ 

ferris_export_formal_context_to_burmeister_SOURCES = ferris-export-formal-context-to-burmeister.cpp
ferris_export_formal_context_to_burmeister_LDADD = -lpopt @LIBFERRIS_LA@
ferris_export_formal_context_to_burmeister_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @LIBPQXX_LIBS@ 




