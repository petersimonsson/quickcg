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

#include "clientconnection.h"
#include "server.h"
#include "mainwindow.h"
#include "show.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ClientConnection::ClientConnection(QTcpSocket *socket, Server *parent) :
    QObject(parent), m_socket (socket), m_server(parent)
{
    initCommandHash();

    connect(m_socket, SIGNAL(readyRead()),
            this, SLOT(readFromSocket()));
    connect(m_socket, SIGNAL(disconnected()),
            this, SLOT(deleteLater()));
    connect(m_socket, SIGNAL(disconnected()),
            m_socket, SLOT(deleteLater()));
}

void ClientConnection::readFromSocket()
{
    while(m_socket->canReadLine())
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

void ClientConnection::parseCommand(const QJsonDocument &jsonDoc)
{
    if (!jsonDoc.isObject())
    {
        qDebug () << "Command is not a JSON object";
        return;
    }

    QJsonObject jsonObject = jsonDoc.object();

    if(!jsonObject.contains("Command"))
    {
        qDebug() << "Data received was not a valid command";
        return;
    }

    QByteArray command = m_commandHash.value(jsonObject.value("Command").toString());
    QJsonValue data = jsonObject.value("Data");
    QMetaObject::invokeMethod(this, command, Q_ARG(QJsonValue, data));
}

void ClientConnection::sendCommand(const QString &command, const QJsonValue &data)
{
    QJsonObject commandObject;
    commandObject.insert("Command", command);

    if(!data.isNull())
    {
        commandObject.insert("Data", data);
    }

    QJsonDocument jsonDoc(commandObject);
    QByteArray commandarray = jsonDoc.toJson(QJsonDocument::Compact);
    commandarray.append("\r\n");

    m_socket->write(commandarray);
}

void ClientConnection::initCommandHash()
{
    m_commandHash.insert("list graphics", "parseListGraphics");
    m_commandHash.insert("toggle state", "parseToggleState");
    m_commandHash.insert("list templates", "parseListTemplates");
    m_commandHash.insert("create graphic", "parseCreateGraphic");
    m_commandHash.insert("get properties", "parseGetProperties");
    m_commandHash.insert("set graphic properties", "parseSetGraphicProperties");
    m_commandHash.insert("remove graphic", "parseRemoveGraphic");
    m_commandHash.insert("list shows", "parseListShows");
    m_commandHash.insert("create show", "parseCreateShow");
    m_commandHash.insert("change current show", "parseChangeCurrentShow");
    m_commandHash.insert("remove show", "parseRemoveShow");
}

void ClientConnection::parseListGraphics(const QJsonValue &data)
{
    Q_UNUSED(data)

    if(!m_server->mainWindow()->currentShow())
    {
        return;
    }

    QStringList graphics = m_server->mainWindow()->currentShow()->graphics();

    sendCommand("graphics", QJsonArray::fromStringList(graphics));
}

void ClientConnection::parseToggleState(const QJsonValue &data)
{
    if(!m_server->mainWindow()->currentShow())
    {
        return;
    }

    QString graphic = data.toString();
    m_server->mainWindow()->currentShow()->setGraphicOnAir(graphic, !m_server->mainWindow()->currentShow()->isGraphicOnAir(graphic));
}

void ClientConnection::sendGraphicStateChanged(const QString &graphic, bool state)
{
    QJsonObject object;
    object.insert("graphic", graphic);
    object.insert("state", state);

    sendCommand("graphic state changed", object);
}

void ClientConnection::parseListTemplates(const QJsonValue &data)
{
    Q_UNUSED(data)

    QStringList templates = m_server->mainWindow()->templates();

    sendCommand("templates", QJsonArray::fromStringList(templates));
}

void ClientConnection::parseCreateGraphic(const QJsonValue &data)
{
    if(!m_server->mainWindow()->currentShow())
    {
        return;
    }

    if (!data.isObject())
    {
        qDebug () << "Create Graphic data is not a JSON object";
        return;
    }

    QJsonObject object = data.toObject();

    QString name = object.value("Name").toString();
    QString templateName = object.value("Template").toString();

    if(name.isEmpty() || templateName.isEmpty())
    {
        qDebug() << "Invalid create graphic command.";
        return;
    }

    m_server->mainWindow()->currentShow()->createGraphic(name, templateName);
    m_server->sendGraphicAdded(name);
}

void ClientConnection::parseGetProperties(const QJsonValue &data)
{
    if(!m_server->mainWindow()->currentShow())
    {
        return;
    }

    QString graphicName = data.toString();
    Graphic* graphic = m_server->mainWindow()->currentShow()->graphicFromName(graphicName);

    if(graphic)
    {
        QJsonObject object;
        object.insert("Name", graphicName);
        object.insert("OnAirTimerEnabled", graphic->onAirTimerEnabled());
        object.insert("OnAirTimerInterval", graphic->onAirTimerInterval());
        object.insert("Group", graphic->group());

        QList<QPair<QString, QVariant> > propertyList = graphic->properties();
        QJsonArray array;

        for(int i = 0; i < propertyList.count(); ++i)
        {
            QJsonObject propertyObject;
            propertyObject.insert("Name", propertyList.at(i).first);
            propertyObject.insert("Value", QJsonValue::fromVariant(propertyList.at(i).second));
            array.append(propertyObject);
        }

        object.insert("Properties", array);

        sendCommand("graphic properties", object);
    }
}

void ClientConnection::parseSetGraphicProperties(const QJsonValue &data)
{
    if(!m_server->mainWindow()->currentShow())
    {
        return;
    }

    if (!data.isObject())
    {
        qDebug () << "Create Graphic data is not a JSON object";
        return;
    }

    QJsonObject object = data.toObject();
    QString graphicName = object.value("Name").toString();
    Graphic* graphic = m_server->mainWindow()->currentShow()->graphicFromName(graphicName);

    if(graphic)
    {
        graphic->setOnAirTimerEnabled(object.value("OnAirTimerEnabled").toBool(false));
        graphic->setOnAirTimerInterval(object.value("OnAirTimerInterval").toInt(10000));
        graphic->setGroup(object.value("Group").toString());
        QJsonArray array = object.value("Properties").toArray();

        foreach(const QJsonValue &property, array)
        {
            QJsonObject propertyObject = property.toObject();
            graphic->setGraphicsProperty(propertyObject.value("Name").toString().toLocal8Bit(), propertyObject.value("Value").toVariant());
        }
    }
}

void ClientConnection::parseRemoveGraphic(const QJsonValue &data)
{
    QString graphic = data.toString();

    if(graphic.isEmpty())
    {
        qDebug() << "Invalid remove graphic command";
        return;
    }

    if(!m_server->mainWindow()->currentShow())
    {
        return;
    }

    m_server->mainWindow()->currentShow()->removeGraphic(graphic);
    m_server->sendGraphicRemoved(graphic);
}

void ClientConnection::sendGraphicAdded(const QString &graphic)
{
    sendCommand("graphic added", graphic);
}

void ClientConnection::sendGraphicRemoved(const QString &graphic)
{
    sendCommand("graphic removed", graphic);
}

void ClientConnection::parseListShows(const QJsonValue &data)
{
    Q_UNUSED(data)

    sendShowList();
}

void ClientConnection::parseCreateShow(const QJsonValue &data)
{
    QString showName = data.toString();
    m_server->mainWindow()->createShow(showName);
}

void ClientConnection::sendShowList()
{
    QJsonObject object;

    if(m_server->mainWindow()->currentShow())
    {
        object.insert("current", m_server->mainWindow()->currentShow()->showName());
    }

    QStringList shows = m_server->mainWindow()->shows();
    object.insert("shows", QJsonArray::fromStringList(shows));

    sendCommand("shows", object);
}

void ClientConnection::parseChangeCurrentShow(const QJsonValue &data)
{
    QString showName = data.toString();
    m_server->mainWindow()->setCurrentShow(showName);
}

void ClientConnection::parseRemoveShow(const QJsonValue &data)
{
    QString showName = data.toString();
    m_server->mainWindow()->removeShow(showName);
}
