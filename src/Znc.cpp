//
//
//  @ Project : ircppbot
//  @ File Name : Znc.cpp
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

#include "include/Znc.h"
#include <core/Global.h>
#include <core/DatabaseData.h>
#include <iostream>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <fstream>

extern "C" ModuleInterface* create()
{
    return new Znc;
}

extern "C" void destroy(ModuleInterface* x)
{
    delete x;
}

Znc::Znc()
{
}

Znc::~Znc()
{
    stop();
    Global::Instance().get_IrcData().DelConsumer(mpDataInterface);
    delete mpDataInterface;
}

void Znc::Init(DataInterface* pData)
{
    srand ( time(NULL) );
    mpDataInterface = pData;
    mpDataInterface->Init(false, false, false, true);
    Global::Instance().get_IrcData().AddConsumer(mpDataInterface);
    ReadFile(Global::Instance().get_ConfigReader().GetString("znc_config_file"));
    ReadAddUserText(Global::Instance().get_ConfigReader().GetString("znc_addusertext"));
    ReadResetPassText(Global::Instance().get_ConfigReader().GetString("znc_resetpasstext"));
    timerlong();
    command_table = "ZncCommands";
    DatabaseData::Instance().DatabaseData::AddBinds(command_table);
}


void Znc::stop()
{
    run = false;
    mpDataInterface->stop();
    std::cout << "Znc::stop" << std::endl;
    privmsg_parse_thread->join();
    std::cout << "privmsg_parse_thread stopped" << std::endl;
}

void Znc::read()
{
    run = true;
    assert(!privmsg_parse_thread);
    privmsg_parse_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&Znc::parse_privmsg, this)));
}

void Znc::parse_privmsg()
{
    std::vector< std::string > data;
    while(run)
    {
        data = mpDataInterface->GetPrivmsgQueue();
        PRIVMSG(data, Global::Instance().get_ConfigReader().GetString("znc_trigger"));
        ParseIrcppbotmod(data);
    }
}


void Znc::ParseIrcppbotmod(std::vector< std::string > vData)
{
    if (vData[0] == ":*ircppbot!znc@znc.in")
    {
        boost::erase_all(vData[3], ":");
        if (boost::iequals(vData[3], "Info"))
        {
            //std::map< std::string, std::string > setting;
            //setting.clear();
            for (unsigned int uiUsersIndex = 0; uiUsersIndex < znc_user_nick.size(); uiUsersIndex++)
            {
                if (boost::iequals(vData[5], znc_user_nick[uiUsersIndex]))
                {
                    std::string sSendString;
                    for (unsigned int j = 6; j < vData.size()-1; j++)
                    {
                        sSendString = sSendString + vData[j] + " ";
                    }
                    if (vData.size() > 0)
                    {
                        sSendString = sSendString + vData[vData.size()-1];
                    }
                    Send(Global::Instance().get_Reply().irc_privmsg(vData[4], sSendString));
                }
            }
        }
        if (boost::iequals(vData[3], "Users"))
        {
        }
        if (boost::iequals(vData[3], "Nick"))
        {
            if (vData.size() > 4)
            {
                for (unsigned int uiUsersIndex = 0; uiUsersIndex < znc_user_nick.size(); uiUsersIndex++)
                {
                    if (boost::iequals(vData[4], znc_user_nick[uiUsersIndex]))
                    {
                        Voice(vData[5]);
                    }
                }
            }
        }
    }
}

