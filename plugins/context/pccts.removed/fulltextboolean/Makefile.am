
DLG_FILE = parser.dlg
AFLAGS = -CC -gt  $(AOTHER)
#AFLAGS = -CC -gt -mrhoist off $(AOTHER)
DFLAGS = -C2 -i -CC $(DOTHER) -cl DLGLexerFulltextBoolean
GRM = fulltextboolean.g
SCAN = DLGLexerFulltextBoolean
ANTLR = antlr
DLG = dlg
TOKENS=tokens.h
ANTLR_SPAWN = fulltextboolean.cpp \
		FulltextbooleanParser.cpp \
		FulltextbooleanParser.h \
		$(DLG_FILE) $(TOKENS) 
DLG_SPAWN = DLGLexerFulltextBoolean.cpp DLGLexerFulltextBoolean.h 



BUILT_SOURCES = AParser.cpp DLexerBase.cpp ASTBase.cpp PCCTSAST.cpp ATokenBuffer.cpp tokens.h \
	parser.dlg FulltextbooleanParser.h FulltextbooleanParser.cpp fulltextboolean.cpp \
	DLGLexerFulltextBoolean.cpp DLGLexerFulltextBoolean.h

#	$(DLG_SPAWN)

EXTRA_DIST = Common.hh
#noinst_PROGRAMS = fulltextboolean 


#fulltextboolean_SOURCES = fulltextboolean.cpp                    \
#	FulltextbooleanParser.cpp FulltextbooleanParser.h        \
#	DLGLexerFulltextBoolean.h DLGLexerFulltextBoolean.cpp                                    \
#	AParser.cpp DLexerBase.cpp ASTBase.cpp PCCTSAST.cpp ATokenBuffer.cpp 

#fulltextboolean_LDADD = -lpopt @LIBFERRIS_LA@ 
#fulltextboolean_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@


FulltextbooleanParser.o : $(TOKENS) DLGLexerFulltextBoolean.h FulltextbooleanParser.cpp FulltextbooleanParser.h
	$(CXXCOMPILE) -c $(CFLAGS) -o FulltextbooleanParser.o FulltextbooleanParser.cpp


$(DLG_SPAWN) : $(DLG_FILE)
	$(DLG) $(DFLAGS) $(DLG_FILE)

#	mkdir -p .deps
#	echo '# dummy' >|.deps/DLGLexerFulltextBoolean.Po
#	echo '# dummy' >|.deps/DLGLexerFulltextBoolean.Plo



DLGLexerFulltextBoolean.o : DLGLexerFulltextBoolean.cpp $(TOKENS)
	$(CXXCOMPILE) -c $(CFLAGS) -o DLGLexerFulltextBoolean.o DLGLexerFulltextBoolean.cpp

$(ANTLR_SPAWN) : $(GRM)
	$(ANTLR) $(AFLAGS) $(GRM)

AParser.cpp : $(ANTLR_H)/AParser.cpp
	cp $(ANTLR_H)/AParser.cpp .

ATokenBuffer.cpp : $(ANTLR_H)/ATokenBuffer.cpp
	cp $(ANTLR_H)/ATokenBuffer.cpp .

DLexerBase.cpp : $(ANTLR_H)/DLexerBase.cpp
	cp $(ANTLR_H)/DLexerBase.cpp .

ASTBase.cpp : $(ANTLR_H)/ASTBase.cpp
	cp $(ANTLR_H)/ASTBase.cpp .

PCCTSAST.cpp : $(ANTLR_H)/PCCTSAST.cpp
	cp $(ANTLR_H)/PCCTSAST.cpp .

#PCCTSAST.o : $(ANTLR_H)/PCCTSAST.cpp
#	$(CXXCOMPILE) -c $(CFLAGS) -o PCCTSAST.o $(ANTLR_H)/PCCTSAST.cpp

$(DLG_SPAWN) : $(DLG_FILE)
	$(DLG) $(DFLAGS) $(DLG_FILE)



###############################################################################
###############################################################################
###############################################################################


plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES = libfulltextboolean.la libfulltextboolean_factory.la
libfulltextboolean_factory_la_SOURCES = libfulltextboolean_factory.cpp
libfulltextboolean_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libfulltextboolean_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libfulltextboolean_la_SOURCES         = libfulltextboolean.cpp                \
	fulltextboolean.g                                                     \
	FulltextbooleanParser.cpp FulltextbooleanParser.h                     \
	DLGLexerFulltextBoolean.cpp DLGLexerFulltextBoolean.h                                                 \
	AParser.cpp DLexerBase.cpp ASTBase.cpp PCCTSAST.cpp ATokenBuffer.cpp  \
	fulltextboolean.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(srcdir)/../..                               \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-DPCCTSLEXERFILENAME="\"DLGLexerFulltextBoolean.h\"" \
	-DPCCTSLEXERCLASSNAME="DLGLexerFulltextBoolean"      \
	-DPCCTSLEXERSUBCLASSNAME="MyDLGLexerFulltextBoolean" \
	@PCCTS_CFLAGS@                                  \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

libfulltextboolean_la_LDFLAGS = @GLIB_LIBS@ 
