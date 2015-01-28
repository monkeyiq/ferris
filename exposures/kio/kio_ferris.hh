/*
 *   Copyright 2012 Ben Martin <monkeyiq@example.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  For more details see the COPYING file in the root directory of this
 *  distribution.
 */

#ifndef KIOFERRIS_H
#define KIOFERRIS_H
 
#include <kio/slavebase.h>
#include <kio/global.h>
#include <kconfiggroup.h>
 
/**
 *
 *
 * 
 */
class KIOFerris : public KIO::SlaveBase
{
  public:
    KIOFerris( const QByteArray &pool, const QByteArray &app );
    void get( const KUrl &url );
    void stat( const KUrl& url );
    void listDir( const KUrl& url );
    KConfigGroup* config();
    bool hasMetaData( const QString& key	) const;

    void open( const KUrl& url, QIODevice::OpenMode mode );
    void write( const QByteArray& data );
    void put( const KUrl& url, int permissions, KIO::JobFlags flags );
    void seek( KIO::filesize_t offset );
    

    
    QString m_openURL;
};
 
#endif