void Znc::ParsePrivmsg(std::string nick, std::string command, std::string chan, std::vector< std::string > args, int chantrigger)
{
    UsersInterface& U = Global::Instance().get_Users();
    std::string auth = U.GetAuth(nick);
    std::string global_trigger = Global::Instance().get_ConfigReader().GetString("znc_global_trigger");
    std::string local_trigger = Global::Instance().get_ConfigReader().GetString("znc_local_trigger");
    bool global = false;
    bool local = false;
    if (command.size() >= 1)
    {
        if (command.substr(0, global_trigger.length()) == global_trigger)
        {
            command = command.substr(global_trigger.length(), command.length()-1);
            global = true;
        }
        if (command.substr(0, local_trigger.length()) == local_trigger)
        {
            command = command.substr(local_trigger.length(), command.length()-1);
            local = true;
        }
    }
    if (local || global)
    {
        std::string bind_command = DatabaseData::Instance().GetCommandByBindNameAndBind(command_table, command);
        int bind_access = DatabaseData::Instance().GetAccessByBindNameAndBind(command_table, command);
        std::cout << bind_command << " " << bind_access << std::endl;

        // save config
        if (boost::iequals(command, "save"))
        {
            if (args.size() == 0)
            {
                Save(chan, nick, auth, bind_access);
            }
            else
            {
                //help(bind_command);
            }
            overwatch(bind_command, command, chan, nick, auth, args);
        }

        // send to status
        if (boost::iequals(command, "sendstatus"))
        {
            if (args.size() >= 1)
            {
                std::string _sSendString;
                for (unsigned int j = 0; j < args.size()-1; j++)
                {
                    _sSendString = _sSendString + args[j] + " ";
                }
                if (args.size() > 0)
                {
                    _sSendString = _sSendString + args[args.size()-1];
                }
                SendStatus(chan,nick,auth, _sSendString, bind_access);
            }
            else
            {
                //help(bind_command);
            }
            overwatch(bind_command, command, chan, nick, auth, args);
        }

        // send to admin of all users
        if (boost::iequals(command, "sendadminall"))
        {
            if (args.size() >= 1)
            {
                std::string _sSendString;
                for (unsigned int j = 0; j < args.size()-1; j++)
                {
                    _sSendString = _sSendString + args[j] + " ";
                }
                if (args.size() > 0)
                {
                    _sSendString = _sSendString + args[args.size()-1];
                }
                SendAdminAll(chan,nick,auth, _sSendString, bind_access);
            }
            else
            {
                //help(bind_command);
            }
            overwatch(bind_command, command, chan, nick, auth, args);
        }

        //znccommands
        if (boost::iequals(bind_command, "znccommands"))
        {
            if (args.size() == 0)
            {
                znccommands(nick, auth, bind_access);
            }
            else
            {
                //help(bind_command);
            }
            overwatch(bind_command, command, chan, nick, auth, args);
        }

        //auth
        if (boost::iequals(bind_command, "stats"))
        {
            if (args.size() == 0)
            {
                Stats(chan, nick, auth, bind_access);
            }
            else
            {
                //help(bind_command);
            }
            overwatch(bind_command, command, chan, nick, auth, args);
        }

        //joinall
        if (boost::iequals(bind_command, "joinall"))
        {
            if (args.size() == 0)
            {
                JoinAll(chan, nick, auth, bind_access);
            }
            else
            {
                //help(bind_command);
            }
            overwatch(bind_command, command, chan, nick, auth, args);
        }

        //voiceall
        if (boost::iequals(bind_command, "voiceall"))
        {
            if (args.size() == 0)
            {
                VoiceAll(chan, nick, auth, bind_access);
            }
            else
            {
                //help(bind_command);
            }
            overwatch(bind_command, command, chan, nick, auth, args);
        }

        //read
        if (boost::iequals(bind_command, "read"))
        {
            if (args.size() == 0)
            {
                ReadFile(Global::Instance().get_ConfigReader().GetString("znc_config_file"));
            }
            else
            {
                //help(bind_command);
            }
            overwatch(bind_command, command, chan, nick, auth, args);
        }

        // SetBindhost
        if (boost::iequals(bind_command, "setbindhost"))
        {
            if (args.size() == 1)
            {
                SetBindhost(chan, nick, auth, args[0], bind_access);
            }
            else
            {
                //help(bind_command);
            }
            overwatch(bind_command, command, chan, nick, auth, args);
        }

        //simulall
        if (boost::iequals(bind_command, "simulall"))
        {
            if (args.size() >= 1)
            {
                std::string simulString;
                for (unsigned int j = 0; j < args.size()-1; j++)
                {
                    simulString = simulString + args[j] + " ";
                }
                if (args.size() > 0)
                {
                    simulString = simulString + args[args.size()-1];
                }
                SimulAll(chan, nick, auth, simulString, bind_access);
            }
            else
            {
                //help(bind_command);
            }
            overwatch(bind_command, command, chan, nick, auth, args);
        }

        //simul
        if (boost::iequals(bind_command, "simul"))
        {
            if (args.size() >= 2)
            {
                std::string simulString;
                for (unsigned int j = 1; j < args.size()-1; j++)
                {
                    simulString = simulString + args[j] + " ";
                }
                if (args.size() > 1)
                {
                    simulString = simulString + args[args.size()-1];
                }
                SimulUser(chan, nick, auth, args[0], simulString, bind_access);
            }
            else
            {
                //help(bind_command);
            }
            overwatch(bind_command, command, chan, nick, auth, args);
        }

        //search
        if (boost::iequals(bind_command, "search"))
        {
            if (args.size() == 1)
            {
                Search(chan, nick, auth, args[0], bind_access);
            }
            else
            {
                //help(bind_command);
            }
            overwatch(bind_command, command, chan, nick, auth, args);
        }

        //info
        if (boost::iequals(bind_command, "info"))
        {
            if (args.size() == 1)
            {
                Info(chan, nick, auth, args[0], bind_access);
            }
            else
            {
                //help(bind_command);
            }
            overwatch(bind_command, command, chan, nick, auth, args);
        }

        // broadcast
        if (boost::iequals(bind_command, "broadcast"))
        {
            if (args.size() >= 1)
            {
                std::string sBroadcastString;
                for (unsigned int j = 0; j < args.size()-1; j++)
                {
                    sBroadcastString = sBroadcastString + args[j] + " ";
                }
                if (args.size() > 0)
                {
                    sBroadcastString = sBroadcastString + args[args.size()-1];
                }
                Broadcast(chan, nick, auth, sBroadcastString, bind_access);
            }
            else
            {
                //help(bind_command);
            }
            overwatch(bind_command, command, chan, nick, auth, args);
        }

        if (local)		//local only
        {

            //adduser
            if (boost::iequals(bind_command, "adduser"))
            {
                if (args.size() == 1)
                {
                    AddUser(chan, nick, auth, U.GetAuth(args[0]), args[0], bind_access);
                }
                else if (args.size() == 2)
                {
                    AddUser(chan, nick, auth, U.GetAuth(args[0]), args[1], bind_access);
                }
                else
                {
                    //help(bind_command);
                }
                overwatch(bind_command, command, chan, nick, auth, args);
            }

            //deluser
            if (boost::iequals(bind_command, "deluser"))
            {
                if (args.size() == 1)
                {
                    DelUser(chan, nick, auth, args[0], bind_access);
                }
                else
                {
                    //help(bind_command);
                }
                overwatch(bind_command, command, chan, nick, auth, args);
            }

            //resetpass
            if (boost::iequals(bind_command, "resetpass"))
            {
                if (args.size() == 1)
                {
                    ResetPasswd(chan, nick, auth, U.GetAuth(args[0]), args[0], bind_access);
                }
                else if (args.size() == 2)
                {
                    ResetPasswd(chan, nick, auth, U.GetAuth(args[0]), args[1], bind_access);
                }
                else
                {
                    //help(bind_command);
                }
                overwatch(bind_command, command, chan, nick, auth, args);
            }

            // send to admin
            if (boost::iequals(command, "sendadmin"))
            {
                if (args.size() >= 1)
                {
                    std::string _sSendString;
                    for (unsigned int j = 0; j < args.size()-1; j++)
                    {
                        _sSendString = _sSendString + args[j] + " ";
                    }
                    if (args.size() > 0)
                    {
                        _sSendString = _sSendString + args[args.size()-1];
                    }
                    SendAdmin(chan,nick,auth, _sSendString, bind_access);
                }
                else
                {
                    //help(bind_command);
                }
                overwatch(bind_command, command, chan, nick, auth, args);
            }
        }
    }
}

