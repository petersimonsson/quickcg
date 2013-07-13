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

#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QPointer>

class Server;

class ClientConnection : public QObject
{
    Q_OBJECT
public:
    explicit ClientConnection(QTcpSocket *socket, Server *parent = 0);

    void sendGraphicAdded(const QString &graphic);
    void sendGraphicRemoved(const QString &graphic);

    void sendGraphicStateChanged(const QString &graphic, bool state);

    void sendShowList();

protected slots:
    void readFromSocket();

    void parseListGraphics(const QVariant &data);
    void parseToggleState(const QVariant &data);
    void parseListTemplates(const QVariant &data);
    void parseCreateGraphic(const QVariant &data);
    void parseGetProperties(const QVariant &data);
    void parseSetGraphicProperties(const QVariant &data);
    void parseRemoveGraphic(const QVariant &data);

    void parseListShows(const QVariant &data);
    void parseCreateShow(const QVariant &data);
    void parseChangeCurrentShow(const QVariant &data);
    void parseRemoveShow(const QVariant &data);

protected:
    void parseCommand(const QVariant &result);
    void sendCommand(const QString &command, const QVariant &data = QVariant());

    void initCommandHash();

private:
    QPointer<QTcpSocket> m_socket;
    Server *m_server;

    QHash<QString, QByteArray> m_commandHash;
};

#endif // CLIENTCONNECTION_H
