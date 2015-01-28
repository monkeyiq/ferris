
EXTRA_DIST = 

if HAVE_GCJLUCENE
GCJLUCENEDIR = lucene
endif

if HAVE_DTL
DTLDIR = odbc
endif

if HAVE_XAPIAN
XAPIANDIR = xapian
endif

if BUILD_PGTSEARCH_FTXIDX
PGTSEARCHSRCS = tsearch2
endif

if HAVE_CLUCENE
CLUCENEDIR = clucene
endif

if HAVE_STRIGI
STRIGIDIR = strigi
endif

if HAVE_QTSQL
QTSQLDIR = qtsql
endif

SUBDIRS = $(GCJLUCENEDIR) $(DTLDIR) $(XAPIANDIR) $(PGTSEARCHSRCS) \
	yahoo external beagle null uniqsorted \
	$(CLUCENEDIR) $(STRIGIDIR) $(QTSQLDIR)




