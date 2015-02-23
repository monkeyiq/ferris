NULL=
SUBDIRS = relativePaths timber

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in


bin_PROGRAMS = createfile createdir createea readea writeea  \
	readcontext writecontext                             \
	renamecontext removecontext

noinst_PROGRAMS = plaything

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@ \
	@CFLAGS@

createfile_SOURCES = createfile.cpp
createfile_LDADD   = -lpopt @LIBFERRIS_LA@ 
createfile_LDFLAGS = @GLIB_LIBS@

createdir_SOURCES = createdir.cpp
createdir_LDADD   = -lpopt @LIBFERRIS_LA@ 
createdir_LDFLAGS = @GLIB_LIBS@

createea_SOURCES = createea.cpp
createea_LDADD   = -lpopt @LIBFERRIS_LA@ 
createea_LDFLAGS = @GLIB_LIBS@

readea_SOURCES = readea.cpp
readea_LDADD   = -lpopt @LIBFERRIS_LA@ 
readea_LDFLAGS = @GLIB_LIBS@

writeea_SOURCES = writeea.cpp
writeea_LDADD   = -lpopt @LIBFERRIS_LA@ 
writeea_LDFLAGS = @GLIB_LIBS@

readcontext_SOURCES = readcontext.cpp
readcontext_LDADD   = -lpopt @LIBFERRIS_LA@ 
readcontext_LDFLAGS = @GLIB_LIBS@

writecontext_SOURCES = writecontext.cpp
writecontext_LDADD   = -lpopt @LIBFERRIS_LA@ 
writecontext_LDFLAGS = @GLIB_LIBS@

renamecontext_SOURCES = renamecontext.cpp
renamecontext_LDADD   = -lpopt @LIBFERRIS_LA@ 
renamecontext_LDFLAGS = @GLIB_LIBS@

removecontext_SOURCES = removecontext.cpp
removecontext_LDADD   = -lpopt @LIBFERRIS_LA@ 
removecontext_LDFLAGS = @GLIB_LIBS@

plaything_SOURCES = plaything.cpp
plaything_LDADD   = -lpopt @LIBFERRIS_LA@ 
plaything_LDFLAGS = @GLIB_LIBS@


