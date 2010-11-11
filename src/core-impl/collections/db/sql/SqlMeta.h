/****************************************************************************************
 * Copyright (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
 * Copyright (c) 2007 Alexandre Pereira de Oliveira <aleprj@gmail.com>                  *
 * Copyright (c) 2010 Ralf Engels <ralf-engels@gmx.de>                                  *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef SQLMETA_H
#define SQLMETA_H

#include "core/meta/Meta.h"
#include "core/meta/support/MetaConstants.h"
#include "amarok_sqlcollection_export.h"
#include "FileType.h"

#include <QByteArray>
#include <QMutex>
#include <QReadWriteLock>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QPixmap>
#include <QPixmap>

namespace Capabilities {
    class AlbumCapabilityDelegate;
    class ArtistCapabilityDelegate;
    class TrackCapabilityDelegate;
}
class QAction;

namespace Collections {
    class SqlCollection;
}

class SqlScanResultProcessor;

namespace Meta
{

/** The SqlTrack is a Meta::Track used by the SqlCollection.
    The SqlTrack has a couple of functions for writing values and also
    some functions for getting e.g. the track id used in the underlying database.
    However it is not recommended to interface with the database directly.

    The whole class should be thread save.
*/
class AMAROK_SQLCOLLECTION_EXPORT_TESTS SqlTrack : public Meta::Track
{
    public:
        /** Creates a new SqlTrack without.
         *  Note that the trackId and urlId are empty meaning that this track
         *  has no database representation until it's written first by setting
         *  some of the meta information.
         *  It is advisable to set at least the path.
         */
        SqlTrack( Collections::SqlCollection *collection, int deviceId,
                  const QString &rpath, int directoryId, const QString uidUrl );
        SqlTrack( Collections::SqlCollection *collection, const QStringList &queryResult );
        ~ SqlTrack();

        /** returns the title of this track as stored in the database **/
        virtual QString name() const;
        /** returns the title of the track if existing in the database,
            a value deduced from the file name otherwise */
        virtual QString prettyName() const;
        /** returns "[artist] - [title]" if both are stored in the database,
            a value deduced from the file name otherwise */
        virtual QString fullPrettyName() const;

        /** returns the KUrl object describing the position of the track */
        virtual KUrl playableUrl() const;

        /** returns a string describing the position of the track; same as url() */
        virtual QString prettyUrl() const;

        /** returns a string describing the position of the track */
        virtual QString uidUrl() const;
        virtual void setUidUrl( const QString &uid );

        /** true if there is a collection and the file exists on disk */
        virtual bool isPlayable() const;
        /** true if there is a collection, the file exists on disk and is writeable */
        virtual bool isEditable() const;

        virtual Meta::AlbumPtr album() const;
        virtual void setAlbum( const QString &newAlbum );
        virtual void setAlbum( int albumId );

        virtual void setAlbumArtist( const QString &newAlbumArtist );

        virtual void setArtist( const QString &newArtist );
        virtual Meta::ArtistPtr artist() const;
        virtual Meta::ComposerPtr composer() const;
        virtual void setComposer( const QString &newComposer );
        virtual Meta::YearPtr year() const;
        virtual void setYear( int newYear );
        virtual Meta::GenrePtr genre() const;
        virtual void setGenre( const QString &newGenre );

        virtual QString type() const;
        virtual void setType( Amarok::FileType newType );

        virtual void setTitle( const QString &newTitle );

        virtual void setUrl( int deviceId, const QString &rpath, int directoryId );

        virtual qreal bpm() const;
        virtual void setBpm( const qreal newBpm );

        virtual QString comment() const;
        virtual void setComment( const QString &newComment );

        virtual double score() const;
        virtual void setScore( double newScore );

        virtual int rating() const;
        virtual void setRating( int newRating );

        virtual qint64 length() const;
        virtual void setLength( qint64 newLength );

        virtual int filesize() const;

        virtual int sampleRate() const;
        virtual void setSampleRate( int newSampleRate );

        virtual int bitrate() const;
        virtual void setBitrate( int newBitrate );

        virtual QDateTime createDate() const;

        virtual int trackNumber() const;
        virtual void setTrackNumber( int newTrackNumber );

        virtual int discNumber() const;
        virtual void setDiscNumber( int newDiscNumber );

        virtual QDateTime firstPlayed() const;
        virtual void setFirstPlayed( const QDateTime &newTime );

        virtual QDateTime lastPlayed() const;
        virtual void setLastPlayed( const QDateTime &newTime );

