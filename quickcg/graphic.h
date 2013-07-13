#ifndef GRAPHIC_H
#define GRAPHIC_H

#include <QObject>
#include <QPair>
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

#include <QStringList>
#include <QTimer>

class QDeclarativeComponent;
class QDeclarativeItem;

class Graphic : public QObject
{
    Q_OBJECT
public:
    explicit Graphic(const QString &name, QObject *parent = 0);
    virtual ~Graphic();

    QString name() const { return m_name; }
    bool isValid() const { return m_component != 0; }

    void setComponent(QDeclarativeComponent *component);
    QDeclarativeComponent *component() const { return m_component; }

    void setPropertyNames(const QStringList &list) { m_propertyNames = list; }
    void setGraphicsProperty(const QByteArray &name, const QVariant &value);
    QList<QPair<QString, QVariant> > properties() const;

    bool isOnAir() const;

    void setOnAirTimerEnabled(bool enabled);
    bool onAirTimerEnabled() const { return m_onAirTimerEnabled; }
    void setOnAirTimerInterval(int ms);
    int onAirTimerInterval() const { return m_onAirTimer->interval(); }

    void setGroup(const QString& name) { m_group = name; }
    QString group() const { return m_group; }

public slots:
    void toggleOnAir();
    void setOnAir(bool state);

protected slots:
    void createItem();

private:
    QString m_name;
    QString m_group;

    QDeclarativeComponent *m_component;
    QDeclarativeItem *m_item;

    QList<QPair<QString, QVariant> > m_tempPropertyList;
    QStringList m_propertyNames;

    QTimer *m_onAirTimer;
    bool m_onAirTimerEnabled;

signals:
    void itemCreated(QDeclarativeItem *item);

    void stateChanged(const QString &name, bool state);
};

#endif // GRAPHIC_H
