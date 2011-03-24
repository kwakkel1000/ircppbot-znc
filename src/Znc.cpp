#include "include/Znc.h"
#include <core/Global.h>
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
	mpDataInterface = pData;
	mpDataInterface->Init(false, false, false, true);
    Global::Instance().get_IrcData().AddConsumer(mpDataInterface);
    MaxUsers = 50;		//config file
	ReadFile(Global::Instance().get_ConfigReader().GetString("zncfile"));
    timerlong();
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
        PRIVMSG(data, Global::Instance().get_ConfigReader().GetString("znctrigger"));
    }
}


void Znc::ParsePrivmsg(std::string nick, std::string command, std::string chan, std::vector< std::string > args, int chantrigger)
{
    std::cout << "Znc" << std::endl;
    UsersInterface& U = Global::Instance().get_Users();
    std::string auth = U.GetAuth(nick);
    if (args.size() == 0)
    {
		if (boost::iequals(command, "stats"))
		{
			overwatch(command, command, chan, nick, auth, args);
			Stats(chan, nick, auth, 0);
		}
		if (boost::iequals(command, "read"))
		{
			overwatch(command, command, chan, nick, auth, args);
			ReadFile(Global::Instance().get_ConfigReader().GetString("zncfile"));
		}
    }
    if (args.size() == 1)
    {
		if (boost::iequals(command, "search"))
		{
			overwatch(command, command, chan, nick, auth, args);
			Search(chan, nick, auth, args[0], 0);
		}
		if (boost::iequals(command, "info"))
		{
			overwatch(command, command, chan, nick, auth, args);
			Info(chan, nick, auth, args[0], 0);
		}
		if (boost::iequals(command, "adduser"))
		{
			overwatch(command, command, chan, nick, auth, args);
			AddUser(chan, nick, auth, args[0], args[0], 0);
		}
		if (boost::iequals(command, "deluser"))
		{
			overwatch(command, command, chan, nick, auth, args);
			DelUser(chan, nick, auth, args[0], args[0], 0);
		}
    }
}

void Znc::AddUser(std::string mChan, std::string mNick, std::string mAuth, std::string mReqNick, std::string mReqAuth, int oas)
{
	std::string pass = generatePwd(8);
	std::string returnstr = "PRIVMSG *admin :adduser " + mReqNick + " " + pass + "irc.onlinegamesnet.net\r\n";
	Send(returnstr);
	returnstr = "PRIVMSG " + mChan + " :";
	returnstr = returnstr + globalsettings["Listener"];
	returnstr = returnstr + ": added ";
	returnstr = returnstr + mReqNick;
	returnstr = returnstr + " : ";
	returnstr = returnstr + pass;
	returnstr = returnstr + "\r\n";
	Send(returnstr);
}

void Znc::DelUser(std::string mChan, std::string mNick, std::string mAuth, std::string mReqNick, std::string mReqAuth, int oas)
{
	std::string returnstr = "PRIVMSG *admin :deluser " + mReqNick + "\r\n";
	Send(returnstr);
	returnstr = "PRIVMSG " + mChan + " :";
	returnstr = returnstr + globalsettings["Listener"];
	returnstr = returnstr + ": deleted ";
	returnstr = returnstr + mReqNick;
	returnstr = returnstr + "\r\n";
	Send(returnstr);
}

void Znc::Stats(std::string mChan, std::string mNick, std::string mAuth, int oas)
{
	std::string nUsers = convertInt(znc_user_nick.size());
	std::string maxUsers = convertInt(MaxUsers);
	std::string returnstr = "PRIVMSG " + mChan + " :";
	returnstr = returnstr + globalsettings["Listener"];
	returnstr = returnstr + ": ";
	if (znc_user_nick.size() < MaxUsers)
	{
		returnstr = returnstr + nUsers;
		returnstr = returnstr + "/";
		returnstr = returnstr + maxUsers;
	}
	if (znc_user_nick.size() >= MaxUsers)
	{
		returnstr = returnstr + "FULL";
	}
	returnstr = returnstr + "\r\n";
	Send(returnstr);
}

void Znc::Search(std::string mChan, std::string mNick, std::string mAuth, std::string mSearchString, int oas)
{
    size_t searchpos;
	for (unsigned int it_i = 0; it_i < znc_user_nick.size(); it_i++)
	{
		searchpos = znc_user_nick[it_i].find(mSearchString);
		if (searchpos != std::string::npos)
		{
			std::string returnstr = "PRIVMSG " + mChan + " :";
			returnstr = returnstr + globalsettings["Listener"];
			returnstr = returnstr + ": ";
			returnstr = returnstr + znc_user_nick[it_i];
			returnstr = returnstr + "\r\n";
			Send(returnstr);
		}
	}
}

void Znc::Info(std::string mChan, std::string mNick, std::string mAuth, std::string mSearchString, int oas)
{
    size_t searchpos;
	for (unsigned int it_i = 0; it_i < znc_user_nick.size(); it_i++)
	{
		searchpos = znc_user_nick[it_i].find(mSearchString);
		if (searchpos != std::string::npos)
		{
			std::string returnstr = "PRIVMSG " + mChan + " :";
			returnstr = returnstr + globalsettings["Listener"];
			returnstr = returnstr + ": ";
			returnstr = returnstr + znc_user_nick[it_i];
			returnstr = returnstr + "\r\n";
			Send(returnstr);
			returnstr = "PRIVMSG " + mChan + " :";
			returnstr = returnstr + globalsettings["Listener"];
			returnstr = returnstr + ": Nick:  ";
			returnstr = returnstr + znc_user_setting_map[znc_user_nick[it_i]]["Nick"];
			returnstr = returnstr + "\r\n";
			Send(returnstr);
			returnstr = "PRIVMSG " + mChan + " :";
			returnstr = returnstr + globalsettings["Listener"];
			returnstr = returnstr + ": Ident:  ";
			returnstr = returnstr + znc_user_setting_map[znc_user_nick[it_i]]["Ident"];
			returnstr = returnstr + "\r\n";
			Send(returnstr);
			returnstr = "PRIVMSG " + mChan + " :";
			returnstr = returnstr + globalsettings["Listener"];
			returnstr = returnstr + ": Server:  ";
			returnstr = returnstr + znc_user_setting_map[znc_user_nick[it_i]]["Server"];
			returnstr = returnstr + "\r\n";
			Send(returnstr);
		}
	}
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
