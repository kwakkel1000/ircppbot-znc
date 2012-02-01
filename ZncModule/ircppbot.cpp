/*
 * Copyright (C) 2011-2012  See the AUTHORS file for details.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include "znc.h"
#include "User.h"
#include "Nick.h"
#include "Modules.h"
#include "Chan.h"
#include "IRCSock.h"

template<std::size_t N>
struct array_size_helper {
    char __place_holder[N];
};

template<class T, std::size_t N>
static array_size_helper<N> array_size(T (&)[N]) {
    return array_size_helper<N>();
}

#define ARRAY_SIZE(array) sizeof(array_size((array)))

class CIrcppbotMod : public CModule {
    using CModule::PutModule;

    void PrintHelp(const CString&) {
        HandleHelpCommand();
    }

    CUser* GetUser(const CString& sUsername) {
            if (sUsername.Equals("$me"))
                    return m_pUser;
            CUser *pUser = CZNC::Get().FindUser(sUsername);
            if (!pUser) {
                    PutModule("Error: User not found: " + sUsername);
                    return NULL;
            }
            if (pUser != m_pUser && !m_pUser->IsAdmin()) {
                    PutModule("Error: You need to have admin rights to modify other users!");
                    return NULL;
            }
            return pUser;
    }
    
    void Users(const CString&)
    {
        const map<CString, CUser*>& msUsers = CZNC::Get().GetUserMap();
        for (map<CString, CUser*>::const_iterator it = msUsers.begin(); it != msUsers.end(); ++it) {
            PutModule("Users " + it->first);
        }
    }

    void Info(const CString& sLine)
    {
        CString sUsername  = sLine.Token(1);
        CString sChannel = sLine.Token(2);
        CUser* pUser;
        const CNick* pNick;

        if (sUsername.empty()) {
                PutModule("Usage: info <username channel>");
                return;
        }
        if (sChannel.empty()) {
                PutModule("Usage: info <username channel>");
                return;
        }
        pUser = GetUser(sUsername);
        pNick = &pUser->GetIRCNick();
        PutModule("Info " + sChannel + " " +  sUsername + " Nick " + pNick->GetNick());
        PutModule("Info " + sChannel + " " +  sUsername + " Ident " + pUser->GetIdent());
        PutModule("Info " + sChannel + " " +  sUsername + " RealName " + pUser->GetRealName());
        PutModule("Info " + sChannel + " " +  sUsername + " IRCServer " + pUser->GetIRCServer());
        if (pUser->IsUserAttached())
        {
            PutModule("Info " + sChannel + " " +  sUsername + " Connected");
        }
        else
        {
            PutModule("Info " + sChannel + " " +  sUsername + " Not Connected");
        }
    }

public:
    MODCONSTRUCTOR(CIrcppbotMod) {
        AddCommand("Help",         static_cast<CModCommand::ModCmdFunc>(&CIrcppbotMod::PrintHelp),
            "",                              "Generates this output");
        AddCommand("Info",         static_cast<CModCommand::ModCmdFunc>(&CIrcppbotMod::Info),
            "username channel",              "Prints the info for the given user");
        AddCommand("Users",        static_cast<CModCommand::ModCmdFunc>(&CIrcppbotMod::Users),
            "",                              "Prints the list of users");
    }

    virtual ~CIrcppbotMod() {}
};

template<> void TModInfo<CIrcppbotMod>(CModInfo& Info) {
    Info.SetWikiPage("ircppbot");
}

MODULEDEFS(CIrcppbotMod, "ircppbot zncmodule")
