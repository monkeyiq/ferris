/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: FerrisBackup.hh,v 1.2 2010/09/24 21:30:34 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_BACKUP_H_
#define _ALREADY_INCLUDED_FERRIS_BACKUP_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <string>

namespace Ferris
{
    namespace Util
    {
        class FERRISEXP_API BackupMaker
        {
        public:

            enum Mode_t
            {
                MODE_NONE = 0,
                MODE_NUMBERED,
                MODE_EXISTING,
                MODE_SIMPLE
            };
            
            BackupMaker();

            void setSuffix( const std::string& s );
            std::string getSuffix();

            void setMode( const std::string& s = "existing" );
            void setMode( Mode_t m );

            bool impotent();

            std::string getBackupName( fh_context c );

            void operator()( fh_context c );
            void operator()( fh_attribute a );
            void perform( fh_context c );
            void perform( const std::string& path );
            

        private:
            
            Mode_t Mode;
            std::string Suffix;
            std::string ModeString;


            std::string getBackupName_Numbered( fh_context c );
            int getHighestBackupNumber( fh_context c );
            std::string getBackupName_Simple( fh_context c );
        };
    };
};
#endif
