// Copyright 2012  Peter Simonsson <peter.simonsson@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SERVER_H
#define SERVER_H

#include "clientconnection.h"

#include <QObject>
#include <QPointer>

class QTcpServer;
class MainWindow;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(MainWindow *parent = 0);

    MainWindow *mainWindow() const { return m_mainWindow; }

    void sendGraphicAdded(const QString& graphic);
    void sendGraphicRemoved(const QString& graphic);

    void sendShowList();

protected slots:
    void createClientConnection();

private:
    QTcpServer *m_serverSocket;

    MainWindow *m_mainWindow;

    QList<QPointer<ClientConnection> > m_connections;
};

#endif // SERVER_H