void Znc::ResetPasswd(std::string mChan, std::string mNick, std::string mAuth, std::string mReqAuth, std::string mSendNick, int oas)
{
    UsersInterface& U = Global::Instance().get_Users();
    int oaccess = U.GetOaccess(mNick);
    if (oaccess >= oas)
    {
        if (mReqAuth != "NULL")
        {
            std::string pass = generatePwd(8);
            std::string returnstr = "PRIVMSG *admin :set Password " + mReqAuth + " " + pass + "\r\n";
            Send(returnstr);
            /*returnstr = "NOTICE " + mSendNick + " :";
            returnstr = returnstr + Global::Instance().get_ConfigReader().GetString("znc_port");
            returnstr = returnstr + ": password for ";
            returnstr = returnstr + mReqAuth;
            returnstr = returnstr + " is now ";
            returnstr = returnstr + pass;
            returnstr = returnstr + "\r\n";
            Send(returnstr);*/

            SaveConfig();
            for (unsigned int ResetPassText_it = 0; ResetPassText_it < ResetPassText.size(); ResetPassText_it++)
            {
                std::string tmpstring = ResetPassText[ResetPassText_it];
                tmpstring = Global::Instance().get_Reply().irc_reply_replace(tmpstring, "%creater%", mNick);
                tmpstring = Global::Instance().get_Reply().irc_reply_replace(tmpstring, "%password%", pass);
                tmpstring = Global::Instance().get_Reply().irc_reply_replace(tmpstring, "%auth%", mReqAuth);
                tmpstring = Global::Instance().get_Reply().irc_reply_replace(tmpstring, "%port%", Global::Instance().get_ConfigReader().GetString("znc_port"));
                returnstr = "PRIVMSG " + mSendNick + " :";
                returnstr = returnstr + tmpstring;
                returnstr = returnstr + "\r\n";
                Send(returnstr);
            }
            Send(returnstr);
            returnstr = "PRIVMSG " + mChan + " :";
            returnstr = returnstr + Global::Instance().get_ConfigReader().GetString("znc_port");
            returnstr = returnstr + ": password resetted for ";
            returnstr = returnstr + mReqAuth;
            returnstr = returnstr + "\r\n";
            Send(returnstr);
        }
        else
        {
            std::string returnstring = "NOTICE " + mNick + " :" + irc_reply("user_not_authed", U.GetLanguage(mNick)) + "\r\n";
            Send(returnstring);
        }
    }
    else
    {
        std::string returnstring = "NOTICE " + mNick + " :" + irc_reply("need_oaccess", U.GetLanguage(mNick)) + "\r\n";
        Send(returnstring);
    }
}

void Znc::SetBindhost(std::string mChan, std::string mNick, std::string mAuth, std::string mBindhost, int oas)
{
    UsersInterface& U = Global::Instance().get_Users();
    int oaccess = U.GetOaccess(mNick);
    if (oaccess >= oas)
    {
        std::string returnstr;
        for (unsigned int uiUsersIndex = 0; uiUsersIndex < znc_user_nick.size(); uiUsersIndex++)
        {
            if (znc_user_nick[uiUsersIndex] != Global::Instance().get_ConfigReader().GetString("znc_account"))
            {
                returnstr = "PRIVMSG *admin :Set VHost " + znc_user_nick[uiUsersIndex] + " " + mBindhost + "\r\n";
                Send(returnstr);
                returnstr = "PRIVMSG *admin :Set BindHost " + znc_user_nick[uiUsersIndex] + " " + mBindhost + "\r\n";
                Send(returnstr);
                returnstr = "PRIVMSG *admin :Set DenySetBindHost " + znc_user_nick[uiUsersIndex] + " true\r\n";
                Send(returnstr);
            }
        }
        SaveConfig();
    }
    else
    {
        std::string returnstring = "NOTICE " + mNick + " :" + irc_reply("need_oaccess", U.GetLanguage(mNick)) + "\r\n";
        Send(returnstring);
    }
}
//
//void Znc::SetServer(std::string msChan, std::string msNick, std::string msAuth, std::string msServer, int miOperAccess)
//{
//    UsersInterface& U = Global::Instance().get_Users();
//    int iOperAccess = U.GetOaccess(msNick);
//    if (iOperAccess >= miOperAccess)
//    {
//        std::string sIrcSendString;
//        for (unsigned int uiUsersIndex = 0; uiUsersIndex < znc_user_nick.size(); uiUsersIndex++)
//        {
//            std::vector< std::string > server_vector;
//            boost::split( server_vector, znc_user_setting_map[znc_user_nick[it_i]]["Server"], boost::is_any_of(" "), boost::token_compress_on );
//            sIrcSendString = "PRIVMSG *admin :Set server " + znc_user_nick[uiUsersIndex] + " " + msServer;
//            if(server_vector.size()>=3)
//            {
//                sIrcSendString = sIrcSendString + " " + server_vector[2];
//            }
//        }
//        SaveConfig();
//    }
//    else
//    {
//        std::string returnstring = "NOTICE " + mNick + " :" + irc_reply("need_oaccess", U.GetLanguage(mNick)) + "\r\n";
//        Send(returnstring);
//    }
//}



