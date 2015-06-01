
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_SCRIPTS = ferris-index-remove ferris-eaindex-postgresql-add-column ferris-eaindex-postgresql-add-fulltext-index-on-column ferris-eaindex-update-existing-attributes
SHELL_FILES = $(srcdir)/*.sh 
EXTRA_DIST = $(srcdir)/*.xsl $(SHELL_FILES) $(bin_SCRIPTS)


bin_PROGRAMS =  findexadd feaindexadd     \
		findexquery feaindexquery \
		findexcompact feaindexcompact \
		findexremove feaindexremove \
		ferris-search \
		feaindex-attach-fulltext-index \
		ferris-find \
		findex-remove-old-instances feaindex-remove-old-instances \
		feaindex-federation-add-url-substitution-regex-for-index \
		feaindex-federation-add-index

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

findexadd_SOURCES = findexadd.cpp
findexadd_LDADD = -lpopt @LIBFERRIS_LA@ 
findexadd_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

feaindexadd_SOURCES = feaindexadd.cpp
feaindexadd_LDADD = -lpopt @LIBFERRIS_LA@ 
feaindexadd_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

findexquery_SOURCES = findexquery.cpp
findexquery_LDADD = -lpopt @LIBFERRIS_LA@  @FTXIDXCUSTOMFERRIS_LA@ @LIBFERRISMG_LA@
findexquery_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

feaindexquery_SOURCES = feaindexquery.cpp
feaindexquery_LDADD = -lpopt @LIBFERRIS_LA@ @EAIDXCUSTOMFERRIS_LA@ @EAIDXCUSTOMFERRISDB4TREE_LA@ @LIBFERRISMG_LA@
feaindexquery_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

findexcompact_SOURCES = findexcompact.cpp
findexcompact_LDADD = -lpopt @LIBFERRIS_LA@ @FTXIDXCUSTOMFERRIS_LA@ @LIBFERRISMG_LA@
findexcompact_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

feaindexcompact_SOURCES = feaindexcompact.cpp
feaindexcompact_LDADD = -lpopt @LIBFERRIS_LA@  @EAIDXCUSTOMFERRIS_LA@ @EAIDXCUSTOMFERRISDB4TREE_LA@ @LIBFERRISMG_LA@
feaindexcompact_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

findexremove_SOURCES = findexremove.cpp
findexremove_LDADD = -lpopt @LIBFERRIS_LA@ @FTXIDXCUSTOMFERRIS_LA@ @LIBFERRISMG_LA@
findexremove_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

feaindexremove_SOURCES = feaindexremove.cpp
feaindexremove_LDADD = -lpopt @LIBFERRIS_LA@  @EAIDXCUSTOMFERRIS_LA@ @EAIDXCUSTOMFERRISDB4TREE_LA@ @LIBFERRISMG_LA@
feaindexremove_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

ferris_search_SOURCES = ferris-search.cpp
ferris_search_LDADD = -lpopt @LIBFERRIS_LA@ 
ferris_search_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

feaindex_attach_fulltext_index_SOURCES = feaindex-attach-fulltext-index.cpp
feaindex_attach_fulltext_index_LDADD = -lpopt @LIBFERRIS_LA@ 
feaindex_attach_fulltext_index_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

ferris_find_SOURCES = ferris-find.cpp
ferris_find_LDADD = -lpopt @LIBFERRIS_LA@ @BOOST_LIBS@ 
ferris_find_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ 

findex_remove_old_instances_SOURCES = findex-remove-old-instances.cpp
findex_remove_old_instances_LDADD = -lpopt @LIBFERRIS_LA@ @BOOST_LIBS@ 
findex_remove_old_instances_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ 

feaindex_remove_old_instances_SOURCES = feaindex-remove-old-instances.cpp
feaindex_remove_old_instances_LDADD = -lpopt @LIBFERRIS_LA@ @BOOST_LIBS@ 
feaindex_remove_old_instances_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ 

feaindex_federation_add_index_SOURCES = feaindex-federation-add-index.cpp DirectIndexConfigAccess.hh
feaindex_federation_add_index_LDADD = -lpopt @LIBFERRIS_LA@ @BOOST_LIBS@
feaindex_federation_add_index_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ 

feaindex_federation_add_url_substitution_regex_for_index_SOURCES = feaindex-federation-add-url-substitution-regex-for-index.cpp DirectIndexConfigAccess.hh
feaindex_federation_add_url_substitution_regex_for_index_LDADD = -lpopt @LIBFERRIS_LA@ @BOOST_LIBS@ 
feaindex_federation_add_url_substitution_regex_for_index_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ 

