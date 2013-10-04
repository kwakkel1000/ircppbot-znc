//
//
// @ Project : ircppbot-zncbot
// @ File Name : zncbot.h
// @ Date : 04-10-2013
// @ Author : Gijs Kwakkel
//
//
// Copyright (c) 2013 Gijs Kwakkel
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//

#ifndef SRC_INCLUDE_ZNCBOT_H
#define SRC_INCLUDE_ZNCBOT_H

#include <string>
#include <vector>
#include <map>

#include <atomic>
#include <thread>
#include <mutex>
#include <memory>

#include <ircppbot/moduleinterface.h>
#include <ircppbot/management.h>
#include <ircppbot/ircdata.h>

class ircdata;
class zncbot : public moduleinterface
{
    public:
        zncbot();
        ~zncbot();
        void read();
        void stop();
        void init();
        void timerrun(){}

    private:

        std::atomic<bool> m_Run;
        ircdata* m_IrcData;

        //void parseModes();
        //void parseEvents();
        void parsePrivmsg();

        //std::shared_ptr< std::thread > m_ModesThread;
        //std::shared_ptr< std::thread > m_EventsThread;
        std::shared_ptr< std::thread > m_PrivmsgThread;

        void version(std::string target, std::string userName);
        void addUser(std::string userName, std::string userAuth, std::string targetAuth, std::string targetUserName, size_t bindAccess);
        void delUser(std::string userName, std::string userAuth, std::string targetAuth, size_t bindAccess);
        void resetPassword(std::string userName, std::string userAuth, std::string targetAuth, std::string targetUserName, size_t bindAccess);

        std::string generatePassword(int length);
        bool fixZncAuth(std::string& targetAuth);

//        void zncCommands(std::string, std::string);
        bool nickFromHostmask(std::string& data);
        bool authFromNick(std::string& data);
        bool deleteFirst(std::string& data, std::string character);
        bool deleteAll(std::string& data, std::string character);


};

#endif // SRC_INCLUDE_ZNCBOT_H
/*
    DataInterface* mpDataInterface;
    void parse_privmsg();
    void ParseIrcppbotmod(std::vector< std::string > vData);
    void ParsePrivmsg(std::string nick, std::string command, std::string chan, std::vector< std::string > args, int chantrigger);

    void ResetPasswd(std::string mChan, std::string mNick, std::string mAuth, std::string mReqAuth, std::string mSendNick, int oas);
    void SetBindhost(std::string mChan, std::string mNick, std::string mAuth, std::string mBindhost, int oas);
    void AddUser(std::string mChan, std::string mNick, std::string mAuth, std::string mReqAuth, std::string mSendNick, int oas);
    void DelUser(std::string mChan, std::string mNick, std::string mAuth, std::string mReqAuth, int oas);
    void Stats(std::string mChan, std::string mNick, std::string mAuth, int oas);
    void JoinAll(std::string mChan, std::string mNick, std::string mAuth, int oas);
    void VoiceAll(std::string mChan, std::string mNick, std::string mAuth, int oas);
    void SimulAll(std::string mChan, std::string mNick, std::string mAuth, std::string mSimulString, int oas);
    void SimulUser(std::string mChan, std::string mNick, std::string mAuth, std::string mReqAuth, std::string mSimulString, int oas);
    void Search(std::string mChan, std::string mNick, std::string mAuth, std::string mSearchString, int oas);
    void Info(std::string mChan, std::string mNick, std::string mAuth, std::string mSearchString, int oas);
    void Broadcast(std::string msChan, std::string msNick, std::string msAuth, std::string msBroadcastMessage, int miOperAccess);
    void SendStatus(std::string msChan, std::string msNick, std::string msAuth, std::string msSendString, int miOperAccess);
    void SendAdminAll(std::string msChan, std::string msNick, std::string msAuth, std::string msSendString, int miOperAccess);
    void SendAdmin(std::string msChan, std::string msNick, std::string msAuth, std::string msSendString, int miOperAccess);
    void Save(std::string msChan, std::string msNick, std::string msAuth, int miOperAccess);
    void znccommands(std::string mNick, std::string mAuth, int oas);

    void timerlong();
    std::vector<int> timer_sec;
    std::vector< std::string > timer_command;
    std::vector<int> timer_long_sec;
    std::vector< std::string > timer_long_command;
    int longtime;

    bool run;
    std::string command_table;
    std::vector< std::string > AddUserText;
    std::vector< std::string > ResetPassText;
    boost::shared_ptr<boost::thread> privmsg_parse_thread;



    std::map< std::string, std::map< std::string, std::string > > znc_user_setting_map;
    std::map< std::string, std::map< std::string, std::string > >::iterator znc_user_setting_it;
    std::vector< std::string > znc_user_nick;

    std::map< std::string, std::string > globalsettings;
    void SaveConfig();
    void SaveRead();
    void JoinChannel(std::string mNick);
    void Voice(std::string mNick);
    void Simul(std::string mNick, std::string mSimulString);
    bool ReadFile( std::string filename );
    bool ReadAddUserText( std::string filename );
    bool ReadResetPassText( std::string filename );
    std::string generatePwd(int length);
};

#endif // Znc_H


*/