void Znc::Broadcast(std::string msChan, std::string msNick, std::string msAuth, std::string msBroadcastMessage, int miOperAccess)
{
    UsersInterface& U = Global::Instance().get_Users();
    int iOperAccess = U.GetOaccess(msNick);
    if (iOperAccess >= miOperAccess)
    {
        std::string sIrcSendString;
        sIrcSendString = "PRIVMSG *status :broadcast [ZNC Announcement (from " + msAuth + ")] " + msBroadcastMessage;
        Send(sIrcSendString);
        Send(Global::Instance().get_Reply().irc_privmsg(msChan, "Broadcast sended: " + sIrcSendString));
    }
    else
    {
        std::string returnstring = "NOTICE " + msNick + " :" + irc_reply("need_oaccess", U.GetLanguage(msNick)) + "\r\n";
        Send(returnstring);
    }
}

void Znc::SendAdminAll(std::string msChan, std::string msNick, std::string msAuth, std::string msSendString, int miOperAccess)
{
    UsersInterface& U = Global::Instance().get_Users();
    if (U.GetOaccess(msNick) >= miOperAccess)
    {
        std::string sSearch = "%users%";
        size_t search_pos;
        search_pos = msSendString.find(sSearch);
        if (search_pos != std::string::npos)
        {
            std::string sTempSendString;
            for (unsigned int uiUsersIndex = 0; uiUsersIndex < znc_user_nick.size(); uiUsersIndex++)
            {
                if (znc_user_nick[uiUsersIndex] != Global::Instance().get_ConfigReader().GetString("znc_account"))
                {
                    sTempSendString = msSendString;
                    sTempSendString.replace(search_pos, sSearch.length(), znc_user_nick[uiUsersIndex]);
                    Send(Global::Instance().get_Reply().irc_privmsg("*admin", sTempSendString));
                }
            }
            Send(Global::Instance().get_Reply().irc_privmsg(msChan, Global::Instance().get_ConfigReader().GetString("znc_port") + ": Send to *admin : " + msSendString));
        }
        else
        {
            Send(Global::Instance().get_Reply().irc_notice(msNick, irc_reply("wrong_input", U.GetLanguage(msNick))));
        }
    }
    else
    {
        Send(Global::Instance().get_Reply().irc_notice(msNick, irc_reply("need_oaccess", U.GetLanguage(msNick))));
    }
}

void Znc::SendAdmin(std::string msChan, std::string msNick, std::string msAuth, std::string msSendString, int miOperAccess)
{
    UsersInterface& U = Global::Instance().get_Users();
    if (U.GetOaccess(msNick) >= miOperAccess)
    {
        Send(Global::Instance().get_Reply().irc_privmsg("*admin", msSendString));
        Send(Global::Instance().get_Reply().irc_privmsg(msChan, Global::Instance().get_ConfigReader().GetString("znc_port") + ": Send to *admin : " + msSendString));
    }
    else
    {
        Send(Global::Instance().get_Reply().irc_notice(msNick, irc_reply("need_oaccess", U.GetLanguage(msNick))));
    }
}

void Znc::SendStatus(std::string msChan, std::string msNick, std::string msAuth, std::string msSendString, int miOperAccess)
{
    UsersInterface& U = Global::Instance().get_Users();
    if (U.GetOaccess(msNick) >= miOperAccess)
    {
        Send(Global::Instance().get_Reply().irc_privmsg("*status", msSendString));
        Send(Global::Instance().get_Reply().irc_privmsg(msChan, Global::Instance().get_ConfigReader().GetString("znc_port") + ": Send to *status"));
    }
    else
    {
        Send(Global::Instance().get_Reply().irc_notice(msNick, irc_reply("need_oaccess", U.GetLanguage(msNick))));
    }
}

void Znc::Save(std::string msChan, std::string msNick, std::string msAuth, int miOperAccess)
{
    UsersInterface& U = Global::Instance().get_Users();
    if (U.GetOaccess(msNick) >= miOperAccess)
    {
        SaveConfig();
        Send(Global::Instance().get_Reply().irc_privmsg(msChan, Global::Instance().get_ConfigReader().GetString("znc_port") + ": Saved Config"));
    }
    else
    {
        Send(Global::Instance().get_Reply().irc_notice(msNick, irc_reply("need_oaccess", U.GetLanguage(msNick))));
    }
}

