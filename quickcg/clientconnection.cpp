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

#include <qjson/parser.h>
#include <qjson/serializer.h>

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

void ClientConnection::parseCommand(const QVariant &result)
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

void ClientConnection::sendCommand(const QString &command, const QVariant &data)
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

void ClientConnection::parseListGraphics(const QVariant& data)
{
    Q_UNUSED(data)

    if(!m_server->mainWindow()->currentShow())
    {
        return;
    }

    QStringList graphics = m_server->mainWindow()->currentShow()->graphics();

    sendCommand("graphics", graphics);
}

void ClientConnection::parseToggleState(const QVariant& data)
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
    QVariantMap map;
    map.insert("graphic", graphic);
    map.insert("state", state);

    sendCommand("graphic state changed", map);
}

void ClientConnection::parseListTemplates(const QVariant& data)
{
    Q_UNUSED(data)

    QStringList templates = m_server->mainWindow()->templates();

    sendCommand("templates", templates);
}

void ClientConnection::parseCreateGraphic(const QVariant &data)
{
    if(!m_server->mainWindow()->currentShow())
    {
        return;
    }

    QVariantMap map = data.toMap();
    QString name = map.value("Name").toString();
    QString templateName = map.value("Template").toString();

    if(name.isEmpty() || templateName.isEmpty())
    {
        qDebug() << "Invalid create graphic command.";
        return;
    }

    m_server->mainWindow()->currentShow()->createGraphic(name, templateName);
    m_server->sendGraphicAdded(name);
}

void ClientConnection::parseGetProperties(const QVariant &data)
{
    if(!m_server->mainWindow()->currentShow())
    {
        return;
    }

    QString graphicName = data.toString();
    Graphic* graphic = m_server->mainWindow()->currentShow()->graphicFromName(graphicName);

    if(graphic)
    {
        QVariantMap commanddata;
        commanddata.insert("Name", graphicName);
        commanddata.insert("OnAirTimerEnabled", graphic->onAirTimerEnabled());
        commanddata.insert("OnAirTimerInterval", graphic->onAirTimerInterval());
        commanddata.insert("Group", graphic->group());

        QList<QPair<QString, QVariant> > propertyList = graphic->properties();
        QVariantList list;

        for(int i = 0; i < propertyList.count(); ++i)
        {
            QVariantMap property;
            property.insert("Name", propertyList.at(i).first);
            property.insert("Value", propertyList.at(i).second);
            list.append(property);
        }

        commanddata.insert("Properties", list);

        sendCommand("graphic properties", commanddata);
    }
}

void ClientConnection::parseSetGraphicProperties(const QVariant &data)
{
    if(!m_server->mainWindow()->currentShow())
    {
        return;
    }

    QVariantMap dataMap = data.toMap();
    QString graphicName = dataMap.value("Name").toString();
    Graphic* graphic = m_server->mainWindow()->currentShow()->graphicFromName(graphicName);

    if(graphic)
    {
        graphic->setOnAirTimerEnabled(dataMap.value("OnAirTimerEnabled", false).toBool());
        graphic->setOnAirTimerInterval(dataMap.value("OnAirTimerInterval", 10000).toInt());
        graphic->setGroup(dataMap.value("Group").toString());
        QVariantList list = dataMap.value("Properties").toList();

        foreach(const QVariant &property, list)
        {
            QVariantMap propertyMap = property.toMap();
            graphic->setGraphicsProperty(propertyMap.value("Name").toString().toLocal8Bit(), propertyMap.value("Value"));
        }
    }
}

void ClientConnection::parseRemoveGraphic(const QVariant &data)
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

void ClientConnection::parseListShows(const QVariant &data)
{
    Q_UNUSED(data)

    sendShowList();
}

void ClientConnection::parseCreateShow(const QVariant &data)
{
    QString showName = data.toString();
    m_server->mainWindow()->createShow(showName);
}

void ClientConnection::sendShowList()
{
    QVariantMap dataMap;

    if(m_server->mainWindow()->currentShow())
    {
        dataMap.insert("current", m_server->mainWindow()->currentShow()->showName());
    }

    QStringList shows = m_server->mainWindow()->shows();
    dataMap.insert("shows", shows);

    sendCommand("shows", dataMap);
}

void ClientConnection::parseChangeCurrentShow(const QVariant &data)
{
    QString showName = data.toString();
    m_server->mainWindow()->setCurrentShow(showName);
}

void ClientConnection::parseRemoveShow(const QVariant &data)
{
    QString showName = data.toString();
    m_server->mainWindow()->removeShow(showName);
}