        virtual int playCount() const;
        virtual void setPlayCount( const int newCount );

        virtual qreal replayGain( Meta::ReplayGainTag mode ) const;
        virtual void setReplayGain( Meta::ReplayGainTag mode, qreal value );
        virtual void beginMetaDataUpdate();
        virtual void endMetaDataUpdate();

        /** Enables or disables writing changes to the file.
         *  This function can be useful when changes are imported from the file.
         *  In such a case writing the changes back again is stupid.
         */
        virtual void setWriteFile( const bool enable )
        { m_writeFile = enable; }

        virtual void finishedPlaying( double playedFraction );

        virtual bool inCollection() const;
        virtual Collections::Collection* collection() const;

        virtual QString cachedLyrics() const;
        virtual void setCachedLyrics( const QString &lyrics );

        virtual bool hasCapabilityInterface( Capabilities::Capability::Type type ) const;

        virtual Capabilities::Capability* createCapabilityInterface( Capabilities::Capability::Type type );

        virtual void addLabel( const QString &label );
        virtual void addLabel( const Meta::LabelPtr &label );
        virtual void removeLabel( const Meta::LabelPtr &label );
        virtual Meta::LabelList labels() const;

        // SqlTrack specific methods

        int deviceId() const;
        QString rpath() const;
        int id() const;
        Collections::SqlCollection* sqlCollection() const { return m_collection; }

        /** Does it's best to remove the track from database.
         *  Considered that there is no signal that says "I am now removed"
         *  this function still tries it's best to notify everyone
         *  That the track is now removed, plus it will also delete it from
         *  the database.
         *  Do not call this directly as it does not clean up the registy.
         *  Call m_registry->deleteTrack() instead
         */
        void remove();

        // SqlDatabase specific values

        /** Some numbers used in SqlRegistry.
         *  Update if getTrackReturnValues is updated.
         */
        enum TrackReturnIndex
        {
            returnIndex_urlId = 0,
            returnIndex_urlDeviceId = 1,
            returnIndex_urlRPath = 2,
            returnIndex_urlUid = 3,
            returnIndex_trackId = 4
        };

        // SqlDatabase specific values

        /** returns a string of all database values that can be fetched for a track */
        static QString getTrackReturnValues();
        /** returns the number of return values in getTrackReturnValues() */
        static int getTrackReturnValueCount();
        /** returns a string of all database joins that are required to fetch all values for a track*/
        static QString getTrackJoinConditions();


    protected:
        /** Will commit all changes in m_cache.
         *  commitMetaDataChanges will do three things:<br>
         *  1. It will update the member variables.
         *  2. It will call all write methods
         *  3. It will notify all observers and the collection about the changes.
         */
        void commitMetaDataChanges();
        void writeMetaDataToFile();
        void writeMetaDataToDb( const FieldHash &fields );
        void writeUrlToDb( const FieldHash &fields );
        void writePlaylistsToDb( const FieldHash &fields, const QString &oldUid );
        void writeStatisticsToDb( const FieldHash &fields );
        void writeStatisticsToDb( qint64 field );

    private:
        //helper functions
        static QString prettyTitle( const QString &filename );

        Collections::SqlCollection* const m_collection;

        QString m_title;

        // the url table
        int m_urlId;
        int m_deviceId;
        QString m_rpath;
        int m_directoryId; // only set when the urls table needs to be written
        KUrl m_url;
        QString m_uid;

        // the rest
        int m_trackId;
        int m_statisticsId;

        qint64 m_length;
        qint64 m_filesize;
        int m_trackNumber;
        int m_discNumber;
        QDateTime m_lastPlayed;
        QDateTime m_firstPlayed;
        int m_playCount;
        int m_bitrate;
        int m_sampleRate;
        int m_rating;
        double m_score;
        QString m_comment;
        qreal m_bpm;
        qreal m_albumGain;
        qreal m_albumPeakGain;
        qreal m_trackGain;
        qreal m_trackPeakGain;
        QDateTime m_createDate;

        Meta::AlbumPtr m_album;
        Meta::ArtistPtr m_artist;
        Meta::GenrePtr m_genre;
        Meta::ComposerPtr m_composer;
        Meta::YearPtr m_year;

        Amarok::FileType m_filetype;

        bool m_batchUpdate;
        bool m_writeFile;
        bool m_writeAllStatisticsFields;
        FieldHash m_cache;

