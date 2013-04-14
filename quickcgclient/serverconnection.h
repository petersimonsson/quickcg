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

#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <QObject>
#include <QTcpSocket>

class ServerConnection : public QObject
{
    Q_OBJECT
public:
    explicit ServerConnection(QObject *parent = 0);

    void fetchGraphicList();
    void toggleGraphicOnAir(const QString &name);

public slots:
    void connectToServer(const QString &address, quint16 port);
    void disconnectFromServer();

    void fetchTemplateList();
    void createGraphic(const QString &name, const QString &templateName);
    void getProperties(const QString &graphic);
    void setGraphicProperties(const QString &graphic, bool onAirTimerEnabled, int onAirTimerInterval,
                              const QString& group, const QList<QPair<QString, QVariant> > &properties);
    void removeGraphic(const QString &name);

    void fetchShowList();
    void createNewShow(const QString &name);
    void changeCurrentShow(const QString &name);
    void removeCurrentShow();

protected slots:
    void handleError(QAbstractSocket::SocketError socketError);

    void readFromSocket();

    void parseGraphics(const QVariant &data);
    void parseTemplates(const QVariant &data);
    void parseGraphicProperties(const QVariant &data);
    void parseGraphicAdded(const QVariant &data);
    void parseGraphicRemoved(const QVariant &data);

    void parseShows(const QVariant &data);

protected:
    void parseCommand(const QVariant &result);
    void sendCommand(const QString &command, const QVariant &data = QVariant());

    void initCommandHash();

private:
    QTcpSocket *m_socket;

    QHash<QString, QByteArray> m_commandHash;

    QString m_currentShow;

signals:
    void connected();
    void disconnected();
    void error(const QString &message);

    void graphicListChanged(const QStringList &list);
    void templateListReceived(const QStringList &list);
    void graphicPropertiesReceived(const QString &graphic, bool onAirTimerEnabled, int onAirTimerInterval,
                                   const QString& group, const QList<QPair<QString, QVariant> > &propertyList);
    void graphicAdded(const QString &graphic);
    void graphicRemoved(const QString &graphic);

    void showListReceived(const QStringList &list, const QString &current);
};

#endif // SERVERCONNECTION_H
