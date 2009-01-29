/***************************************************************************
 *   Copyright (c) 2007  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/ 

#ifndef AMAROKSCRIPTABLESERVICEMANAGER_H
#define AMAROKSCRIPTABLESERVICEMANAGER_H

#include "AmarokProcess.h"
#include "../ServiceBase.h"
#include "ScriptableService.h"

#include <QObject>
#include <QString>

class ScriptableServiceManager;

namespace The {
    AMAROK_EXPORT ScriptableServiceManager* scriptableServiceManager();
}
 
class ScriptableServiceManager : public QObject
{
    Q_OBJECT

    friend ScriptableServiceManager* The::scriptableServiceManager();

    public:
        void removeRunningScript( const QString &name );
        ScriptableService * service( const QString &name );
        

    signals:
        /**
         * Signal emitted whenever a new service is ready to be added
         * @param service The service to add
         */
        void addService( ServiceBase * service );

        void serviceUpdated( ServiceBase * service );

    public slots:

        /**
         * Initialzises a new service. This method is exported to DBUS
         * @param name the name of the service to create. Mujst be the same as the name of the script
         * @param levels How many levels of items should be added ( 1 - 4 )
         * @param rootHtml The html to display when the service is selected
         * @return returns true if successful and false otherwise
         */
        bool initService( const QString &name, int levels, const QString &shortDescription, const QString &rootHtml, bool showSearchBar );

        /**
         * Add a new item to a service
         * @param serviceName The name of the service to add an item to
         * @param level The level of this item
         * @param parentId The id of the parent item that this item should be a child of ( -1 if no parent )
         * @param name The name of this item
         * @param infoHtml The html info to display when this item is selected
         * @param callbackData The callback data needed to let the script know how to populate this items children ( Empty string if leaf item )
         * @param playableUrl The url to play if added to the playlist ( Empty string if not leaf node )
         * @return the id of the created item ( or -1 on failure )
         */
        int insertItem( const QString &serviceName, int level, int parentId, const QString &name, const QString &infoHtml, const QString &callbackData, const QString &playableUrl,
                    const QString & albumOverride, const QString & artistOverride, const QString & genreOverride,
                    const QString & composerOverride, int yearOverride, const QString &coverUrl );

        
        /**
         * Let the service know that the script has added all child nodes
         * @param serviceName The service we have been adding items to
         * @param parentId The id of the parent node
         */
        void donePopulating( const QString &serviceName, int parentId );

        void setIcon( const QString &serviceName, const QPixmap &icon );

        void setEmblem( const QString &serviceName, const QPixmap &emblem );

        void setCurrentInfo( const QString &serviceName, const QString &info );


    private:

        /**
        * Constructor
        */
        ScriptableServiceManager();

        /**
        * Destructor
        */
        ~ScriptableServiceManager();

        static ScriptableServiceManager * s_instance;

        QMap<QString, ScriptableService *> m_serviceMap;
        QString m_rootHtml;
};

#endif