        /** This ReadWriteLock is protecting all internal variables.
            It is ensuring that m_cache, m_batchUpdate and the othre internal variable are
            in a consistent state all the time.
        */
        mutable QReadWriteLock m_lock;

        mutable bool m_labelsInCache;
        mutable Meta::LabelList m_labelsCache;
};

class AMAROK_SQLCOLLECTION_EXPORT_TESTS SqlArtist : public Meta::Artist
{
    public:
        SqlArtist( Collections::SqlCollection* collection, int id, const QString &name );
        ~SqlArtist();

        virtual QString name() const { return m_name; }

        virtual void invalidateCache();

        virtual Meta::TrackList tracks();

        virtual Meta::AlbumList albums();

        virtual bool hasCapabilityInterface( Capabilities::Capability::Type type ) const;

        virtual Capabilities::Capability* createCapabilityInterface( Capabilities::Capability::Type type );

        //SQL specific methods
        int id() const { return m_id; }
    private:
        Collections::SqlCollection* const m_collection;
        const int m_id;
        const QString m_name;

        bool m_tracksLoaded;
        Meta::TrackList m_tracks;
        bool m_albumsLoaded;
        Meta::AlbumList m_albums;
        QMutex m_mutex;

        friend class SqlTrack; // needs to call notifyObservers
};

/** Represents an albums stored in the database.
    Note: The album without name is special. It will always be a compilation
    and never have a picture.
*/
class AMAROK_SQLCOLLECTION_EXPORT_TESTS SqlAlbum : public Meta::Album
{
    public:
        SqlAlbum( Collections::SqlCollection* collection, int id, const QString &name, int artist );
        ~SqlAlbum();

        virtual QString name() const { return m_name; }

        virtual void invalidateCache();

        virtual Meta::TrackList tracks();

        /** Returns true if this album is a compilation, meaning that the included songs are from different artists.
        */
        virtual bool isCompilation() const;

        /** Returns true if this album has an artist.
         *  The following equation is always true: isCompilation() != hasAlbumArtist()
         */
        virtual bool hasAlbumArtist() const;

        /** Returns the album artist.
         *  Note that setting the album artist is not supported.
         *  A compilation does not have an artist and not only an empty artist.
         */
        virtual Meta::ArtistPtr albumArtist() const;

        //updating album images is possible for local tracks, but let's ignore it for now

        /** Returns true if the album has a cover image.
         *  @param size The maximum width or height of the result.
         *  when size is <= 1, return the full size image
         */
        virtual bool hasImage(int size = 0) const;
        virtual bool canUpdateImage() const { return true; }

        /** Returns the album cover image.
         *  Returns a default image if no specific album image could be found.
         *  In such a case it will start the cover fetcher.
         *
         *  Note: as this function can create a pixmap it is not recommended to
         *  call this function from outside the UI thread.
         *
         *  @param size is the maximum width or height of the resulting image.
         *  when size is <= 1, return the full size image
         */
        virtual QPixmap image( int size = 0 );

        virtual KUrl imageLocation( int size = 0 );
        virtual void setImage( const QImage &image );
        virtual void removeImage();
        virtual void setSuppressImageAutoFetch( const bool suppress ) { m_suppressAutoFetch = suppress; }
        virtual bool suppressImageAutoFetch() const { return m_suppressAutoFetch; }

        virtual bool hasCapabilityInterface( Capabilities::Capability::Type type ) const;

        virtual Capabilities::Capability* createCapabilityInterface( Capabilities::Capability::Type type );

        //SQL specific methods
        int id() const { return m_id; }

        /** Set the compilation flag.
         *  Actually it does not cange this album but instead moves
         *  the tracks to other albums (e.g. one with the same name which is a
         *  compilation)
         *  If the compilation flag is set to "false" then all songs
         *  with different artists will be moved to other albums, possibly even
         *  creating them.
         */
        void setCompilation( bool compilation );
        Collections::SqlCollection *sqlCollection() const { return m_collection; }

    private:
        /** Cleans all images from this album cached in the QPixmapCache */
        void clearPixmapCache();

        QByteArray md5sum( const QString& artist, const QString& album, const QString& file ) const;

        /** Returns a unique key for the album cover. */
        QByteArray imageKey() const;

        /** Returns the path that the large scale image should have on the disk
         *  Does not check if the file exists.
         *  Note: not all large images have a disk cache, e.g. if they are set from outside
         *    or embedded inside an audio file.
         *    The largeDiskCache is only used for images set via setImage(QImage)
         */
        QString largeDiskCachePath() const;

