



bin_PROGRAMS = mbox

DLG_FILE = parser.dlg
AFLAGS = -CC -gt  -ck 10 -e2 -k 3 -gl -mrhoist off $(AOTHER) # pp 145.
DFLAGS = -C2 -i -CC $(DOTHER)
GRM = mbox.g
SCAN = DLGLexer
ANTLR = antlr
DLG = dlg
ANTLR_SPAWN = mbox.cpp MBoxParser.cpp \
              MBoxParser.h $(DLG_FILE) $(TOKENS)
DLG_SPAWN = $(SCAN).cpp $(SCAN).h




mbox_SOURCES = mbox.cpp                          \
	MBoxParser.cpp MBoxParser.h              \
	$(SCAN).cpp $(SCAN).h                    \
	AParser.cpp DLexerBase.cpp ASTBase.cpp PCCTSAST.cpp ATokenBuffer.cpp 

mbox_LDADD = -lpopt @LIBFERRIS_LA@ 
mbox_LDFLAGS = @GLIB_LIBS@




FFilterParser.o : $(TOKENS) $(SCAN).h MBoxParser.cpp MBoxParser.h
	$(CXXCOMPILE) -c $(CFLAGS) -o MBoxParser.o MBoxParser.cpp

$(SCAN).o : $(SCAN).cpp $(TOKENS)
	$(CXXCOMPILE) -c $(CFLAGS) -o $(SCAN).o $(SCAN).cpp

$(ANTLR_SPAWN) : $(GRM)
	$(ANTLR) $(AFLAGS) $(GRM)

$(DLG_SPAWN) : $(DLG_FILE)
	$(DLG) $(DFLAGS) $(DLG_FILE)

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




###############################################################################
###############################################################################
###############################################################################


plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES = libmbox.la libmbox_factory.la
libmbox_factory_la_SOURCES = libmbox_factory.cpp
libmbox_la_SOURCES         = libmbox.cpp                      \
	mbox.g                                                \
	MBoxParser.cpp MBoxParser.h                           \
	AParser.cpp DLexerBase.cpp ASTBase.cpp PCCTSAST.cpp ATokenBuffer.cpp DLGLexer.cpp  \
	mbox.cpp

INCLUDES =						\
	-I$(srcdir)/..                                  \
	-I$(srcdir)/../..                               \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@PCCTS_CFLAGS@                                  \
	`glib-config --cflags gmodule`

libmbox_la_LDFLAGS = @GLIB_LIBS@

