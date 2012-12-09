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

class ServerConnection;
class QStringListModel;
class QModelIndex;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    virtual bool eventFilter(QObject *watched, QEvent *event);

protected slots:
    void connectToServer();

    void showConnectionErrorDialog(const QString &message);
    void onConnected();
    void onDisconnected();

    void updateGraphicList(const QStringList &list);
    void toggleGraphicOnAir(const QModelIndex &index);

    void createGraphic(const QStringList &templates);

    void onEditGraphic();
    void editGraphicProperties(const QString &graphic, bool onAirTimerEnabled, int onAirTimerInterval, const QList<QPair<QString, QVariant> > &properties);

    void onRemoveGraphic();

    void addGraphic(const QString &graphic);
    void removeGraphic(const QString &graphic);

    void updateShowList(const QStringList &list, const QString &current);
    void onNewShow();
    void onCurrentShowChanged(const QString &showName);
    void onRemoveShow();

    void showGraphicContextMenu(const QPoint &pos);

private:
    Ui::MainWindow *ui;

    ServerConnection *m_connection;

    QStringListModel *m_graphicModel;
};

#endif // MAINWINDOW_H