void Znc::AddUser(std::string mChan, std::string mNick, std::string mAuth, std::string mReqAuth, std::string mSendNick, int oas)
{
    UsersInterface& U = Global::Instance().get_Users();
    int oaccess = U.GetOaccess(mNick);
    if (oaccess >= oas)
    {
        if (mReqAuth != "NULL")
        {
            std::string returnstr;
            returnstr = "PRIVMSG " + mChan + " :";
            returnstr = returnstr + Global::Instance().get_ConfigReader().GetString("znc_port");
            returnstr = returnstr + ": adding user: ";
            returnstr = returnstr + mReqAuth;
            returnstr = returnstr + "\r\n";
            Send(returnstr);
            std::string pass = generatePwd(8);
            returnstr = "PRIVMSG *admin :adduser " + mReqAuth + " " + pass + " " + Global::Instance().get_ConfigReader().GetString("znc_irc_server") + "\r\n";
            Send(returnstr);
            returnstr = "PRIVMSG *admin :Set VHost " + mReqAuth + " " + Global::Instance().get_ConfigReader().GetString("znc_vhost") + "\r\n";
            Send(returnstr);
            returnstr = "PRIVMSG *admin :Set BindHost " + mReqAuth + " " + Global::Instance().get_ConfigReader().GetString("znc_vhost") + "\r\n";
            Send(returnstr);
            returnstr = "PRIVMSG *admin :Set DenySetBindHost " + mReqAuth + " true\r\n";
            Send(returnstr);
            returnstr = "PRIVMSG *admin :LoadModule " + mReqAuth + " " + "admin" + "\r\n";
            Send(returnstr);
            JoinChannel(mReqAuth);
            usleep(1000000);
            Simul(mReqAuth, "QUIT");
            usleep(1000000);
            SaveConfig();
            for (unsigned int AddUserText_it = 0; AddUserText_it < AddUserText.size(); AddUserText_it++)
            {
                std::string tmpstring = AddUserText[AddUserText_it];
                tmpstring = Global::Instance().get_Reply().irc_reply_replace(tmpstring, "%creater%", mNick);
                tmpstring = Global::Instance().get_Reply().irc_reply_replace(tmpstring, "%password%", pass);
                tmpstring = Global::Instance().get_Reply().irc_reply_replace(tmpstring, "%auth%", mReqAuth);
                tmpstring = Global::Instance().get_Reply().irc_reply_replace(tmpstring, "%port%", Global::Instance().get_ConfigReader().GetString("znc_port"));
                returnstr = "PRIVMSG " + mSendNick + " :";
                returnstr = returnstr + tmpstring;
                returnstr = returnstr + "\r\n";
                Send(returnstr);
            }
            Send(returnstr);
            returnstr = "PRIVMSG " + mChan + " :";
            returnstr = returnstr + Global::Instance().get_ConfigReader().GetString("znc_port");
            returnstr = returnstr + ": added ";
            returnstr = returnstr + mReqAuth;
            returnstr = returnstr + "\r\n";
            Send(returnstr);
            JoinChannel(mReqAuth);
        }
        else
        {
            std::string returnstring = "NOTICE " + mNick + " :" + irc_reply("user_not_authed", U.GetLanguage(mNick)) + "\r\n";
            Send(returnstring);
        }
    }
    else
    {
        std::string returnstring = "NOTICE " + mNick + " :" + irc_reply("need_oaccess", U.GetLanguage(mNick)) + "\r\n";
        Send(returnstring);
    }
}

void Znc::DelUser(std::string mChan, std::string mNick, std::string mAuth, std::string mReqAuth, int oas)
{
    UsersInterface& U = Global::Instance().get_Users();
    int oaccess = U.GetOaccess(mNick);
    if (oaccess >= oas)
    {
        std::string returnstr = "PRIVMSG *admin :deluser " + mReqAuth + "\r\n";
        Send(returnstr);
        SaveConfig();
        returnstr = "PRIVMSG " + mChan + " :";
        returnstr = returnstr + Global::Instance().get_ConfigReader().GetString("znc_port");
        returnstr = returnstr + ": deleted ";
        returnstr = returnstr + mReqAuth;
        returnstr = returnstr + "\r\n";
        Send(returnstr);
    }
    else
    {
        std::string returnstring = "NOTICE " + mNick + " :" + irc_reply("need_oaccess", U.GetLanguage(mNick)) + "\r\n";
        Send(returnstring);
    }
}

void Znc::Stats(std::string mChan, std::string mNick, std::string mAuth, int oas)
{
    ReadFile(Global::Instance().get_ConfigReader().GetString("znc_config_file"));
    UsersInterface& U = Global::Instance().get_Users();
    int oaccess = U.GetOaccess(mNick);
    if (oaccess >= oas)
    {
        std::string nUsers = convertInt(znc_user_nick.size());
        std::string maxUsers = convertInt(convertString(Global::Instance().get_ConfigReader().GetString("znc_max_users")));
        std::string returnstr = "PRIVMSG " + mChan + " :";
        returnstr = returnstr + Global::Instance().get_ConfigReader().GetString("znc_port");
        returnstr = returnstr + ": ";
        if (znc_user_nick.size() < convertString(Global::Instance().get_ConfigReader().GetString("znc_max_users")))
        {
            returnstr = returnstr + nUsers;
            returnstr = returnstr + "/";
            returnstr = returnstr + maxUsers;
        }
        if (znc_user_nick.size() >= convertString(Global::Instance().get_ConfigReader().GetString("znc_max_users")))
        {
            returnstr = returnstr + "FULL";
        }
        returnstr = returnstr + "\r\n";
        Send(returnstr);
    }
    else
    {
        std::string returnstring = "NOTICE " + mNick + " :" + irc_reply("need_oaccess", U.GetLanguage(mNick)) + "\r\n";
        Send(returnstring);
    }
}

void Znc::JoinAll(std::string mChan, std::string mNick, std::string mAuth, int oas)
{
    UsersInterface& U = Global::Instance().get_Users();
    int oaccess = U.GetOaccess(mNick);
    if (oaccess >= oas)
    {
        for (unsigned int uiUsersIndex = 0; uiUsersIndex < znc_user_nick.size(); uiUsersIndex++)
        {
            if (znc_user_nick[uiUsersIndex] != Global::Instance().get_ConfigReader().GetString("znc_account"))
            {
                JoinChannel(znc_user_nick[uiUsersIndex]);
            }
        }
    }
    else
    {
        std::string returnstring = "NOTICE " + mNick + " :" + irc_reply("need_oaccess", U.GetLanguage(mNick)) + "\r\n";
        Send(returnstring);
    }
}

