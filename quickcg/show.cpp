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

#include "show.h"
#include "mainwindow.h"

#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDebug>
#include <QUrl>
#include <QDir>
#include <QDeclarativeComponent>
#include <QFileInfo>

Show::Show(MainWindow *mainWindow) :
    QObject(mainWindow), m_mainWindow(mainWindow)
{
}

Show::~Show()
{
    qDeleteAll(m_graphicHash);
}

void Show::load(const QString &path)
{
    QFile file(path);

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Error opening show file:" << file.errorString();
        return;
    }

    m_showPath = path;

    QDomDocument doc;
    QString errorString;
    int errorLine = 0;
    int errorColumn = 0;

    if(!doc.setContent(&file, &errorString, &errorLine, &errorColumn))
    {
        qDebug() << "Failed parsing the show file at line" << errorLine << "column" << errorColumn << ":" << errorString;
        file.close();
        return;
    }

    file.close();

    QDomElement element = doc.documentElement();

    if(element.tagName() != "QuickCGShow")
    {
        qDebug() << "File is not a QuickCG show file";
    }

    element = element.firstChildElement();

    while(!element.isNull())
    {
        if(element.tagName() == "Graphic")
        {
            loadGraphic(element);
        }

        element = element.nextSiblingElement();
    }
}

void Show::loadGraphic(const QDomElement &element)
{
    QString templateFile = element.attribute("template");

    if(templateFile.isEmpty())
    {
        qDebug() << "Failed to load graphic due to missing template attribute";
        return;
    }

    QString name = element.attribute("name");

    if(name.isEmpty())
    {
        qDebug() << "Failed to load graphic due to missing name attribute";
        return;
    }

    Graphic *graphic = createGraphic(name, templateFile);

    if(!graphic)
    {
        return;
    }

    graphic->setOnAirTimerEnabled(element.attribute("onairtimerenabled", "false").toLower() == "true");
    graphic->setOnAirTimerInterval(element.attribute("onairtimerinterval", "10000").toInt());

    QDomElement child = element.firstChildElement();

    while(!child.isNull())
    {
        if(child.tagName() == "Property")
        {
            QString propertyName = child.attribute("name");

            if(!propertyName.isEmpty())
            {
                graphic->setGraphicsProperty(propertyName.toLocal8Bit(), child.attribute("value"));
            }
        }

        child = child.nextSiblingElement();
    }
}

void Show::setGraphicOnAir(const QString &name, bool state)
{
    Graphic *graphic = m_graphicHash.value(name);

    if(graphic)
    {
        graphic->setOnAir(state);
    }
}

bool Show::isGraphicOnAir(const QString &name) const
{
    Graphic *graphic = m_graphicHash.value(name);

    if(graphic)
    {
        return graphic->isOnAir();
    }

    return false;
}

Graphic *Show::createGraphic(const QString &name, const QString &templateName)
{
    if(name.isEmpty())
    {
        qDebug() << "Failed to create graphic due to missing name attribute";
        return 0;
    }
    else if(m_graphicHash.contains(name))
    {
        qDebug() << "This show already contains a graphic called" << name << ", skipping";
        return 0;
    }

    Graphic *graphic = new Graphic(name);
    m_graphicHash.insert(name, graphic);

    connect(graphic, SIGNAL(itemCreated(QDeclarativeItem*)), m_mainWindow, SLOT(addItem(QDeclarativeItem*)));

    graphic->setPropertyNames(parseGraphicProperties(templateName));
    graphic->setComponent(m_mainWindow->loadTemplate(templateName));

    return graphic;
}

QList<QPair<QString, QVariant> > Show::graphicProperties(const QString &name) const
{
    Graphic *graphic = m_graphicHash.value(name);

    if(graphic)
    {
        return graphic->properties();
    }

    return QList<QPair<QString, QVariant> >();
}

QStringList Show::parseGraphicProperties(const QString &name)
{
    QFile file(m_mainWindow->templateDir().absoluteFilePath(name));

    if(!file.exists())
    {
        return QStringList();
    }

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open" << file.fileName() << "with the following error:" << file.errorString();
        return QStringList();
    }

    QRegExp regexp("\\b(qcg[A-Za-z0-9]*)\\b");
    QTextStream stream(&file);
    QStringList list;

    while(!stream.atEnd())
    {
        QString line = stream.readLine();
        int pos = 0;

        while((pos = regexp.indexIn(line, pos)) != -1)
        {
            if(!list.contains(regexp.cap(1)))
            {
                list << regexp.cap(1);
            }

            pos += regexp.matchedLength();
        }
    }

    return list;
}

void Show::setGraphicProperties(const QString &name, const QList<QPair<QString, QVariant> > &properties)
{
    Graphic* graphic = m_graphicHash.value(name);

    if(graphic)
    {
        for(int i = 0; i < properties.count(); ++i)
        {
            graphic->setGraphicsProperty(properties.at(i).first.toLocal8Bit(), properties.at(i).second);
        }
    }
}

bool Show::graphicOnAirTimerEnabled(const QString &name) const
{
    Graphic* graphic = m_graphicHash.value(name);

    if(graphic)
    {
        return graphic->onAirTimerEnabled();
    }

    return false;
}

int Show::graphicOnAirTimerInterval(const QString &name) const
{
    Graphic* graphic = m_graphicHash.value(name);

    if(graphic)
    {
        return graphic->onAirTimerInterval();
    }

    return 0;
}

void Show::setGraphicOnAirTimerEnabled(const QString &name, bool enabled)
{
    Graphic* graphic = m_graphicHash.value(name);

    if(graphic)
    {
        graphic->setOnAirTimerEnabled(enabled);
    }
}

void Show::setGraphicOnAirTimerInterval(const QString &name, int interval)
{
    Graphic* graphic = m_graphicHash.value(name);

    if(graphic)
    {
        graphic->setOnAirTimerInterval(interval);
    }
}

void Show::save()
{
    if(m_showPath.isEmpty())
    {
        return;
    }

    QFile file(m_showPath);

    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open" << m_showPath << "for writing with the following error:" << file.errorString();
        return;
    }

    QDomDocument doc;
    QDomElement rootElement = doc.createElement("QuickCGShow");
    doc.appendChild(rootElement);

    foreach(Graphic* graphic, m_graphicHash)
    {
        QDomElement graphicElement = doc.createElement("Graphic");
        graphicElement.setAttribute("name", graphic->name());
        QFileInfo info (graphic->component()->url().path());
        graphicElement.setAttribute("template", info.fileName());
        graphicElement.setAttribute("onairtimerenabled", graphic->onAirTimerEnabled() ? "true" : "false");
        graphicElement.setAttribute("onairtimerinterval", graphic->onAirTimerInterval());

        QList<QPair<QString, QVariant> > list = graphic->properties();

        for(int i = 0; i < list.count(); ++i)
        {
            QDomElement propertyElement = doc.createElement("Property");
            propertyElement.setAttribute("name", list.at(i).first);
            propertyElement.setAttribute("value", list.at(i).second.toString());
            graphicElement.appendChild(propertyElement);
        }

        rootElement.appendChild(graphicElement);
    }

    file.write(doc.toString(4).toUtf8());
}

void Show::removeGraphic(const QString &name)
{
    Graphic* graphic = m_graphicHash.value(name);

    if(graphic)
    {
        m_graphicHash.remove(name);
        delete graphic;
    }
}

QString Show::showName() const
{
    QFileInfo info;
    info.setFile(m_showPath);

    return info.fileName();
}
