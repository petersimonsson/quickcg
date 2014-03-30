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
#include <QJsonValue>

class Server;
class QJsonDocument;

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

    void parseListGraphics(const QJsonValue &data);
    void parseToggleState(const QJsonValue &data);
    void parseListTemplates(const QJsonValue &data);
    void parseCreateGraphic(const QJsonValue &data);
    void parseGetProperties(const QJsonValue &data);
    void parseSetGraphicProperties(const QJsonValue &data);
    void parseRemoveGraphic(const QJsonValue &data);

    void parseListShows(const QJsonValue &data);
    void parseCreateShow(const QJsonValue &data);
    void parseChangeCurrentShow(const QJsonValue &data);
    void parseRemoveShow(const QJsonValue &data);

protected:
    void parseCommand(const QJsonDocument &jsonDoc);
    void sendCommand(const QString &command, const QJsonValue &data = QJsonValue ());

    void initCommandHash();

private:
    QPointer<QTcpSocket> m_socket;
    Server *m_server;

    QHash<QString, QByteArray> m_commandHash;
};

#endif // CLIENTCONNECTION_H
