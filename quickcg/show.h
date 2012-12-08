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

#ifndef SHOW_H
#define SHOW_H

#include <graphic.h>

#include <QObject>
#include <QHash>
#include <QStringList>

class QUrl;
class QDomElement;
class MainWindow;

class Show : public QObject
{
    Q_OBJECT
public:
    explicit Show(MainWindow* mainWindow);
    ~Show();

    void load(const QString &path);
    void save();

    QStringList graphics() const { return m_graphicHash.keys(); }

    bool isGraphicOnAir(const QString &name) const;

    Graphic *createGraphic(const QString &name, const QString &templateName);

    QList<QPair<QString, QVariant> > graphicProperties(const QString &name) const;
    void setGraphicProperties(const QString &name, const QList<QPair<QString, QVariant> > &properties);

    bool graphicOnAirTimerEnabled(const QString &name) const;
    int graphicOnAirTimerInterval(const QString &name) const;
    void setGraphicOnAirTimerEnabled(const QString &name, bool enabled);
    void setGraphicOnAirTimerInterval(const QString &name, int interval);

    void removeGraphic(const QString &name);

    QString showName() const;
    QString showPath() const { return m_showPath; }

public slots:
    void setGraphicOnAir(const QString &name, bool state);

protected:
    void loadGraphic(const QDomElement &element);
    QStringList parseGraphicProperties(const QString &name);

private:
    QHash<QString, Graphic*> m_graphicHash;
    QString m_showPath;

    MainWindow *m_mainWindow;
};

#endif //SHOW_H
