/*Copyright (C) 2009 Cleriot Simon
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA*/

#include "serveur.h"
#include "chatwindow.h"

#define AUTO_OPEN_CHANNEL   "#Caishen"

#define REGEX_PRIV_MESSAGE      ":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ PRIVMSG ([a-zA-Z0-9\\#]+) :(.+)"
#define REGEX_JOIN              ":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ JOIN ([a-zA-Z0-9\\#]+)"
#define REGEX_PART              ":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ PART ([a-zA-Z0-9\\#]+)"
#define REGEX_QUIT              ":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ QUIT (.+)"
#define REGEX_NAME              ":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ NICK :(.+)"
#define REGEX_KICK              ":([a-zA-Z0-9]+)\\![a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ KICK ([a-zA-Z0-9\\#]+) ([a-zA-Z0-9]+) :(.+)"
#define REGEX_NOTICE            ":([a-zA-Z0-9]+)\\![a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ NOTICE ([a-zA-Z0-9]+) :(.+)"
#define REGEX_NOTICE_CHANGE     ":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ NOTICE [a-zA-Z0-9]+ :(.+)"

QStringList users;
bool needsDeleteUserList = true;

ServerIrc::ServerIrc()
{
	connect(this, SIGNAL(readyRead()), this, SLOT(readServeur()));
	connect(this, SIGNAL(connected()), this, SLOT(connected()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorSocket(QAbstractSocket::SocketError)));

	updateUsers=false;
}

void ServerIrc::errorSocket(QAbstractSocket::SocketError error)
{
	switch(error)
	{
		case QAbstractSocket::HostNotFoundError:
            affichage->append(tr("<em>ERROR : can't find freenode server.</em>"));
			break;
		case QAbstractSocket::ConnectionRefusedError:
            affichage->append(tr("<em>ERROR : server refused connection</em>"));
			break;
		case QAbstractSocket::RemoteHostClosedError:
            affichage->append(tr("<em>ERROR : server cut connection</em>"));
			break;
		default:
            affichage->append(tr("<em>ERROR : ") + this->errorString() + tr("</em>"));
	}
}

void ServerIrc::connected()
{
    affichage->append("Connecting...");

    sendData("USER " + pseudo + " localhost " + serverLink + " :" + pseudo);
    sendData("NICK " + pseudo);
    affichage->append("Connected to freenode.");
}

void ServerIrc::joins()
{
    join(AUTO_OPEN_CHANNEL);
}

void ServerIrc::changeNickname()
{
    pseudo = pseudo + "_2";
    pseudo.remove("\r\n");
    sendData("NICK " + pseudo);
    emit pseudoChanged(pseudo);
    writeToChatWindow("-> Name changed to " + pseudo);
}

void ServerIrc::sendPong(QString message)
{
    QStringList liste = message.split(" ");
    QString msg = "PONG " + liste.at(1);
    sendData(msg);
}