void Znc::VoiceAll(std::string mChan, std::string mNick, std::string mAuth, int oas)
{
    UsersInterface& U = Global::Instance().get_Users();
    int oaccess = U.GetOaccess(mNick);
    if (oaccess >= oas)
    {
        for (unsigned int uiUsersIndex = 0; uiUsersIndex < znc_user_nick.size(); uiUsersIndex++)
        {
            if (znc_user_nick[uiUsersIndex] != Global::Instance().get_ConfigReader().GetString("znc_account"))
            {
                Send(Global::Instance().get_Reply().irc_privmsg("*ircppbot", "nick " + znc_user_nick[uiUsersIndex]));
                //Voice(znc_user_nick[uiUsersIndex]);
            }
        }
    }
    else
    {
        std::string returnstring = "NOTICE " + mNick + " :" + irc_reply("need_oaccess", U.GetLanguage(mNick)) + "\r\n";
        Send(returnstring);
    }
}

void Znc::SimulAll(std::string mChan, std::string mNick, std::string mAuth, std::string mSimulString, int oas)
{
    UsersInterface& U = Global::Instance().get_Users();
    int oaccess = U.GetOaccess(mNick);
    if (oaccess >= oas)
    {
        for (unsigned int uiUsersIndex = 0; uiUsersIndex < znc_user_nick.size(); uiUsersIndex++)
        {
            if (znc_user_nick[uiUsersIndex] != Global::Instance().get_ConfigReader().GetString("znc_account"))
            {
                Simul(znc_user_nick[uiUsersIndex], mSimulString);
            }
        }
    }
    else
    {
        std::string returnstring = "NOTICE " + mNick + " :" + irc_reply("need_oaccess", U.GetLanguage(mNick)) + "\r\n";
        Send(returnstring);
    }
}

void Znc::SimulUser(std::string mChan, std::string mNick, std::string mAuth, std::string mReqAuth, std::string mSimulString, int oas)
{
    UsersInterface& U = Global::Instance().get_Users();
    int oaccess = U.GetOaccess(mNick);
    if (oaccess >= oas)
    {
        Simul(mReqAuth, mSimulString);
    }
    else
    {
        std::string returnstring = "NOTICE " + mNick + " :" + irc_reply("need_oaccess", U.GetLanguage(mNick)) + "\r\n";
        Send(returnstring);
    }
}

void Znc::Search(std::string mChan, std::string mNick, std::string mAuth, std::string mSearchString, int oas)
{
    ReadFile(Global::Instance().get_ConfigReader().GetString("znc_config_file"));
    boost::to_lower(mSearchString);
    size_t star;
    star = mSearchString.find("*");	//first *
    if (star != std::string::npos)
    {
        mSearchString.replace(star, 1, "");
    }
    star = mSearchString.find("*");	//last * (if used right)
    if (star != std::string::npos)
    {
        mSearchString.replace(star, 1, "");
    }
    UsersInterface& U = Global::Instance().get_Users();
    int oaccess = U.GetOaccess(mNick);
    if (oaccess >= oas)
    {
        size_t searchpos;
        for (unsigned int it_i = 0; it_i < znc_user_nick.size(); it_i++)
        {
            std::string tmp_nick = znc_user_nick[it_i];
            boost::to_lower(tmp_nick);
            searchpos = tmp_nick.find(mSearchString);
            if (searchpos != std::string::npos)
            {
                std::string returnstr = "PRIVMSG " + mChan + " :";
                returnstr = returnstr + Global::Instance().get_ConfigReader().GetString("znc_port");
                returnstr = returnstr + ": ";
                returnstr = returnstr + znc_user_nick[it_i];
                returnstr = returnstr + "\r\n";
                Send(returnstr);
            }
        }
    }
    else
    {
        std::string returnstring = "NOTICE " + mNick + " :" + irc_reply("need_oaccess", U.GetLanguage(mNick)) + "\r\n";
        Send(returnstring);
    }
}

