/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

    libferris is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libferris is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libferris.  If not, see <http://www.gnu.org/licenses/>.

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: Support.hh,v 1.5 2010/09/24 21:31:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_SUPPORT_H_
#define _ALREADY_INCLUDED_SUPPORT_H_

#include <AParser.h>
#include <FerrisContextPlugin.hh>
#include <SignalStreams.hh>
#include <DLexerBase.h>
#include <AToken.h>

#include <string>
#include <iomanip>

using namespace std;

namespace Ferris
{
    

// FIXME:
inline void myPanic( const std::string& s )
{
    cerr << "PCCTS myPanic() s:" << s << endl;
    exit(1);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class DLGInputIStream : public DLGInputStream
{
private:

    fh_istream iss;

public:

	DLGInputIStream(const fh_istream& v)
        :
        iss(v)
        {
        }

    int nextChar()
		{
            char ch;
            // Used good() on stream result before.
            if(iss >> std::noskipws >> ch)
            {
                return ch;
            }
            return EOF;
		}

};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <ATokPtr.h>

struct ANTLRAbstractToken;

class SimpleToken : public ANTLRRefCountToken
{

protected:
    
    ANTLRTokenType      _type;

public:

    int _line;
    int serial;


    SimpleToken(ANTLRTokenType t)
        :
        _line(0),
        serial(bumpCounter())
        {
            setType(t);
        }

    SimpleToken()
        :
        _line(0),
        serial(bumpCounter())
        {
            setType((ANTLRTokenType)0);
        }

    
    ANTLRTokenType getType() const  { return _type; }
    void setType(ANTLRTokenType t)  { _type = t; }
    virtual int getLine() const     { return _line; }
    void setLine(int line)          { _line = line; }

    int bumpCounter() 
        {
            static int counter = 0;
            return ++counter;
        }
    
};

struct ANTLRToken : public SimpleToken
{
private:

    void init()
        {
            pText=0;
            ws=0;
            col=0;
        }
    
public:

    int                      col;        // column
    ANTLRChar *              pText;
    ANTLRChar *              ws;     // whitespace string

    ANTLRToken()
        {
            init();
        }

    ANTLRToken(ANTLRTokenType tokenTypeNew)
        {
            setType(tokenTypeNew);
            setLine(0);
            init();
        }

    ANTLRToken(ANTLRTokenType    tokenTypeNew,
               const ANTLRChar * textNew,   
               int               lineNew=0)
        {
            setType(tokenTypeNew);
            setLine(lineNew);
            init();
            setText(textNew);
        }         

    ANTLRToken(const ANTLRToken & from)
        :
        SimpleToken(from)
        {
            init();
            col=from.col;
            setText(from.pText);
            setws(from.ws);
        };  

    virtual ~ANTLRToken()
        {
            delete [] pText;
            pText=0;
            delete [] ws;
            ws=0;
        }


    ANTLRToken & operator = (const ANTLRToken & from)
        {
            this->SimpleToken::operator = (from);
            if (this != &from) {
                col=from.col;
                setText(from.pText);
                setws(from.ws);
            };
            return *this;
        }      
    

    void setText(const ANTLRChar *s)
        {
            
            if (pText != 0) {
                delete [] pText;
                pText=0;
            };
            if (s != 0) {
                pText=new ANTLRChar [strlen(s)+1];
                if (pText == 0)
                    {
                        myPanic ("ANTLRToken::setText strdup failed");
                    }
                
                strcpy(pText,s);
            };
        }

    void setws(ANTLRChar *s)
        {
            if (ws != 0) {
                delete [] ws;
                ws=0;
            };
            if (s != 0) {
                ws=new ANTLRChar [strlen(s)+1];
                if (ws == 0) myPanic ("ANTLRToken::setws strdup failed");
                strcpy(ws,s);
            };
        }

    ANTLRAbstractToken * makeToken(ANTLRTokenType   tokenType,
                                   ANTLRChar *      text,
                                   int              line)
        {
            return new ANTLRToken(tokenType,text,line);
        }
    

    virtual ANTLRChar* getText() const
        {
            return pText;
        }

    void dumpNode(const char * s=0)
        {

            ANTLRChar *  theText=0;
            ANTLRChar *  theWS=0;
            ANTLRChar *  p=0;
            
            if (s != 0) {printf("%s ",s);};
            if (getType() == Eof) {
                printf("TokenType \"EOF\" Token # %d\n",serial);
            } else {
                if (pText == 0) {
                    theText="";
                } else if (strcmp(pText,"\n") == 0) {
                    theText="NL";
                } else {
                    theText=pText;
                }; 
                if (ws != 0) {
                    theWS=new ANTLRChar [strlen(ws)+1];
                    if (theWS == 0) myPanic ("ANTLRToken::dumpNode strdup failed");
                    strcpy(theWS,ws);
                    for (p=theWS;*p != 0;p++) {
                        if (*p == ' ') *p='.';
                        if (*p == '\n') *p='$';
                    };
                };
                printf("TokenType (%s) Text (%s) WS (%s) Token # %d  Line=%d  Col=%d\n",
                       PARSER::tokenName(getType()),
                       theText,
                       theWS,
                       serial,     
                       getLine(),
                       col);
                delete [] theWS;
            };
        }


    
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


#ifndef PCCTSLEXERSUBCLASSNAME
#define PCCTSLEXERSUBCLASSNAME MyDLGLexer
#endif

#ifndef PCCTSLEXERCLASSNAME
#define PCCTSLEXERCLASSNAME DLGLexer
#endif

struct PCCTSLEXERSUBCLASSNAME : public PCCTSLEXERCLASSNAME
{
    int              foundRealToken;
    ANTLRTokenPtr    next;
    ANTLRTokenPtr    prev;
    ANTLRChar *      ws;         //  whitespace str
    int              wslen;      //  actual len of str
    int              wsmax;      //  space allocated for str
//    bool             isWS;
    
    PCCTSLEXERSUBCLASSNAME(DLGInputStream *in,unsigned bufsize=2000)
        :
        PCCTSLEXERCLASSNAME(in,bufsize),
        ws(0),
        wslen(0),
        wsmax(0),
        prev(0),
        next(0),
//        isWS(0),
        foundRealToken(0)
        {
            isWS = 0;
            trackColumns();

//            cerr << "Created a MyDLGLexer" << endl;
        }

    virtual ~PCCTSLEXERSUBCLASSNAME()
        {
            delete [] ws;
        }


    _ANTLRTokenPtr MyGetToken()
        {
            if ( token_to_fill==NULL ) panic("NULL token_to_fill");
            ANTLRTokenType tt = nextTokenType();

//            cerr << "MyGetToken() tt:" << tt << " _lextext:" << _lextext << endl; 
            
            _ANTLRTokenPtr tk = token_to_fill->makeToken(tt, _lextext,_line);
            return tk;
        }


    ::ANTLRAbstractToken * getToken()
        {
            static int callcount = 0;

            //bool z = 0; if( z ) return DLGLexer::getToken();
            //cerr << "getToken() callcount:" << callcount++ << endl;
            
            ANTLRChar *      p;

            //
            // If we have hit the End of File then we can do no more
            // prev is not set for the first invokation
            //
            if ( prev!=0 && prev->getType() == Eof) {
                goto exit;
            };

            // Lets just use the standard lexer.
//             prev=DLGLexer::getToken(); // <<<===
//             goto exit;
            
            
            //
            //  only until first non-ws token (foundRealToken) is found
            //
            if ( !foundRealToken )
            {
//                prev=DLGLexer::getToken();
                prev=MyGetToken();
//                cerr << "!foundRealToken token:" << prev->getText() << endl;
                if (isWS) {
//                    cerr << "got ws token!" << endl;
                    isWS=0;
                    mytoken(prev)->col=begcol();
                    mytoken(prev)->setws(mytoken(prev)->getText());
                    mytoken(prev)->setText(0);
                    goto exit;
                } else {
                    foundRealToken=1;
                    mytoken(prev)->col=begcol();
                    next=prev;
                };
            } else {
                /* nothing */
            };
            
            //
            //  the immediately preceding token was a real token which stopped the
            //    scan for ws
            //  now start the scan for ws following the latest real token
            //
            prev=next;
            while (1)
            {
                next=MyGetToken(); // <<<===
//                next=DLGLexer::getToken(); // <<<===

//                cerr << "while(1) token:" << prev->getText() << endl;

                if (isWS == 0) break;
                wsAppend(next->getText());
                isWS=0;
            };            
            mytoken(next)->col=begcol();
            //
            //  now copy trailing ws to previous real token
            //
            if (wslen == 0)
            {
                p=0;
            }
            else
            {
                p=new ANTLRChar [wslen+1];
                if (p == 0) myPanic("MyDLGLexer::getToken new ANTLRChar failed");   
                strcpy(p,ws);
                wslen=0;
            };
            
            mytoken(prev)->ws=p;

        exit:

//            cerr << "Returning token:" << prev->getText() << endl;
            
            return mytoken(prev);
        }

    
    void wsAppend(ANTLRChar * s)
        {
            int        l;
            ANTLRChar* wsnew;

//            cerr << "wsAppend(" << s << ")" << endl;
            

            l=strlen(s);
            if (wslen+l >= wsmax-1)
            {
                wsmax=wslen+l+1000;
                wsnew=new ANTLRChar [wsmax];
                if (wsnew == 0) myPanic("MyDLG::wsAppend new failed");
                if (ws != 0)
                {
                    strcpy(wsnew,ws);
                    delete [] ws;
                };
                ws=wsnew;
            };
            strcpy(&(ws[wslen]),s);
            wslen+=l;
        }


//     virtual ANTLRTokenType erraction()
//         {
//             ostringstream ss;
//             ss << "Error at :" << line() << "lexErrCount: " << lexErrCount << endl;
//             myPanic(tostr(ss));
//        }
    


    void tabAdjust()
        {
            char * p=0;

//            cerr << "tabAdjust()" << endl;

            // give better error messages when lexical problem on first of multi-line
            //   element.  Variable begline is in class derived from DLGLexer.

            if (_lextext == _begexpr) begline=_line;

            // back out naive computation of column position by DLG

            _endcol=_endcol - ( _endexpr - _begexpr);
            
            for (p=_begexpr;*p != 0; p++) {
                if (*p == '\n') {
                    newline();_endcol=0;
                } else if (*p == '\t') {
                    _endcol=((_endcol-1) & ~7) + 8;
                };
                _endcol++;
            };
            _endcol--;           // DLG will compute begcol=endcol+1
        }


    
};







///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


template<class Lexer, class Parser, class Token>
class MyParserBlackBox
{
private:

    DLGInputIStream in;
 	Lexer *scan;
 	ANTLRTokenBuffer* pipe;
 	_ANTLRTokenPtr tok;
	Parser* _parser;
    
public:

	inline MyParserBlackBox( const fh_istream& v )
        :
        in( v ),
        scan(0),
        pipe(0),
        tok(0),
        _parser(0)
		{
            scan = new Lexer( &in );
            pipe = new ANTLRTokenBuffer(scan,1);
            tok = new Token;
            scan->setToken(tok);
            _parser = new Parser(pipe);
            _parser->init();

            scan->debugLexer(1);
		}

    inline ~MyParserBlackBox()
        {
            delete scan; delete pipe; delete _parser; delete tok;
		}

    inline Parser *parser()	   { return _parser; }
    inline Lexer  *getLexer()  { return scan;    }
};

 
};

#endif
