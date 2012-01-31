/*
 * Copyright (C) 2011-2012  See the AUTHORS file for details.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include "znc.h"
#include "User.h"
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

    void Info(const CString& sLine)
    {
        CString sUsername  = sLine.Token(1, true);
        CString sChannel = sLine.Token(2);
        CUser* pUser;

        if (sUsername.empty()) {
                PutModule("Usage: info <username channel>");
                return;
        }
        if (sChannel.empty()) {
                PutModule("Usage: info <username channel>");
                return;
        }
        pUser = GetUser(sUsername);
        PutModule(sChannel + " " +  sUsername + " Nick " + pUser->GetNick());
        PutModule(sChannel + " " +  sUsername + " Ident " + pUser->GetIdent());
        PutModule(sChannel + " " +  sUsername + " RealName " + pUser->GetRealName());
        PutModule(sChannel + " " +  sUsername + " IRCServer " + pUser->GetIRCServer());
    }

public:
    MODCONSTRUCTOR(CIrcppbotMod) {
        AddCommand("Help",         static_cast<CModCommand::ModCmdFunc>(&CIrcppbotMod::PrintHelp),
            "",                              "Generates this output");
        AddCommand("Info",         static_cast<CModCommand::ModCmdFunc>(&CIrcppbotMod::Info),
            "variable [username channel]",   "Prints the info for the given user");
    }

    virtual ~CIrcppbotMod() {}
};

template<> void TModInfo<CIrcppbotMod>(CModInfo& Info) {
    Info.SetWikiPage("ircppbot");
}

MODULEDEFS(CIrcppbotMod, "ircppbot zncmodule")