void ServerIrc::readServeur()
{
    QString message = QString::fromUtf8(this->readAll());
    QString currentChan = tab->tabText(tab->currentIndex());

    qDebug() << "Chat server message: " << message;

    if (message.startsWith("PING :")) {
        sendPong(message);
    } else if (message.contains("Nickname is already in use.")) {
        changeNickname();
    } else if (updateUsers == true) {
        updateUsersList("", message);
	}

    QStringList list = message.split("\r\n");
    foreach(QString msg, list) {

        if (msg.contains(QRegExp(REGEX_PRIV_MESSAGE))) {

            QRegExp reg(REGEX_PRIV_MESSAGE);
            QString msg2=msg;
            writeToChatWindow(msg.replace(reg,"\\2 <b>&lt;\\1&gt;</b> \\3"),"",msg2.replace(reg,"\\2 <\\1> \\3"));

        } else if (msg.contains(QRegExp(REGEX_JOIN))) {

            QRegExp reg(REGEX_JOIN);
            QString msg2 = msg;
            writeToChatWindow(msg.replace(reg,"\\2 <i>-> \\1 join \\2</i><br />"),"",msg2.replace(reg,"-> \\1 join \\2"));
            updateUsersList(msg.replace(reg,"\\2"));

        } else if (msg.contains(QRegExp(REGEX_PART))) {

            QRegExp reg(REGEX_PART);
            QString msg2=msg;
            writeToChatWindow(msg.replace(reg,"\\2 <i>-> \\1 quit \\2 (\\3)</i><br />"),"",msg2.replace(reg,"-> \\1 quit \\2"));
            updateUsersList(msg.replace(reg,"\\2"));

        } else if (msg.contains(QRegExp(REGEX_QUIT))) {

            QRegExp reg(REGEX_QUIT);
            QString msg2=msg;
            writeToChatWindow(msg.replace(reg,"\\2 <i>-> \\1 quit this server (\\2)</i><br />"),"",msg2.replace(reg,"-> \\1 left"));
            updateUsersList(msg.replace(reg,"\\2"));

        } else if (msg.contains(QRegExp(REGEX_NAME))) {

            QRegExp reg(REGEX_NAME);
            QString msg2=msg;
            writeToChatWindow(msg.replace(reg,"<i>\\1 is now called \\2</i><br />"),"",msg2.replace(reg,"-> \\1 is now called \\2"));
            updateUsersList(currentChan);

        } else if (msg.contains(QRegExp(REGEX_KICK))) {

            QRegExp reg(REGEX_KICK);
            QString msg2=msg;
            writeToChatWindow(msg.replace(reg,"\\2 <i>-> \\1 kicked \\3 (\\4)</i><br />"),"",msg2.replace(reg,"-> \\1 quit \\3"));
            updateUsersList(msg.replace(reg,"\\2"));

        } else if (msg.contains(QRegExp(REGEX_NOTICE))) {

            if (conversations.contains(currentChan)) {
                QRegExp reg(REGEX_NOTICE_CHANGE);
                writeToChatWindow(msg.replace(reg,"<b>[NOTICE] <i>\\1</i> : \\2 <br />"),currentChan);
            } else if (currentChan == serverLink) {
                QRegExp reg(REGEX_NOTICE_CHANGE);
                writeToChatWindow(msg.replace(reg,"<b>[NOTICE] <i>\\1</i> : \\2 <br />"));
            }

        } else if (msg.contains("/MOTD command.")) {
            joins();
        } else if (msg.contains(QRegExp("= ([a-zA-Z0-9\\#]+) :"))) {

            QStringList msg3 = msg.split("= ");
            QStringList msg4 = msg3[1].split(" :");
            updateUsersList(msg4[0],msg);
        } else if (msg.contains("366") && msg.contains("End of /NAMES")) {
            this->updateUsersListGUI();
            this->requestClearUserList();
        }
    }
}

void ServerIrc::sendData(QString txt)
{
	if(this->state()==QAbstractSocket::ConnectedState)
	{
        qDebug() << "Writing to socket: " << txt;
        this->write((txt + "\r\n").toUtf8());
	}
}

void ServerIrc::join(QString chan)
{
    affichage->append("Joining "+ chan +" channel");
	emit joinTab();
    QTextEdit *textEdit = new QTextEdit;
	int index=tab->insertTab(tab->currentIndex()+1,textEdit,chan);
    tab->setTabToolTip(index,serverLink);
	tab->setCurrentIndex(index);

	textEdit->setReadOnly(true);

    conversations.insert(chan, textEdit);

    sendData("JOIN "+chan);

	emit tabJoined();
}
void ServerIrc::leave(QString chan)
{
    sendData(parseCommand("/part "+chan+ " "+msgQuit));
}

