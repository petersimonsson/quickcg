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

#include "graphic.h"

#include <QDeclarativeComponent>
#include <QDeclarativeItem>
#include <QDebug>

Graphic::Graphic(const QString &name, QObject *parent) :
    QObject(parent), m_name(name), m_component(0), m_item(0), m_onAirTimerEnabled(false)
{
    m_onAirTimer = new QTimer(this);
    m_onAirTimer->setInterval(10000);
    m_onAirTimer->setSingleShot(true);
    connect(m_onAirTimer, SIGNAL(timeout()),
            this, SLOT(toggleOnAir()));
}

Graphic::~Graphic()
{
    delete m_component;
    delete m_item;
}

void Graphic::setComponent(QDeclarativeComponent *component)
{
    if(!component)
    {
        return;
    }

    m_component = component;

    if(m_component->isReady())
    {
        createItem();
    }
    else
    {
        QObject::connect(m_component, SIGNAL(statusChanged(QDeclarativeComponent::Status)),
                         this, SLOT(createItem()));
    }
}

void Graphic::createItem()
{
    if(!m_component)
    {
        qDebug() << "createItem() failed, graphic" <<  m_name << "has no component!";
        return;
    }

    QObject* object = m_component->create();
    m_item = qobject_cast<QDeclarativeItem*>(object);
    m_item->setProperty("state", "offAir"); // Ensure the item is in the offAir state after it's loaded

    for(int i = 0; i < m_tempPropertyList.count(); ++i)
    {
        m_item->setProperty(m_tempPropertyList[i].first.toLocal8Bit(), m_tempPropertyList[i].second);
    }

    m_tempPropertyList.clear();

    emit itemCreated(m_item);
}

void Graphic::toggleOnAir()
{
    if(!m_item)
    {
        return;
    }

    if(m_item->property("state").toString() != "onAir")
    {
        m_item->setProperty("state", "onAir");

        if(m_onAirTimerEnabled)
        {
            m_onAirTimer->start();
        }
    }
    else
    {
        m_item->setProperty("state", "offAir");
        m_onAirTimer->stop();
    }
}

void Graphic::setOnAir(bool state)
{
    if(state)
    {
        m_item->setProperty("state", "onAir");

        if(m_onAirTimerEnabled)
        {
            m_onAirTimer->start();
        }
    }
    else
    {
        m_item->setProperty("state", "offAir");
        m_onAirTimer->stop();
    }
}

void Graphic::setGraphicsProperty(const QByteArray &name, const QVariant &value)
{
    if(name.isEmpty())
    {
        return;
    }

    QByteArray propertyName = name;

    if(!propertyName.startsWith("qcg"))
    {
        propertyName.prepend("qcg");
    }

    if(!m_item)
    {
        m_tempPropertyList.append(QPair<QString, QVariant>(propertyName, value));
        return;
    }

    m_item->setProperty(propertyName, value);
}

bool Graphic::isOnAir() const
{
    return (m_item->property("state").toString() == "onAir");
}

QList<QPair<QString, QVariant> > Graphic::properties() const
{
    QList<QPair<QString, QVariant> > propertyList;

    foreach(const QString &name, m_propertyNames)
    {
        if(name.startsWith("qcg"))
        {
            propertyList.append(QPair<QString, QVariant>(name, m_item->property(name.toLocal8Bit())));
        }
    }

    return propertyList;
}

void Graphic::setOnAirTimerEnabled(bool enabled)
{
    if(m_onAirTimerEnabled == enabled)
    {
        return;
    }

    m_onAirTimerEnabled = enabled;

    if(!m_onAirTimerEnabled && m_onAirTimer->isActive())
    {
        m_onAirTimer->stop();
    }
    else if(m_onAirTimerEnabled && isOnAir())
    {
        m_onAirTimer->start();
    }
}

void Graphic::setOnAirTimerInterval(int ms)
{
    m_onAirTimer->setInterval(ms);
}
