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

    $Id: libferrisid3.cpp,v 1.6 2010/09/24 21:31:55 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>

#include <id3/tag.h>


using namespace std;
namespace Ferris
{


class FERRISEXP_DLLLOCAL ImageDataEAGenerator_ID3
    :
        public MatchedEAGeneratorFactory
{

    ID3_Tag Tag;

protected:

    virtual void Brew( const fh_context& a );

public:

    ImageDataEAGenerator_ID3();
    inline const ID3_Tag& getTag()
        {
            return Tag;
        }

    void augmentRecommendedEA( const fh_context& a, fh_stringstream& ss )
        {
            ss << ",artist,album,year,title,track";
        }
};



class FERRISEXP_DLLLOCAL ID3Attribute
{
    ImageDataEAGenerator_ID3* ID3;
    ID3_FrameID FRID;
    ID3_FieldID FIID;

protected:
    
    inline ImageDataEAGenerator_ID3* getID3() { return ID3; }
    inline ID3_FrameID               getFRID(){ return FRID; }
    inline ID3_FieldID               getFIID(){ return FIID; }
    inline ID3_Tag                   getTag(){ return getID3()->getTag(); }

    ID3Attribute(
        ImageDataEAGenerator_ID3* id3,        
        ID3_FrameID id,
        ID3_FieldID fld
        )
        :
        ID3(id3), FRID(id), FIID(fld)
        {
        }
    
};

class FERRISEXP_DLLLOCAL ID3StringAttribute
    :
    public ID3Attribute,
    public EA_Atom_ReadWrite
{
    string Value;

public:

    fh_iostream getStream( Context* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            ss << Value;
            return ss;
        }

    void setStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
        {
            ID3_Frame* Fra = getTag().Find(getFRID(), getFIID(), Value.c_str() );
            ID3_Field* F = Fra ? Fra->GetField( getFIID() ) : 0;
     
            if( F )
            {
                string s;
                getline( ss, s );
                F->Set( s.c_str() );
                Value = s;
                getTag().Update();
            }
        }

public:

    ID3StringAttribute( ImageDataEAGenerator_ID3* id3,
                        ID3_FrameID id,
                        ID3_FieldID fld,
                        const string& s )
        :
        EA_Atom_ReadWrite( this, &ID3StringAttribute::getStream,
                           this, &ID3StringAttribute::getStream,
                           this, &ID3StringAttribute::setStream ),
        ID3Attribute( id3, id, fld ),
        Value(s)
        {
        }
};



class FERRISEXP_DLLLOCAL ID3ByteArrayAttribute
    :
    public EA_Atom_ReadOnly,
    public ID3Attribute
{
    ID3_Field* F;

protected:

    fh_iostream getStream( Context* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            int sz = F->BinSize();
            ss.write( (char*)F->GetRawBinary(), sz );
            return ss;
        }
    
public:
    
    ID3ByteArrayAttribute(
        ImageDataEAGenerator_ID3* id3,
        ID3_FrameID id,
        ID3_FieldID fld,
        ID3_Field* f
        )
        :
        EA_Atom_ReadOnly( this, &ID3ByteArrayAttribute::getStream ),
        ID3Attribute( id3, id, fld ),
        F(f)
        {
        }
};
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


ImageDataEAGenerator_ID3::ImageDataEAGenerator_ID3()
    :
    MatchedEAGeneratorFactory()
{
}


void
ImageDataEAGenerator_ID3::Brew( const fh_context& a )
{
    LG_ID3_D << "ImageDataEAGenerator_ID3" << endl;
    

    
    try
    {
        string dn = a->getDirPath();
        fh_istream iss = a->getLocalIStream( dn );
        LG_ID3_D << "dn:" << dn << endl;

        Tag.Link( dn.c_str() );

//        FerrisHandle<ID3_Tag::Iterator> iter = Tag.CreateIterator();
        Loki::SmartPtr< ID3_Tag::Iterator,
                  Loki::DestructiveCopy,
                  Loki::DisallowConversion,
                  Loki::AssertCheck,
                  Loki::DefaultSPStorage > iter(Tag.CreateIterator());
        
        ID3_Frame* myFrame = NULL;
        while (NULL != (myFrame = iter->GetNext()))
        {
            LG_ID3_D << "Frame:" << myFrame->GetDescription() << endl;
//            cerr     << "Frame:" << myFrame->GetDescription() << endl;
            
//            FerrisHandle<ID3_Frame::Iterator> iter = myFrame->CreateIterator();
            Loki::SmartPtr< ID3_Frame::Iterator,
                Loki::DestructiveCopy,
                Loki::DisallowConversion,
                Loki::AssertCheck,
                Loki::DefaultSPStorage > iter(myFrame->CreateIterator());
            
            ID3_Field* myField = NULL;
            ID3_Field* initialField = NULL;
            string  running_str = "";
            guint32 string_field_number = 0;
            guint32 binary_field_number = 0;
            guint32    int_field_number = 0;
            
            while (NULL != (myField = iter->GetNext()))
            {
                if( !initialField )
                    initialField = myField;
                

                switch( myField->GetType() )
                {
                case ID3FTY_NONE:
                case ID3FTY_INTEGER:
                {
                    guint32 val = myField->Get();

                    ostringstream ss;
                    ss <<        myFrame->GetDescription()
                       << "-" << int_field_number;

                    a->addAttribute( tostr(ss), tostr(val), XSD_BASIC_INT );

//                     if( StringAttribute* sa = new ID3uint32Attribute(
//                             a, tostr(ss), this,
//                             myFrame->GetID(), myField->GetID(), val))
//                     {
//                         ret.push_back( sa );
//                     }
                    ++int_field_number;
                    break;
                }
                

                case ID3FTY_BINARY:
                {
                    if( myFrame && myFrame->GetDescription() )
                    {
                        ostringstream ss;
                        ss <<        myFrame->GetDescription()
                           << "-" << binary_field_number;

                        a->addAttribute(
                            tostr(ss), 
                            (EA_Atom*)new ID3ByteArrayAttribute(
                                this,
                                myFrame->GetID(), myField->GetID(), myField),
                            FXD_BINARY );
                    }
                    
                    ++binary_field_number;
                    break;
                }

//                 // A start on utf16 support.
//                 {
//                     const unicode_t* utxt = myField->GetRawUnicodeText();
//                     cerr << " utxt:" << utxt << " encoding:" <<  myField->GetEncoding() << endl;
//                     if( !utxt )
//                         continue;
//                     cerr << "Have utxt" << endl;

//                     string s;
//                     wstring sw( (const wchar_t*)utxt );
//                     s = Util::wstring_to_utf8( s, sw );
//                     cerr << "Have string:" << s << endl;
//                 }
                
                
                case ID3FTY_TEXTSTRING:
                {
                    if( !myField->GetRawText() )
                        continue;
                    
                    string s(myField->GetRawText());
                    s = Util::iso8859_to_utf8( s );
                    
                    LG_ID3_D << "Field:" << s << endl;
                    running_str += s;

                    bool skipAttr = false;
                    ostringstream ss;
                    switch( myFrame->GetID() )
                    {
                    case ID3FID_TITLE:
                        ss << "title";
                        break;
                    case ID3FID_SUBTITLE:
                        ss << "subtitle";
                        break;
                    case ID3FID_LEADARTIST:
                        ss << "artist";
                        break;
                    case ID3FID_WWWARTIST:
                        ss << "artist-url";
                        break;
                    case ID3FID_ALBUM:
                        ss << "album";
                        break;
                    case ID3FID_YEAR:
                        ss << "year";
                        break;
                    case ID3FID_TRACKNUM:
                        ss << "track";
                        break;
                    case ID3FID_SONGLEN:
                        ss << "length";
                        break;
                    case ID3FID_COMPOSER:
                        ss << "composer";
                        break;
                    case ID3FID_LANGUAGE:
                        ss << "human-language";
                        break;
                    case ID3FID_BAND:
                        ss << "band";
                        break;
                    case ID3FID_CONDUCTOR:
                        ss << "conductor";
                        break;
                        
                        
                    default:
                        skipAttr = (myFrame->GetDescription() == 0);
                        if( !skipAttr )
                        {
                            ss <<        myFrame->GetDescription()
                               << "-" << string_field_number;
                            ++string_field_number;
                        }
                    }

                    if( skipAttr )
                        continue;

                    a->addAttribute( tostr(ss), 
                                     (EA_Atom*)new ID3StringAttribute(
                                         this,
                                         myFrame->GetID(),
                                         myField->GetID(),
                                         s),
                                     XSD_BASIC_STRING );
                    
                    break;
                }
                
                
                case ID3FTY_NUMTYPES:
                    break;
                    
                }
            }

            if( running_str.length() )
            {
                const string& rdn = myFrame->GetDescription();

                a->addAttribute( rdn,
                                 (EA_Atom*)new ID3StringAttribute( this,
                                                                   myFrame->GetID(),
                                                                   initialField->GetID(),
                                                                   running_str),
                                 XSD_BASIC_STRING );
            }
        }
    }
    catch( exception& e )
    {
        LG_ID3_W << "Failed to load ID3 EA, error:" << e.what() << endl;
    }
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern "C"
{
    FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
    {
        return new ImageDataEAGenerator_ID3();
    }
};
 
};
