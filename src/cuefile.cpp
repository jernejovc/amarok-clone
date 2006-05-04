// (c) 2005 Martin Ehmke <ehmke@gmx.de>
// License: GNU General Public License V2

#define DEBUG_PREFIX "CueFile"

#include <qfile.h>
#include <qmap.h>
#include <qstringlist.h>

#include "cuefile.h"
#include "metabundle.h"
#include "enginecontroller.h"
#include "debug.h"

CueFile *CueFile::instance()
{
    static CueFile *s_instance = 0;

    if(!s_instance)
    {
        s_instance = new CueFile(EngineController::instance()); // FIXME berkus: le grand borkage (if engine is changed, e.g.)?
    }

    return s_instance;
}

CueFile::~CueFile()
{
    debug() << "shmack! destructed" << endl;
}


/*
+ - set and continue in the same state
x - cannot happen
track - switch to new state

state/next token >
 v         PERFORMER               TITLE           FILE            TRACK            INDEX          PREGAP
begin         +                      +             file              x                x              x
file          x                      x             file            track              x              x
track         +                      +             file              x                +              +
index         x                      x             file            track              +              x

1. Ignore FILE completely.
2. INDEX 00 is gap abs position in cue formats we care about (use it to calc length of prev track and then drop on the floor).
3. Ignore subsequent INDEX entries (INDEX 02, INDEX 03 etc). - FIXME? this behavior is different from state chart above.
4. For a valid cuefile at least TRACK and INDEX are required.
*/
enum {
    BEGIN = 0,
    TRACK_FOUND, // track found, index not yet found
    INDEX_FOUND
};


/**
* @return true if the cuefile could be successfully loaded
*/
bool CueFile::load()
{
    clear();
    m_lastSeekPos = -1;

    if( QFile::exists( m_cueFileName ) )
    {
        QFile file( m_cueFileName );
        int track = 0;
        QString defaultArtist = QString::null;
        QString defaultAlbum = QString::null;
        QString artist = QString::null;
        QString title = QString::null;
        long length = 0;
        long prevIndex = -1;
        long index = -1;

        int mode = BEGIN;
        if( file.open( IO_ReadOnly ) )
        {
            QTextStream stream( &file );
            QString line;

            while ( !stream.atEnd() )
            {
                line = stream.readLine().simplifyWhiteSpace();

                if( line.startsWith( "title", false ) )
                {
                    title = line.mid( 6 ).remove( '"' );
                    if( mode == BEGIN )
                    {
                        defaultAlbum = title;
                        title = QString::null;
                        debug() << "Album: " << defaultAlbum << endl;
                    }
                    else
                        debug() << "Title: " << title << endl;
                }

                else if( line.startsWith( "performer", false ))
                {
                    artist = line.mid( 10 ).remove( '"' );
                    if( mode == BEGIN )
                    {
                        defaultArtist = artist;
                        artist = QString::null;
                        debug() << "Album Artist: " << defaultArtist << endl;
                    }
                    else
                        debug() << "Artist: " << artist << endl;
                }

                else if( line.startsWith( "track", false ) )
                {
                    if( mode == TRACK_FOUND )
                    {
                        // not valid, because we have to have an index for the previous track
                        file.close();
                        debug() << "Mode is TRACK_FOUND, abort." << endl;
                        return false;
                    }
                    if( mode == INDEX_FOUND )
                    {
                        if(artist.isNull())
                            artist = defaultArtist;

                        debug() << "Inserting item: " << title << " - " << artist << " on " << defaultAlbum << " (" << track << ")" << endl;
                        // add previous entry to map
                        insert( index, CueFileItem( title, artist, defaultAlbum, track, index ) );
                        prevIndex = index;
                        index  = -1;
                        title  = QString::null;
                        artist = QString::null;
                        track  = 0;
                    }
                    track = line.section (' ',1,1).toInt();
                    debug() << "Track: " << track << endl;
                    mode = TRACK_FOUND;
                }
                else if( line.startsWith( "index", false ) )
                {
                    if( mode == TRACK_FOUND)
                    {
                        int indexNo = line.section(' ',1,1).toInt();

                        if(indexNo == 1)
                        {
                            QStringList time = QStringList::split( QChar(':'),line.section (' ',-1,-1) );

                            index = time[0].toLong()*60*1000 + time[1].toLong()*1000 + time[2].toLong()*1000/75; //75 frames per second
                            mode = INDEX_FOUND;
                            length = 0;
                        }
                        else if(indexNo == 0) // gap, use to calc prev track length
                        {
                            QStringList time = QStringList::split( QChar(':'),line.section (' ',-1,-1) );
                            length = time[0].toLong()*60*1000 + time[1].toLong()*1000 + time[2].toLong()*1000/75; //75 frames per second

                            if(prevIndex != -1)
                            {
                            	length -= prevIndex; //this[prevIndex].getIndex();
                            	debug() << "Setting length of track " << (*this)[prevIndex].getTitle() << " to " << length << " msecs." << endl;
                            	(*this)[prevIndex].setLength(length);
                            	prevIndex = -1; // FIXME safety?
                            }
                            else
                            	length = 0;
                        }
                        else
                        {
                            debug() << "Skipping unsupported INDEX " << indexNo << endl;
                        }
                    }
                    else
                    {
                        // not valid, because we don't have an associated track
                        file.close();
                        debug() << "Mode is not TRACK_FOUND but encountered INDEX, abort." << endl;
                        return false;
                    }
                    debug() << "index: " << index << endl;
                }
            }

            if(artist.isNull())
                artist = defaultArtist;

            debug() << "Inserting item: " << title << " - " << artist << " on " << defaultAlbum << " (" << track << ")" << endl;
            // add previous entry to map
            insert( index, CueFileItem( title, artist, defaultAlbum, track, index ) );
            file.close();
        }
        return true;
    }
    else {
        return false;
    }
}

void CueFile::engineTrackPositionChanged( long position, bool userSeek )
{
    position /= 1000;
    if(userSeek || position > m_lastSeekPos)
    {
        CueFile::Iterator it;
        for ( it = end(); --it != begin(); )
        {
//            debug() << "Checking " << position << " against pos " << it.key()/1000 << " title " << it.data().getTitle() << endl;
            if(it.key()/1000 <= position)
            {
                MetaBundle bundle = EngineController::instance()->bundle(); // take current one and modify it
                if(it.data().getTitle() != bundle.title()
                   || it.data().getArtist() != bundle.artist()
                   || it.data().getAlbum() != bundle.album()
                   || it.data().getTrackNumber() != bundle.track())
                {
                    bundle.setTitle(it.data().getTitle());
                    bundle.setArtist(it.data().getArtist());
                    bundle.setAlbum(it.data().getAlbum());
                    bundle.setTrack(it.data().getTrackNumber());
                    emit metaData(bundle);
                }
                break;
            }
        }
    }

    m_lastSeekPos = position;
}


#include "cuefile.moc"



