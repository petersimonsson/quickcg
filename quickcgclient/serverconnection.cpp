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

#include "serverconnection.h"

#include <qjson/serializer.h>
#include <qjson/parser.h>

#include <QStringList>

ServerConnection::ServerConnection(QObject *parent) :
    QObject(parent)
{
    initCommandHash();

    m_socket = new QTcpSocket(this);

    connect(m_socket, SIGNAL(connected()), this, SIGNAL(connected()));
    connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(handleError(QAbstractSocket::SocketError)));

    connect(m_socket, SIGNAL(readyRead()),
            this, SLOT(readFromSocket()));
}

void ServerConnection::connectToServer(const QString &address, quint16 port)
{
    m_socket->connectToHost(address, port);
}

void ServerConnection::disconnectFromServer()
{
    m_socket->disconnectFromHost();
}

void ServerConnection::handleError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)

    emit error(m_socket->errorString());
}

void ServerConnection::readFromSocket()
{
    while (m_socket->canReadLine())
    {
        QByteArray command = m_socket->readLine();

        QJson::Parser parser;
        bool ok = false;

        QVariant result = parser.parse(command, &ok);

        if(!ok)
        {
            qDebug() << "JSON parser error:" << parser.errorString();
            return;
        }

        parseCommand(result);
    }
}

void ServerConnection::parseCommand(const QVariant &result)
{
    QVariantMap commandMap = result.toMap();

    if(!commandMap.contains("Command"))
    {
        qDebug() << "Data received was not a valid command";
        return;
    }

    QByteArray command = m_commandHash.value(commandMap.value("Command").toString());
    QVariant data = commandMap.value("Data");
    QMetaObject::invokeMethod(this, command, Q_ARG(QVariant, data));
}

void ServerConnection::sendCommand(const QString &command, const QVariant &data)
{
    QVariantMap commandMap;
    commandMap.insert("Command", command);

    if(data.isValid())
    {
        commandMap.insert("Data", data);
    }

    bool ok = false;
    QJson::Serializer serializer;
    QByteArray commandarray = serializer.serialize(commandMap, &ok);

    if (ok)
    {
        commandarray.append("\r\n");
        m_socket->write(commandarray);
    }
    else
    {
        qDebug() << "Error serializing command";
    }
}

void ServerConnection::fetchGraphicList()
{
    sendCommand("list graphics");
}

void ServerConnection::toggleGraphicOnAir(const QString &name)
{
    sendCommand("toggle state", name);
}

void ServerConnection::initCommandHash()
{
    m_commandHash.insert("graphics", "parseGraphics");
    m_commandHash.insert("templates", "parseTemplates");
    m_commandHash.insert("graphic properties", "parseGraphicProperties");
    m_commandHash.insert("graphic added", "parseGraphicAdded");
    m_commandHash.insert("graphic removed", "parseGraphicRemoved");
    m_commandHash.insert("shows", "parseShows");
}

void ServerConnection::parseGraphics(const QVariant &data)
{
    QStringList list = data.toStringList();
    emit graphicListChanged(list);
}

void ServerConnection::parseTemplates(const QVariant &data)
{
    QStringList list = data.toStringList();
    emit templateListReceived(list);
}

void ServerConnection::fetchTemplateList()
{
    sendCommand("list templates");
}

void ServerConnection::createGraphic(const QString &name, const QString &templateName)
{
    QVariantMap data;
    data.insert("Name", name);
    data.insert("Template", templateName);

    sendCommand("create graphic", data);
}

void ServerConnection::getProperties(const QString &graphic)
{
    sendCommand("get properties", graphic);
}

void ServerConnection::parseGraphicProperties(const QVariant &data)
{
    QVariantMap dataMap = data.toMap();
    QString graphic = dataMap.value("Name").toString();
    bool timerEnabled = dataMap.value("OnAirTimerEnabled", false).toBool();
    int timerInterval = dataMap.value("OnAirTimerInterval", 10000).toInt();
    QVariantList list = dataMap.value("Properties").toList();
    QList<QPair<QString, QVariant> > propertyList;

    foreach(const QVariant &property, list)
    {
        QVariantMap propertyMap = property.toMap();
        propertyList.append(QPair<QString, QVariant>(propertyMap.value("Name").toString(), propertyMap.value("Value")));
    }

    emit graphicPropertiesReceived(graphic, timerEnabled, timerInterval, propertyList);
}

void ServerConnection::setGraphicProperties(const QString &graphic, bool onAirTimerEnabled, int onAirTimerInterval, const QList<QPair<QString, QVariant> > &properties)
{
    QVariantMap commanddata;
    commanddata.insert("Name", graphic);
    commanddata.insert("OnAirTimerEnabled", onAirTimerEnabled);
    commanddata.insert("OnAirTimerInterval", onAirTimerInterval);

    QVariantList list;

    for(int i = 0; i < properties.count(); ++i)
    {
        QVariantMap property;
        property.insert("Name", properties.at(i).first);
        property.insert("Value", properties.at(i).second);
        list.append(property);
    }

    commanddata.insert("Properties", list);

    sendCommand("set graphic properties", commanddata);
}

void ServerConnection::removeGraphic(const QString &name)
{
    sendCommand("remove graphic", name);
}

void ServerConnection::parseGraphicAdded(const QVariant &data)
{
    QString graphic = data.toString();

    emit graphicAdded(graphic);
}

void ServerConnection::parseGraphicRemoved(const QVariant &data)
{
    QString graphic = data.toString();

    emit graphicRemoved(graphic);
}

void ServerConnection::fetchShowList()
{
    sendCommand("list shows");
}

void ServerConnection::parseShows(const QVariant &data)
{
    QVariantMap dataMap = data.toMap();
    QStringList shows = dataMap.value("shows").toStringList();
    QString current = dataMap.value("current").toString();

    if(current != m_currentShow)
    {
        fetchGraphicList();
        m_currentShow = current;
    }

    emit showListReceived(shows, current);
}

void ServerConnection::createNewShow(const QString &name)
{
    sendCommand("create show", name);
}

void ServerConnection::changeCurrentShow(const QString &name)
{
    sendCommand("change current show", name);
}

void ServerConnection::removeCurrentShow()
{
    if(m_currentShow.isEmpty())
    {
        return;
    }

    sendCommand("remove show", m_currentShow);
}