QString ServerIrc::parseCommand(QString comm, bool serveur)
{
    if(comm.startsWith("/"))
    {
        comm.remove(0,1);
        QString pref = comm.split(" ").first();
        QStringList args = comm.split(" ");
        args.removeFirst();
        QString destChan = tab->tabText(tab->currentIndex());
        QString msg = args.join(" ");

        if(pref == "me")
            return "PRIVMSG " + destChan + " ACTION " + msg + "";
        else if(pref == "join")
        {
            join(msg);
            return " ";
        }
        else if(pref=="quit")
        {
            if(msg == "")
                return "QUIT " + msgQuit;
            else
                return "QUIT " + msg;
        }
        else if(pref=="part")
        {
            tab->removeTab(tab->currentIndex());

            if(msg == "")
            {
                if(msg.startsWith("#"))
                    destChan = msg.split(" ").first();

                if(msgQuit == "")
                    return "PART " + destChan + " using IrcLightClient";
                else
                    return "PART " + destChan + " " + msgQuit;
            }
            else
                return "PART " + destChan + " " + msg;

            conversations.remove(destChan);
        }
        else if(pref=="kick")
        {
            QStringList tableau = msg.split(" ");
            QString c1,c2,c3;
            if(tableau.count() > 0) c1 = " " + tableau.first();
            else c1 = "";
            if(tableau.count() > 1) c2 = " "+tableau.at(1);
            else c2 = "";
            if(tableau.count() > 2) c3 = " "+tableau.at(2);
            else c3 = "";

            if(c1.startsWith("#"))
                return "KICK"+c1+c2+c3;
            else
                return "KICK "+destChan+c1+c2;
        }
        else if(pref=="update")
        {
            updateUsers=true;
            return "WHO "+destChan;
        }
        else if(pref=="ns")
        {
            return "NICKSERV "+msg;
        }
        else if(pref=="nick")
        {
            emit pseudoChanged(msg);
            writeToChatWindow("-> Nickname changed to "+msg);
            return "NICK "+msg;
        }
        else
            return pref + " " + msg;
    }
    else if(!serveur)
    {
        QString destChan=tab->tabText(tab->currentIndex());
        if(comm.endsWith("<br />"))
            comm=comm.remove(QRegExp("<br />$"));
        writeToChatWindow("<b>&lt;"+pseudo+"&gt;</b> "+comm,destChan);

        if(comm.startsWith(":")) {
            comm.insert(0,":");
        }

        return "PRIVMSG " + destChan + " :" + comm + "";
    }
    else
    {
        return "";
    }
}

void ServerIrc::printTextOnChannel(QString destChan, QString txt)
{
    QString chatKey;

    if (conversations.contains(destChan)) {
        chatKey = destChan;
    } else if (conversations.contains(destChan.toLower())) {
        chatKey = destChan.toLower();
    } else {
        return;
    }

    conversations[chatKey]->setHtml(conversations[chatKey]->toHtml()+txt);
    QScrollBar *sb = conversations[chatKey]->verticalScrollBar();
    sb->setValue(sb->maximum());
}

QString ServerIrc::extractChannelName(QString txt)
{
    QString dest = txt.split(" ").first();
    return dest;
}

void ServerIrc::writeToChatWindow(QString txt,QString destChan,QString msgTray)
{
    if(destChan!="") {
        printTextOnChannel(destChan, txt);
    } else if (txt.startsWith("#")) {

        QString dest = extractChannelName(txt);
        QStringList list = txt.split(" ");
        list.removeFirst();
        txt = list.join(" ");

        printTextOnChannel(dest, txt);

    } else {

        txt.replace("\r\n","<br />");
        affichage->setHtml(affichage->toHtml()+txt+"<br />");
        QScrollBar *sb = affichage->verticalScrollBar();
        sb->setValue(sb->maximum());
    }
}

void ServerIrc::updateUsersList(QString chan,QString message)
{
    qDebug() << "Received IRC users list: " << message;
    message = message.replace("\r\n","");
    message = message.replace("\r","");
    if(chan != serverLink)
    {
        if(updateUsers==true || message != "")
        {
            QString liste2 = message.replace(":","");
            QStringList usersNamesList = liste2.split(" ");

            // Checking if we got users list, not other info
            if (usersNamesList.at(1) != "353") {
                return;
            }

            if (needsDeleteUserList == true) {
                users.clear();
                needsDeleteUserList = false;
            }

            for(int i=5; i < usersNamesList.count(); i++)
            {
                users.append(usersNamesList.at(i));
            }
            updateUsers=false;
            users.sort(Qt::CaseInsensitive);
        }
        else
        {
            updateUsers=true;
            sendData("NAMES " + chan);
        }
    }
    else
    {
        QStringListModel model;
        userList->setModel(&model);
        userList->update();
    }
}

void ServerIrc::requestClearUserList()
{
    needsDeleteUserList = true;
}

void ServerIrc::updateUsersListGUI()
{
    QStringListModel *model = new QStringListModel(users);
    userList->setModel(model);
    userList->update();
}