void Znc::Info(std::string mChan, std::string mNick, std::string mAuth, std::string mSearchString, int oas)
{
    ReadFile(Global::Instance().get_ConfigReader().GetString("znc_config_file"));
    boost::to_lower(mSearchString);
    size_t star;
    star = mSearchString.find("*");	//first *
    if (star != std::string::npos)
    {
        mSearchString.replace(star, star + 1, "");
    }
    star = mSearchString.find("*");	//last * (if used right)
    if (star != std::string::npos)
    {
        mSearchString.replace(star, star + 1, "");
    }
    UsersInterface& U = Global::Instance().get_Users();
    int oaccess = U.GetOaccess(mNick);
    if (oaccess >= oas)
    {
        size_t searchpos;
        for (unsigned int it_i = 0; it_i < znc_user_nick.size(); it_i++)
        {
            std::string tmp_nick = znc_user_nick[it_i];
            boost::to_lower(tmp_nick);
            searchpos = tmp_nick.find(mSearchString);
            if (searchpos != std::string::npos)
            {
                Send(Global::Instance().get_Reply().irc_privmsg("*ircppbot", "info " + znc_user_nick[it_i] + " " + mChan));
                /*
                std::string returnstr = "PRIVMSG " + mChan + " :";
                returnstr = returnstr + Global::Instance().get_ConfigReader().GetString("znc_port");
                returnstr = returnstr + ": ";
                returnstr = returnstr + znc_user_nick[it_i];
                returnstr = returnstr + "\r\n";
                Send(returnstr);
                returnstr = "PRIVMSG " + mChan + " :";
                returnstr = returnstr + Global::Instance().get_ConfigReader().GetString("znc_port");
                returnstr = returnstr + ": Nick:  ";
                returnstr = returnstr + znc_user_setting_map[znc_user_nick[it_i]]["Nick"];
                returnstr = returnstr + "\r\n";
                Send(returnstr);
                returnstr = "PRIVMSG " + mChan + " :";
                returnstr = returnstr + Global::Instance().get_ConfigReader().GetString("znc_port");
                returnstr = returnstr + ": Ident:  ";
                returnstr = returnstr + znc_user_setting_map[znc_user_nick[it_i]]["Ident"];
                returnstr = returnstr + "\r\n";
                Send(returnstr);
                returnstr = "PRIVMSG " + mChan + " :";
                returnstr = returnstr + Global::Instance().get_ConfigReader().GetString("znc_port");
                returnstr = returnstr + ": Server:  ";
                std::vector< std::string > server_vector;
                boost::split( server_vector, znc_user_setting_map[znc_user_nick[it_i]]["Server"], boost::is_any_of(" "), boost::token_compress_on );
                if(server_vector.size()>=1)
                {
                    returnstr = returnstr + server_vector[0];
                }
                if(server_vector.size()>=2)
                {
                    returnstr = returnstr + " " + server_vector[1];
                }
                returnstr = returnstr + "\r\n";
                Send(returnstr);*/
            }
        }
    }
    else
    {
        std::string returnstring = "NOTICE " + mNick + " :" + irc_reply("need_oaccess", U.GetLanguage(mNick)) + "\r\n";
        Send(returnstring);
    }
}

void Znc::znccommands(std::string mNick, std::string mAuth, int oas)
{
    UsersInterface& U = Global::Instance().get_Users();
    int oaccess = U.GetOaccess(mNick);
    if (oaccess >= oas)
    {
        UsersInterface& U = Global::Instance().get_Users();
        std::string returnstring;
        unsigned int length = U.GetWidth(mNick);
        unsigned int amount = U.GetWidthLength(mNick);
        std::string commandrpl = irc_reply("znccommands", U.GetLanguage(mNick));
        returnstring = "NOTICE " + mNick + " :";
        for (unsigned int l = 0; l < (((length * amount) / 2) - commandrpl.size()/2); l++)
        {
            returnstring = returnstring + " ";
        }
        returnstring = returnstring + commandrpl + "\r\n";
        Send(returnstring);


        returnstring = "NOTICE " + mNick + " :";
        returnstring = returnstring + fillspace("bind", 20);
        returnstring = returnstring + fillspace("command", 20);
        returnstring = returnstring + "access\r\n";
        Send(returnstring);
        std::vector< std::string > binds = DatabaseData::Instance().GetBindVectorByBindName(command_table);
        sort (binds.begin(), binds.end());
        for (unsigned int binds_it = 0; binds_it < binds.size(); binds_it++)
        {
            std::string bind_access = convertInt(DatabaseData::Instance().GetAccessByBindNameAndBind(command_table, binds[binds_it]));
            std::string bind_command = DatabaseData::Instance().GetCommandByBindNameAndBind(command_table, binds[binds_it]);
            if (bind_command != "")
            {
                returnstring = "NOTICE " + mNick + " :";
                returnstring = returnstring + fillspace(binds[binds_it], 20);
                returnstring = returnstring + fillspace(bind_command, 20);
                returnstring = returnstring + bind_access + "\r\n";
                Send(returnstring);
            }
        }
    }
}

void Znc::SaveConfig()
{
    std::string returnstr = "PRIVMSG *status :SaveConfig\r\n";
    Send(returnstr);
    boost::shared_ptr<boost::thread> SaveReadThread;
    assert(!SaveReadThread);
    SaveReadThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&Znc::SaveRead, this)));
}

void Znc::SaveRead()
{
    usleep(5000000);
    ReadFile(Global::Instance().get_ConfigReader().GetString("znc_config_file"));
}

void Znc::JoinChannel(std::string mNick)
{
    std::string returnstr = "PRIVMSG *send_raw :";
    returnstr = returnstr + mNick;
    returnstr = returnstr + " JOIN ";
    returnstr = returnstr + Global::Instance().get_ConfigReader().GetString("znc_idle_channel");
    returnstr = returnstr + "\r\n";
    Send(returnstr);
}

void Znc::Voice(std::string mNick)
{
    std::string returnstr = "PRIVMSG chanserv ";
    returnstr = returnstr + Global::Instance().get_ConfigReader().GetString("znc_idle_channel");
    returnstr = returnstr + " voice ";
    returnstr = returnstr + mNick;
    returnstr = returnstr + "\r\n";
    Send(returnstr);
}

void Znc::Simul(std::string mNick, std::string mSimulString)
{
    std::string returnstr = "PRIVMSG *send_raw :";
    returnstr = returnstr + mNick;
    returnstr = returnstr + " ";
    returnstr = returnstr + mSimulString;
    returnstr = returnstr + "\r\n";
    Send(returnstr);
}


void Znc::timerrun()
{
    //cout << "channelbot::timerrun()" << endl;
    int Tijd;
    time_t t= time(0);
    Tijd = t;
    longtime++;
    if (longtime >= 100)
    {
        timerlong();
        longtime = 0;
    }
    for (int i = timer_sec.size() - 1; i >= 0; i--)
    {
        if (timer_sec[i] < Tijd)
        {
            std::cout << timer_command[i] << std::endl;
            timer_sec.erase(timer_sec.begin()+i);
            timer_command.erase(timer_command.begin()+i);
        }
    }
}

