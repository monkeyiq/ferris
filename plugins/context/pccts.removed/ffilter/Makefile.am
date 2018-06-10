
DLG_FILE = parser.dlg
AFLAGS = -CC -gt  $(AOTHER)
#AFLAGS = -CC -gt -mrhoist off $(AOTHER)
DFLAGS = -C2 -i -CC $(DOTHER)
GRM = ffilter.g
SCAN = DLGLexer
ANTLR = antlr
DLG = dlg
TOKENS=tokens.h
ANTLR_SPAWN = ffilter.cpp FFilterParser.cpp \
              FFilterParser.h $(DLG_FILE) $(TOKENS) 
DLG_SPAWN = $(SCAN).cpp $(SCAN).h


BUILT_SOURCES = AParser.cpp DLexerBase.cpp ASTBase.cpp PCCTSAST.cpp ATokenBuffer.cpp tokens.h \
	parser.dlg FFilterParser.h FFilterParser.cpp ffilter.cpp \
	$(DLG_SPAWN)

EXTRA_DIST = Common.hh
#noinst_PROGRAMS = ffilter 


#ffilter_SOURCES = ffilter.cpp                    \
#	FFilterParser.cpp FFilterParser.h        \
#	$(SCAN).h $(SCAN).cpp                    \
#	AParser.cpp DLexerBase.cpp ASTBase.cpp PCCTSAST.cpp ATokenBuffer.cpp



#ffilter_LDADD = -lpopt @LIBFERRIS_LA@ 
#ffilter_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@


FFilterParser.o : $(TOKENS) $(SCAN).h FFilterParser.cpp FFilterParser.h
	$(CXXCOMPILE) -c $(CFLAGS) -o FFilterParser.o FFilterParser.cpp


$(DLG_SPAWN) : $(DLG_FILE)
	$(DLG) $(DFLAGS) $(DLG_FILE)

$(SCAN).o : $(SCAN).cpp $(TOKENS)
	$(CXXCOMPILE) -c $(CFLAGS) -o $(SCAN).o $(SCAN).cpp

$(ANTLR_SPAWN) : $(GRM)
	$(ANTLR) $(AFLAGS) $(GRM)

AParser.cpp:
	cp $(ANTLR_H)/AParser.cpp .

ATokenBuffer.cpp:
	cp $(ANTLR_H)/ATokenBuffer.cpp .

DLexerBase.cpp:
	cp $(ANTLR_H)/DLexerBase.cpp .

ASTBase.cpp:
	cp $(ANTLR_H)/ASTBase.cpp .

PCCTSAST.cpp:
	cp $(ANTLR_H)/PCCTSAST.cpp .




###############################################################################
###############################################################################
###############################################################################


plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES = libffilter.la libffilter_factory.la
libffilter_factory_la_SOURCES = libffilter_factory.cpp
libffilter_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libffilter_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libffilter_la_SOURCES         = libffilter.cpp                \
	ffilter.g                                             \
	FFilterParser.cpp FFilterParser.h                     \
	$(SCAN).cpp $(SCAN).h                                 \
	AParser.cpp DLexerBase.cpp ASTBase.cpp PCCTSAST.cpp ATokenBuffer.cpp  \
	ffilter.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib  						\
	-I$(srcdir)/..                                  \
	-I$(srcdir)/../..                               \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@PCCTS_CFLAGS@                                  \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

libffilter_la_LDFLAGS = @GLIB_LIBS@
