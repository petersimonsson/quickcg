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

#include "server.h"
#include "mainwindow.h"

#include <QTcpServer>
#include <QTcpSocket>

Server::Server(MainWindow *parent) :
    QObject(parent), m_mainWindow(parent)
{
    m_serverSocket = new QTcpServer(this);

    m_serverSocket->listen(QHostAddress::Any, 31337);

    connect(m_serverSocket, SIGNAL(newConnection()),
            this, SLOT(createClientConnection()));
}

void Server::createClientConnection()
{
    while(m_serverSocket->hasPendingConnections())
    {
        QTcpSocket* socket = m_serverSocket->nextPendingConnection();

        QPointer<ClientConnection> connection = new ClientConnection(socket, this);
        m_connections.append(connection);

        m_mainWindow->removeAddressInfo();
    }
}

void Server::sendGraphicAdded(const QString& graphic)
{
    for (int i = 0; i < m_connections.count(); ++i)
    {
        if(m_connections[i])
        {
            m_connections[i]->sendGraphicAdded(graphic);
        }
    }
}

void Server::sendGraphicRemoved(const QString& graphic)
{
    for (int i = 0; i < m_connections.count(); ++i)
    {
        if(m_connections[i])
        {
            m_connections[i]->sendGraphicRemoved(graphic);
        }
    }
}

void Server::sendShowList()
{
    for(int i = 0; i < m_connections.count(); ++i)
    {
        if(m_connections[i])
        {
            m_connections[i]->sendShowList();
        }
    }
}

void Server::sendGraphicStateChanged(const QString &graphic, bool state)
{
    for(int i = 0; i < m_connections.count(); ++i)
    {
        if(m_connections[i])
        {
            m_connections[i]->sendGraphicStateChanged(graphic, state);
        }
    }
}