void Znc::timerlong()
{
    int Tijd;
    time_t t= time(0);
    Tijd = t;
    Tijd = Tijd + 100;
    for (int i = timer_long_sec.size() - 1; i >= 0; i--)
    {
        if (timer_long_sec[i] < Tijd)
        {
            std::cout << "timer_long to timer " << timer_long_command[i] << std::endl;
            timer_sec.push_back(timer_long_sec[i]);
            timer_command.push_back(timer_long_command[i]);
            timer_long_sec.erase(timer_long_sec.begin()+i);
            timer_long_command.erase(timer_long_command.begin()+i);
        }
    }
}


bool Znc::ReadFile( std::string filename )
{
    std::cout << "readfile: " << filename << std::endl;
    std::string tmp_user_str;
    bool in_user_section = false;
    std::map< std::string, std::string > setting;
    std::string line;
    std::ifstream configfile;
    int linenr = 0;

    // Start fresh :)
    setting.clear();
    znc_user_setting_map.clear();
    znc_user_nick.clear();

    configfile.open(filename.c_str());
    if (configfile.is_open())
    {
        while (configfile.good())
        {
            getline(configfile,line);
            linenr++;

            boost::trim(line);

            // Filter empty and comment lines
            if ((line.length()>0) && (line[0] != '#'))
            {
                std::vector< std::string > vector_line;
                boost::split( vector_line, line, boost::is_any_of(" "), boost::token_compress_on );
                // Debug
                //cout << "Line " << linenr << ": " << line << endl;

                if (vector_line.size() == 2)
                {
                    if (boost::iequals(vector_line[0], "<User"))
                    {
                        tmp_user_str = vector_line[1];
                        boost::erase_all(tmp_user_str, ">");
                        boost::trim(tmp_user_str);
                        in_user_section = true;
                        setting.clear();
                    }
                }
                if (in_user_section)
                {
                    std::vector< std::string > setting_vector;
                    boost::split( setting_vector, line, boost::is_any_of("="), boost::token_compress_on );
                    if (setting_vector.size() >= 2)
                    {
                        //std::cout << "line:" << line << std::endl;
                        boost::trim(setting_vector[0]);
                        boost::trim(setting_vector[1]);
                        setting[setting_vector[0]] = setting_vector[1];
                    }
                }
                else
                {
                    std::vector< std::string > setting_vector;
                    boost::split( setting_vector, line, boost::is_any_of("="), boost::token_compress_on );
                    if (setting_vector.size() >= 2)
                    {
                        //std::cout << "line:" << line << std::endl;
                        boost::trim(setting_vector[0]);
                        boost::trim(setting_vector[1]);
                        globalsettings[setting_vector[0]] = setting_vector[1];
                    }
                }

                if (vector_line.size() == 1)
                {
                    if (boost::iequals(vector_line[0], "</User>"))
                    {
                        std::pair< std::map< std::string, std::map< std::string, std::string > >::iterator, bool > znc_user_setting_ret;
                        znc_user_setting_ret = znc_user_setting_map.insert (std::pair< std::string, std::map< std::string, std::string > > (tmp_user_str, setting));
                        if (znc_user_setting_ret.second == true)
                        {
                            std::cout << "new user: " << tmp_user_str << std::endl;
                            znc_user_nick.push_back(tmp_user_str);
                        }
                        else
                        {
                            std::cout << "user more thene once in config :/" << std::endl;
                        }
                        in_user_section = false;
                    }
                }
            }

        }
        configfile.close();
        for (unsigned int it_i = 0; it_i < znc_user_nick.size(); it_i++)
        {
            std::cout << "user " << znc_user_nick[it_i] << std::endl;
            std::cout << "value " << znc_user_setting_map[znc_user_nick[it_i]]["Pass"] << std::endl;
        }
        return true;
    }
    else
    {
        std::cout << "Could not open file '" << filename << "'" << std::endl;
    }

    return false;
}

bool Znc::ReadResetPassText( std::string filename )
{
    std::cout << "ReadResetPassText readfile: " << filename << std::endl;
    ResetPassText.clear();
    std::string line;
    std::ifstream configfile;
    int linenr = 0;

    configfile.open(filename.c_str());
    if (configfile.is_open())
    {
        while (configfile.good())
        {
            getline(configfile,line);
            linenr++;

            boost::trim(line);
            ResetPassText.push_back(line);

        }
        configfile.close();
        return true;
    }
    else
    {
        std::cout << "Could not open file '" << filename << "'" << std::endl;
    }
    return false;
}

bool Znc::ReadAddUserText( std::string filename )
{
    std::cout << "ReadAddUserText readfile: " << filename << std::endl;
    AddUserText.clear();
    std::string line;
    std::ifstream configfile;
    int linenr = 0;

    configfile.open(filename.c_str());
    if (configfile.is_open())
    {
        while (configfile.good())
        {
            getline(configfile,line);
            linenr++;

            boost::trim(line);
            AddUserText.push_back(line);

        }
        configfile.close();
        return true;
    }
    else
    {
        std::cout << "Could not open file '" << filename << "'" << std::endl;
    }
    return false;
}



std::string Znc::generatePwd(int length)
{
  std::string out="";
  for(int i=0; i<length; i++)
  {
    char chr=rand()%52;
    if(chr<=25)
    {
        out+='A'+chr;
    }
    else
    {
        out+='a'+chr-26;
    }
  }
  return out;
}
