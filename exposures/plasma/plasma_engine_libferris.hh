/*
 *   libferris
 *   Copyright (C) 2001-2010 Ben Martin
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
 
#ifndef FERRISENGINE_H
#define FERRISENGINE_H
 
// We need the DataEngine header, since we are inheriting it
#include <Plasma/DataEngine>
#include <Plasma/Service>
#include <QString>

class FerrisEngine;

class FerrisService : public Plasma::Service
{
    Q_OBJECT;
    FerrisEngine* m_engine;
    QString m_source;
    
public:
    FerrisService( FerrisEngine* engine, QString src ); 
    Plasma::ServiceJob * createJob(const QString &operation,
                                   QMap<QString, QVariant> &parameters);
};



/**
 * This engine provides information from the Australian Bureau of Meteorology.
 */
class FerrisEngine : public Plasma::DataEngine
{
    Q_OBJECT;
    QStringList m_sources;
    
private:
    
public:
    FerrisEngine(QObject* parent, const QVariantList& args);
    Plasma::Service* serviceForSource(const QString &source);
 
protected:
    virtual bool sourceRequestEvent(const QString& name);
    virtual bool updateSourceEvent(const QString& source);
    virtual QStringList sources() const;
    virtual void init();
            
public slots:
    
};
 
#endif // FERRISENGINE_H
