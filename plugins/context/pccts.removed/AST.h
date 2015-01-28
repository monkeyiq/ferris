#ifndef AST_h
#define AST_h

#include <ASTBase.h>
#include <AParser.h>
#include <AToken.h>
#include <ferrisPcctsContext.hh>

class AST : public ASTBase
        {
        protected:

            ::ANTLRTokenPtr token;

        public:
            /* This ctor is implicitly called when you ref node constructor #[tok,s] */
            AST( ::ANTLRTokenType tok, char *s)
                {
                    token = new Ferris::ANTLRToken(tok, s);
                }
            AST( ::ANTLRTokenPtr t)
                {
                    token = t;
                }

            void preorder_action()
                {
                    char *s = token->getText();
                    printf(" %s", s);
                }


            std::string getName()
                {
                    return token->getText();
                }
    
        };

 


#endif

