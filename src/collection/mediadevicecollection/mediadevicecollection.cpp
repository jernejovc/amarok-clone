/* 
   Mostly taken from Daap code:
   Copyright (C) 2006 Ian Monroe <ian@monroe.nu>
   Copyright (C) 2006 Seb Ruiz <ruiz@kde.org>  
   Copyright (C) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#define DEBUG_PREFIX "MediaDeviceCollection"

#include "mediadevicecollection.h"

#include "amarokconfig.h"
#include "mediadevicemeta.h"
#include "Debug.h"
#include "MediaDeviceCache.h"
#include "MemoryQueryMaker.h"

#include <QStringList>


AMAROK_EXPORT_PLUGIN( MediaDeviceCollectionFactory )

MediaDeviceCollectionFactory::MediaDeviceCollectionFactory()
    : CollectionFactory()
{
    //nothing to do
}

MediaDeviceCollectionFactory::~MediaDeviceCollectionFactory()
{
  //    delete m_browser;
}

void
MediaDeviceCollectionFactory::init()
{
    DEBUG_BLOCK    
      /* Test out using the cache */
    MediaDeviceCache::instance()->refreshCache();
    QStringList udiList = MediaDeviceCache::instance()->getAll();
    debug() << "MediaDeviceCollection found " << udiList;

    /* test mediadevicecollection constructor */
    MediaDeviceCollection *coll = new MediaDeviceCollection();
    delete coll;

    return;
}

//MediaDeviceCollection

MediaDeviceCollection::MediaDeviceCollection()
    : Collection()
    , MemoryCollection()
{
    DEBUG_BLOCK

    /* test out using libgpod */
    m_itdb = 0;
    m_itdb = itdb_new();
    if(m_itdb) {
      debug() << "Itunes database created!";
      itdb_free(m_itdb);
    }
    else
      debug() << "Itunes database not created!";
}

MediaDeviceCollection::~MediaDeviceCollection()
{

}

void
MediaDeviceCollection::startFullScan()
{
    //ignore
}

QueryMaker*
MediaDeviceCollection::queryMaker()
{
    return new MemoryQueryMaker( this, collectionId() );
}

QString
MediaDeviceCollection::collectionId() const
{
     return "filler";
}

QString
MediaDeviceCollection::prettyName() const
{
    return "prettyfiller";
}

#include "mediadevicecollection.moc"

