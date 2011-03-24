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

	void AddUser(std::string mChan, std::string mNick, std::string mAuth, std::string mReqNick, std::string mReqAuth, int oas);
	void DelUser(std::string mChan, std::string mNick, std::string mAuth, std::string mReqNick, std::string mReqAuth, int oas);
	void Stats(std::string mChan, std::string mNick, std::string mAuth, int oas);
	void Search(std::string mChan, std::string mNick, std::string mAuth, std::string mSearchString, int oas);
	void Info(std::string mChan, std::string mNick, std::string mAuth, std::string mSearchString, int oas);

    void timerlong();
    std::vector<int> timer_sec;
    std::vector< std::string > timer_command;
    std::vector<int> timer_long_sec;
    std::vector< std::string > timer_long_command;
    int longtime;

    bool run;
    unsigned int MaxUsers;
    boost::shared_ptr<boost::thread> privmsg_parse_thread;


    std::map< std::string, std::map< std::string, std::string > > znc_user_setting_map;
    std::map< std::string, std::map< std::string, std::string > >::iterator znc_user_setting_it;
    std::vector< std::string > znc_user_nick;

    std::map< std::string, std::string > globalsettings;
    void SaveConfig();
    bool ReadFile( std::string filename );
    std::string generatePwd(int length);
};

#endif // Znc_H


