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

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("QuickCG");
    a.setApplicationVersion("0.1");
    a.setOrganizationName("Peter Simonsson");
    a.setOrganizationDomain("petersimonsson.net");

    QStringList arguments = a.arguments();
    arguments.removeFirst();
    bool fullscreen = false;

    if(!arguments.isEmpty())
    {
        foreach(const QString &argument, arguments)
        {
            if (argument == "--fullscreen")
            {
                fullscreen = true;
            }
        }
    }

    MainWindow w;

    if(fullscreen)
    {
        w.showFullScreen();
    }
    else
    {
        w.show();
    }

    return a.exec();
}