        /** Returns the path that the image should have on the disk
         *  Does not check if the file exists.
         *  @param size is the maximum width or height of the resulting image.
         *         size==0 is the large image and the location of this file is completely different.
         *         there should never be a scaled cached version of the large image. it dose not make
         *         sense.
         */
        QString scaledDiskCachePath( int size ) const;

        /** Returns the path to the large image
         * Queries the database for the path of the large scale image.
         */
        QString largeImagePath();

        /** Updates the database
         *  Sets the current albums image to the given path.
         *  The path should point to a valid image.
         *  Note: setImage will not delete the already set image.
         */
       void setImage( const QString &path );

       /** Finds or creates a magic value in the database which tells Amarok not to auto fetch an image since it has been explicitly unset.
       */
       int unsetImageId() const;

    private:
        Collections::SqlCollection* const m_collection;


        QString m_name;
        int m_id; // the id of this album in the database
        int m_artistId;
        int m_imageId;
        mutable QString m_imagePath; // path read from the database
        mutable bool m_hasImage; // true if we have an original image
        mutable bool m_hasImageChecked; // true if hasImage was checked
        QSet<QString> m_pixmapCacheIds; // all our image keys
        bool m_pixmapCacheDirty; // true if we should remove our images from the cache

        mutable int m_unsetImageId; // this is the id of the unset magic value in the image sql database
        static const QString AMAROK_UNSET_MAGIC;

        bool m_tracksLoaded;
        bool m_suppressAutoFetch;
        Meta::ArtistPtr m_artist;
        Meta::TrackList m_tracks;
        mutable QMutex m_mutex;

        //TODO: add album artist

        friend class SqlTrack; // needs to call notifyObservers
        friend class ::SqlScanResultProcessor; // needs to set images directly
};

class AMAROK_SQLCOLLECTION_EXPORT_TESTS SqlComposer : public Meta::Composer
{
    public:
        SqlComposer( Collections::SqlCollection* collection, int id, const QString &name );

        virtual QString name() const { return m_name; }

        virtual void invalidateCache();

        virtual Meta::TrackList tracks();

        //SQL specific methods
        int id() const { return m_id; }

    private:
        Collections::SqlCollection* const m_collection;

        const int m_id;
        const QString m_name;

        bool m_tracksLoaded;
        Meta::TrackList m_tracks;
        QMutex m_mutex;

        friend class SqlTrack; // needs to call notifyObservers
};

class SqlGenre : public Meta::Genre
{
    public:
        SqlGenre( Collections::SqlCollection* collection, int id, const QString &name );

        virtual QString name() const { return m_name; }

        /** Invalidates the tracks cache */
        /** Invalidates the tracks cache */
        virtual void invalidateCache();

        virtual Meta::TrackList tracks();

        //SQL specific methods
        int id() const { return m_id; }

    private:
        Collections::SqlCollection* const m_collection;

        const int m_id;
        const QString m_name;

        bool m_tracksLoaded;
        Meta::TrackList m_tracks;
        QMutex m_mutex;

        friend class SqlTrack; // needs to call notifyObservers
};

class AMAROK_SQLCOLLECTION_EXPORT_TESTS SqlYear : public Meta::Year
{
    public:
        SqlYear( Collections::SqlCollection* collection, int id, int year );

        virtual QString name() const { return QString::number(m_year); }

        virtual int year() const { return m_year; }

        /** Invalidates the tracks cache */
        virtual void invalidateCache();

        virtual Meta::TrackList tracks();

        //SQL specific methods
        int id() const { return m_id; }

    private:
        Collections::SqlCollection* const m_collection;
        const int m_id;
        const int m_year;


        bool m_tracksLoaded;
        Meta::TrackList m_tracks;
        QMutex m_mutex;

        friend class SqlTrack; // needs to call notifyObservers
};

class AMAROK_SQLCOLLECTION_EXPORT_TESTS SqlLabel : public Meta::Label
{
public:
    SqlLabel( Collections::SqlCollection *collection, int id, const QString &name );

    virtual QString name() const { return m_name; }

    /** Invalidates the tracks cache */
    virtual void invalidateCache();

    virtual Meta::TrackList tracks();

    //SQL specific methods
    int id() const { return m_id; }

private:
    Collections::SqlCollection* const m_collection;
    const int m_id;
    const QString m_name;

    bool m_tracksLoaded;
    Meta::TrackList m_tracks;
    QMutex m_mutex;

    friend class SqlTrack; // needs to call notifyObservers
};

}

#endif /* SQLMETA_H */
