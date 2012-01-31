//
//
//  @ Project : ircppbot
//  @ File Name : Znc.h
//  @ Date : 4/18/2011
//  @ Author : Gijs Kwakkel
//
//
// Copyright (c) 2011 Gijs Kwakkel
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

#ifndef Znc_H
#define Znc_H
#include <core/ModuleBase.h>
#include <interfaces/DataInterface.h>
#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

class DataInterface;
class Znc : public ModuleBase
{
public:
    Znc();
    ~Znc();
    void read();
    void stop();
    void Init(DataInterface* pData);
    void timerrun();

private:

    DataInterface* mpDataInterface;
    void parse_privmsg();
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


