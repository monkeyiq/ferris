package Ferris;

/**
 * stream analyzer used in lucene fulltext indexing module.
 */

import java.util.*;
import java.io.Reader;
import org.apache.lucene.analysis.*;
import org.apache.lucene.analysis.standard.*;
import org.apache.lucene.analysis.PorterStemFilter;



public class MyAnalyzer extends org.apache.lucene.analysis.Analyzer
{
    static int STEM_PORTER = (1<<2); // FIXME: this should be the value in TypeDecl.hh
    
    boolean caseSensitive;
    boolean dropStopWords;
    int stemMode;
    java.util.Hashtable stopTable;
            
    public MyAnalyzer( boolean caseSensitive,
                       boolean dropStopWords,
                       int stemMode,
                       String[] droplist ) 
        {
            if( droplist.length == 0 )
            {
                droplist = StopAnalyzer.ENGLISH_STOP_WORDS;
            }
            
            caseSensitive = caseSensitive;
            dropStopWords = dropStopWords;
            stemMode      = stemMode;
            stopTable = StopFilter.makeStopTable( droplist );
        }
            
    public TokenStream tokenStream( String fieldName, Reader reader )
        {
            TokenStream ret = new StandardTokenizer( reader );
            ret = new StandardFilter( ret );
            if( !caseSensitive )
                ret = new LowerCaseFilter( ret );
            if( dropStopWords )
                ret = new StopFilter( ret, stopTable );
            if( stemMode == STEM_PORTER )
                ret = new PorterStemFilter( ret );

            return ret;
        }
};
