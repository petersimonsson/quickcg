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

#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(command, &parseError);

        if(jsonDoc.isNull())
        {
            qDebug() << "JSON parser error:" << parseError.errorString();
            return;
        }

        parseCommand(jsonDoc);
    }
}

void ServerConnection::parseCommand(const QJsonDocument &jsonDoc)
{
    if (!jsonDoc.isObject())
    {
        qDebug () << "Command is not a JSON object";
        return;
    }

    QJsonObject object = jsonDoc.object();

    if(!object.contains("Command"))
    {
        qDebug() << "Data received was not a valid command";
        return;
    }

    QByteArray command = m_commandHash.value(object.value("Command").toString());
    QJsonValue data = object.value("Data");
    QMetaObject::invokeMethod(this, command, Q_ARG(QJsonValue, data));
}

void ServerConnection::sendCommand(const QString &command, const QJsonValue &data)
{
    QJsonObject object;
    object.insert("Command", command);

    if(!data.isNull())
    {
        object.insert("Data", data);
    }

    QJsonDocument jsonDoc(object);
    QByteArray commandarray = jsonDoc.toJson(QJsonDocument::Compact);
    commandarray.append("\r\n");

    m_socket->write(commandarray);
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
    m_commandHash.insert("graphic state changed", "parseGraphicStateChanged");
}

void ServerConnection::parseGraphics(const QJsonValue &data)
{
    QStringList list;

    foreach (const QJsonValue &value, data.toArray())
    {
        list.append(value.toString());
    }

    emit graphicListChanged(list);
}

void ServerConnection::parseTemplates(const QJsonValue &data)
{
    QStringList list;

    foreach (const QJsonValue &value, data.toArray())
    {
        list.append(value.toString());
    }

    emit templateListReceived(list);
}

void ServerConnection::fetchTemplateList()
{
    sendCommand("list templates");
}

void ServerConnection::createGraphic(const QString &name, const QString &templateName)
{
    QJsonObject object;
    object.insert("Name", name);
    object.insert("Template", templateName);

    sendCommand("create graphic", object);
}

void ServerConnection::getProperties(const QString &graphic)
{
    sendCommand("get properties", graphic);
}

void ServerConnection::parseGraphicProperties(const QJsonValue &data)
{
    QJsonObject object = data.toObject();
    QString graphic = object.value("Name").toString();
    bool timerEnabled = object.value("OnAirTimerEnabled").toBool(false);
    int timerInterval = object.value("OnAirTimerInterval").toInt(10000);
    QString group = object.value("Group").toString();
    QJsonArray array = object.value("Properties").toArray();
    QList<QPair<QString, QVariant> > propertyList;

    foreach(const QJsonValue &property, array)
    {
        QJsonObject propertyObject = property.toObject();
        propertyList.append(QPair<QString, QVariant>(propertyObject.value("Name").toString(), propertyObject.value("Value")));
    }

    emit graphicPropertiesReceived(graphic, timerEnabled, timerInterval, group, propertyList);
}

void ServerConnection::setGraphicProperties(const QString &graphic, bool onAirTimerEnabled, int onAirTimerInterval,
                                            const QString& group, const QList<QPair<QString, QVariant> > &properties)
{
    QJsonObject object;
    object.insert("Name", graphic);
    object.insert("OnAirTimerEnabled", onAirTimerEnabled);
    object.insert("OnAirTimerInterval", onAirTimerInterval);
    object.insert("Group", group);

    QJsonArray array;

    for(int i = 0; i < properties.count(); ++i)
    {
        QJsonObject propertyObject;
        propertyObject.insert("Name", properties.at(i).first);
        propertyObject.insert("Value", QJsonValue::fromVariant(properties.at(i).second));
        array.append(propertyObject);
    }

    object.insert("Properties", array);

    sendCommand("set graphic properties", object);
}

void ServerConnection::removeGraphic(const QString &name)
{
    sendCommand("remove graphic", name);
}

void ServerConnection::parseGraphicAdded(const QJsonValue &data)
{
    QString graphic = data.toString();

    emit graphicAdded(graphic);
}

void ServerConnection::parseGraphicRemoved(const QJsonValue &data)
{
    QString graphic = data.toString();

    emit graphicRemoved(graphic);
}

void ServerConnection::fetchShowList()
{
    sendCommand("list shows");
}

void ServerConnection::parseShows(const QJsonValue &data)
{
    QJsonObject object = data.toObject();
    QStringList shows;

    foreach (const QJsonValue &value, object.value("shows").toArray())
    {
        shows.append(value.toString());
    }

    QString current = object.value("current").toString();

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

void ServerConnection::parseGraphicStateChanged(const QJsonValue &data)
{
    QJsonObject object = data.toObject();
    QString graphic = object.value("graphic").toString();
    bool state = object.value("state").toBool();

    qDebug() << "State change in" << graphic << "to" << state;
}
