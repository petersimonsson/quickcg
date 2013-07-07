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
#include "graphic.h"
#include "show.h"
#include "server.h"

#include <QShortcut>
#include <QDeclarativeComponent>
#include <QUrl>
#include <QDebug>
#include <QDeclarativeItem>
#include <QSettings>
#include <QDomDocument>
#include <QApplication>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QNetworkInterface>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_show(0),
    m_server(0),
    m_addressInfoItem(NULL)
{
    initDirs();

    ui->setupUi(this);

    ui->m_graphicsView->scene()->setBackgroundBrush(Qt::green);

    (void) new QShortcut(Qt::CTRL + Qt::Key_F, this, SLOT(toggleFullscreen()), 0, Qt::ApplicationShortcut);
    (void) new QShortcut(Qt::CTRL + Qt::Key_Q, this, SLOT(quit()), 0, Qt::ApplicationShortcut);

    QStringList showList = shows();

    if(!showList.isEmpty())
    {
        QString current = QSettings().value("CurrentShow").toString();

        if(current.isEmpty())
        {
            current = showList.first();
        }

        setCurrentShow(current);
    }

    m_server = new Server(this);

    m_addressInfoItem = new QGraphicsRectItem;
    m_addressInfoItem->setBrush(Qt::white);
    m_addressInfoItem->setRect(0, 0, 1920, 1080);
    ui->m_graphicsView->scene()->addItem(m_addressInfoItem);
    QGraphicsSimpleTextItem *textitem = new QGraphicsSimpleTextItem(m_addressInfoItem, ui->m_graphicsView->scene());
    textitem->setBrush(Qt::black);
    textitem->setPos(20, 20);
    QFont f = font();
    f.setPointSize(30);
    textitem->setFont(f);
    QNetworkInterface net;
    QString text;

    foreach(const QHostAddress &address, net.allAddresses())
    {
        if(!text.isEmpty())
        {
            text += "\n";
        }

        text += address.toString();
    }

    textitem->setText("Addresses:\n" + text);
}

MainWindow::~MainWindow()
{
    if(m_show)
    {
        m_show->save();
        QSettings().setValue("CurrentShow", m_show->showName());
    }

    delete ui;
}

void MainWindow::toggleFullscreen()
{
    if(isFullScreen())
    {
        showNormal();
    }
    else
    {
        showFullScreen();
    }
}

QDeclarativeComponent *MainWindow::loadTemplate(const QString &name)
{
    QUrl url = QUrl::fromLocalFile(m_templateDir.absoluteFilePath(name));
    QDeclarativeComponent *component = new QDeclarativeComponent(ui->m_graphicsView->engine(), url);

    if(component->isError())
    {
        qDebug() << "Failed to load component.";
        component->deleteLater();
        return 0;
    }

    return component;
}

void MainWindow::addItem(QDeclarativeItem* item)
{
    if(!item)
    {
        return;
    }

    ui->m_graphicsView->scene()->addItem(item);
}

void MainWindow::initDirs()
{
    m_templateDir = QDir::home();

    if(!m_templateDir.exists("QuickCG"))
    {
        m_templateDir.mkdir("QuickCG");
    }

    m_templateDir.cd("QuickCG");

    m_showDir = m_templateDir;

    if(!m_templateDir.exists("templates"))
    {
        m_templateDir.mkdir("templates");
    }

    m_templateDir.cd("templates");

    if (!m_showDir.exists("shows"))
    {
        m_showDir.mkdir("shows");
    }

    m_showDir.cd("shows");
}

QStringList MainWindow::templates() const
{
    return m_templateDir.entryList(QStringList() << "*.qml", QDir::Files);
}

QStringList MainWindow::shows() const
{
    return m_showDir.entryList(QStringList() << "*.show", QDir::Files);
}

void MainWindow::setCurrentShow(const QString& show)
{
    if(m_show)
    {
        m_show->save();
        m_show->deleteLater();
    }

    QString absolutePath = m_showDir.absoluteFilePath(show);
    QFileInfo info;
    info.setFile(absolutePath);

    if(info.exists() && info.isFile())
    {
        m_show = new Show(this);
        m_show->load(absolutePath);

        if(m_server)
        {
            m_server->sendShowList();
        }
    }
}

void MainWindow::createShow(const QString &name)
{
    if(name.isEmpty())
    {
        return;
    }

    QFileInfo info;
    QString filename = name + ".show";
    QString path = m_showDir.absoluteFilePath(filename);
    info.setFile(path);

    if(info.exists())
    {
        return;
    }

    QFile file(path);

    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open" << path << "for writing with the following error:" << file.errorString();
        return;
    }

    QDomDocument doc;
    QDomElement rootElement = doc.createElement("QuickCGShow");
    doc.appendChild(rootElement);
    file.write(doc.toString(4).toLocal8Bit());

    setCurrentShow(filename);
}

void MainWindow::removeShow(const QString &name)
{
    if(name.isEmpty())
    {
        return;
    }

    if(m_show && m_show->showName() == name)
    {
        m_show->deleteLater();
        m_show = 0;
    }

    showDir().remove(name);

    if(m_show)
    {
        m_server->sendShowList();
    }
    else
    {
        QStringList showList = shows();

        if(!showList.isEmpty())
        {
            setCurrentShow(showList.first());
        }
        else
        {
            m_server->sendShowList();
        }
    }
}

void MainWindow::quit()
{
    qApp->quit();
}

void MainWindow::removeAddressInfo()
{
    delete m_addressInfoItem;
    m_addressInfoItem = NULL;
}
