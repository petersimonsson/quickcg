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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serverconnection.h"
#include "creategraphicdialog.h"
#include "graphicpropertiesdialog.h"

#include <QMessageBox>
#include <QStringListModel>
#include <QInputDialog>
#include <QSettings>
#include <QKeyEvent>
#include <QItemSelectionModel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    onDisconnected(); // Init as disconnected

    m_connection = new ServerConnection(this);

    connect(ui->actionQuit, SIGNAL(triggered()),
            qApp, SLOT(quit()));
    connect(ui->actionConnect, SIGNAL(triggered()),
            this, SLOT(connectToServer()));
    connect(ui->actionDisconnect, SIGNAL(triggered()),
            m_connection, SLOT(disconnectFromServer()));

    connect(ui->actionNewGraphic, SIGNAL(triggered()),
            m_connection, SLOT(fetchTemplateList()));
    connect(m_connection, SIGNAL(templateListReceived(QStringList)),
            this, SLOT(createGraphic(QStringList)));

    connect(ui->actionEditGraphic, SIGNAL(triggered()),
            this, SLOT(onEditGraphic()));
    connect(m_connection, SIGNAL(graphicPropertiesReceived(QString,bool,int,QString,QList<QPair<QString,QVariant> >)),
            this, SLOT(editGraphicProperties(QString,bool,int,QString,QList<QPair<QString,QVariant> >)));

    connect(ui->actionRemoveGraphic, SIGNAL(triggered()),
            this, SLOT(onRemoveGraphic()));

    connect(ui->actionNewShow, SIGNAL(triggered()),
            this, SLOT(onNewShow()));
    connect(ui->actionRemoveShow, SIGNAL(triggered()),
            this, SLOT(onRemoveShow()));

    connect(m_connection, SIGNAL(error(QString)),
            this, SLOT(showConnectionErrorDialog(QString)));
    connect(m_connection, SIGNAL(connected()),
            this, SLOT(onConnected()));
    connect(m_connection, SIGNAL(disconnected()),
            this, SLOT(onDisconnected()));

    connect(m_connection, SIGNAL(graphicListChanged(QStringList)),
            this, SLOT(updateGraphicList(QStringList)));

    m_graphicModel = new QStringListModel(ui->m_graphicTreeView);
    ui->m_graphicTreeView->setModel(m_graphicModel);

    connect(ui->m_graphicTreeView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(toggleGraphicOnAir(QModelIndex)));
    connect (ui->m_graphicTreeView, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(showGraphicContextMenu(QPoint)));

    connect(m_connection, SIGNAL(graphicAdded(QString)),
            this, SLOT(addGraphic(QString)));
    connect(m_connection, SIGNAL(graphicRemoved(QString)),
            this, SLOT(removeGraphic(QString)));

    connect(m_connection, SIGNAL(showListReceived(QStringList,QString)),
            this, SLOT(updateShowList(QStringList,QString)));
    connect(ui->m_showCombo, SIGNAL(activated(QString)),
            this, SLOT(onCurrentShowChanged(QString)));

    ui->m_graphicTreeView->installEventFilter(this);

    QSettings settings;
    QString serverAddress = settings.value ("Connection/Server").toString ();

    if (settings.value ("Connection/AutoConnect").toString ().toLower () == "true" && !serverAddress.isEmpty ())
    {
        m_connection->connectToServer (serverAddress, settings.value ("Connection/Port", 31337).toInt ());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectToServer()
{
    bool ok = false;
    QSettings settings;
    QString address = QInputDialog::getText(this, tr("Connect To Server"), tr("Address:"), QLineEdit::Normal, settings.value("Connection/Server").toString(), &ok);

    if(ok && !address.isEmpty())
    {
        m_connection->connectToServer(address, settings.value("Connection/Port", 31337).toInt());
        settings.setValue("Connection/Server", address);
    }
}

void MainWindow::showConnectionErrorDialog(const QString &message)
{
    QMessageBox::critical(this, tr("Connection Error"), message);
}

void MainWindow::onConnected()
{
    ui->actionConnect->setEnabled(false);
    ui->actionDisconnect->setEnabled(true);
    ui->actionNewGraphic->setEnabled(true);

    m_connection->fetchShowList();
}

void MainWindow::onDisconnected()
{
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionNewGraphic->setEnabled(false);
}

void MainWindow::updateGraphicList(const QStringList &list)
{
    m_graphicModel->setStringList(list);
}

void MainWindow::toggleGraphicOnAir(const QModelIndex &index)
{
    QString graphic = m_graphicModel->data(index, Qt::DisplayRole).toString();
    m_connection->toggleGraphicOnAir(graphic);
}

void MainWindow::createGraphic(const QStringList &templates)
{
    QScopedPointer<CreateGraphicDialog> dialog(new CreateGraphicDialog(this));

    dialog->setTemplates(templates);

    if(dialog->exec() == QDialog::Accepted)
    {
        m_connection->createGraphic(dialog->name(), dialog->templateName());
    }
}

void MainWindow::onEditGraphic()
{
    QModelIndexList indexes = ui->m_graphicTreeView->selectionModel()->selectedIndexes();

    if(indexes.isEmpty())
    {
        return;
    }

    QString graphic = m_graphicModel->data(indexes.first(), Qt::DisplayRole).toString();

    if(graphic.isEmpty())
    {
        return;
    }

    m_connection->getProperties(graphic);
}

void MainWindow::editGraphicProperties(const QString &graphic, bool onAirTimerEnabled, int onAirTimerInterval,
                                       const QString& group, const QList<QPair<QString, QVariant> > &properties)
{
    QScopedPointer<GraphicPropertiesDialog> dialog(new GraphicPropertiesDialog(this));

    dialog->setOnAirTimerEnabled(onAirTimerEnabled);
    dialog->setOnAirTimerInterval(onAirTimerInterval);
    dialog->setGroup(group);
    dialog->setProperties(properties);

    if(dialog->exec() == QDialog::Accepted)
    {
        m_connection->setGraphicProperties(graphic, dialog->onAirTimerEnabled(), dialog->onAirTimerInterval(), dialog->group(), dialog->properties());
    }
}

void MainWindow::onRemoveGraphic()
{
    QModelIndexList indexes = ui->m_graphicTreeView->selectionModel()->selectedIndexes();

    if(indexes.isEmpty())
    {
        return;
    }

    QString graphic = m_graphicModel->data(indexes.first(), Qt::DisplayRole).toString();

    if(graphic.isEmpty())
    {
        return;
    }

    m_connection->removeGraphic(graphic);
}

void MainWindow::addGraphic(const QString &graphic)
{
    m_graphicModel->insertRows(m_graphicModel->rowCount(), 1);
    m_graphicModel->setData(m_graphicModel->index(m_graphicModel->rowCount() - 1), graphic);
}

void MainWindow::removeGraphic(const QString &graphic)
{
    for(int i = 0; i < m_graphicModel->rowCount(); ++i)
    {
        if(m_graphicModel->data(m_graphicModel->index(i), Qt::DisplayRole).toString() == graphic)
        {
            m_graphicModel->removeRows(i, 1);
            break;
        }
    }
}

void MainWindow::updateShowList(const QStringList &list, const QString &current)
{
    ui->m_showCombo->clear();
    ui->m_showCombo->addItems(list);

    if(!current.isEmpty())
    {
        ui->m_showCombo->setCurrentIndex(ui->m_showCombo->findText(current));
    }
}

void MainWindow::onNewShow()
{
    bool ok = false;
    QString showName = QInputDialog::getText(this, tr("New Show"), tr("Show name:"), QLineEdit::Normal, QString(), &ok);

    if(ok)
    {
        m_connection->createNewShow(showName);
    }
}

void MainWindow::onCurrentShowChanged(const QString &showName)
{
    m_connection->changeCurrentShow(showName);
}

void MainWindow::onRemoveShow()
{
    m_connection->removeCurrentShow();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if(keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Space)
        {
            if(ui->m_graphicTreeView->selectionModel()->hasSelection())
            {
                QModelIndex index = ui->m_graphicTreeView->selectionModel()->selectedIndexes().first();
                toggleGraphicOnAir(index);

                event->accept();
                return true;
            }
        }
    }

    return false;
}

void MainWindow::showGraphicContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->m_graphicTreeView->indexAt(pos);

    QScopedPointer<QMenu> menu(new QMenu(this));

    menu->addAction(ui->actionNewGraphic);

    if(index.isValid())
    {
        menu->addAction(ui->actionEditGraphic);
        menu->addAction(ui->actionRemoveGraphic);
    }

    menu->exec(ui->m_graphicTreeView->mapToGlobal(pos));
}
