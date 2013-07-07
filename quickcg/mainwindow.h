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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>

namespace Ui {
    class MainWindow;
}

class QUrl;
class QDeclarativeItem;
class QDeclarativeComponent;
class QGraphicsRectItem;
class Show;
class Server;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QDeclarativeComponent* loadTemplate(const QString &name);

    Show* currentShow() const { return m_show; }

    QStringList templates() const;
    QDir templateDir() const { return m_templateDir; }

    QStringList shows() const;
    QDir showDir() const { return m_showDir; }

    void createShow(const QString &name);
    void removeShow(const QString &name);

    void removeAddressInfo();

public slots:
    void addItem(QDeclarativeItem* item);

    void setCurrentShow(const QString& show);

protected slots:
    void toggleFullscreen();

    void quit();

protected:
    void initDirs();

private:
    Ui::MainWindow *ui;

    Show *m_show;
    Server *m_server;

    QDir m_templateDir;
    QDir m_showDir;

    QGraphicsRectItem *m_addressInfoItem;
};

#endif // MAINWINDOW_H
