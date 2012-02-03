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
#include "Chan.h"
#include "IRCSock.h"


class CIrcppbotMod : public CGlobalModule {

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


    void Nick(const CString& sLine)
    {
        if(m_pUser->IsAdmin())
        {
            CString sUsername = sLine.Token(1);
            CUser* pUser;
            const CNick* pNick;

            if (sUsername.empty()) {
                PutModule("Usage: nick username");
                return;
            }
            pUser = GetUser(sUsername);
            if (!pUser)
                return;

            pNick = &pUser->GetIRCNick();
            PutModule("Nick " + sUsername + " " + pNick->GetNick());
        }
        else
        {
            PutModule("Error: You need to have admin rights to use this module!");
        }
    }

    void Info(const CString& sLine)
    {
        if(m_pUser->IsAdmin())
        {
            CString sUsername = sLine.Token(1);
            CString sChannel = sLine.Token(2);
            CUser* pUser;
            const CNick* pNick;

            if (sUsername.empty()) {
                PutModule("Usage: info username channel");
                return;
            }
            if (sChannel.empty()) {
                PutModule("Usage: info username channel");
                return;
            }
            pUser = GetUser(sUsername);
            if (!pUser)
                return;

            pNick = &pUser->GetIRCNick();
            PutModule("Info " + sChannel + " " + sUsername + " Nick " + pNick->GetNick());
            PutModule("Info " + sChannel + " " + sUsername + " Ident " + pUser->GetIdent());
            PutModule("Info " + sChannel + " " + sUsername + " RealName " + pUser->GetRealName());
            PutModule("Info " + sChannel + " " + sUsername + " IRCServer " + pUser->GetIRCServer());
            if (pUser->IsUserAttached())
            {
                PutModule("Info " + sChannel + " " + sUsername + " Connected");
            }
            else
            {
                PutModule("Info " + sChannel + " " + sUsername + " Not Connected");
            }
        }
        else
        {
            PutModule("Error: You need to have admin rights to use this module!");
        }
    }

    void Set(const CString& sLine)
    {
        if(!m_pUser->IsAdmin())
        {
            PutModule("Access denied");
            return;
        }
        CString sWhat = sLine.Token(1);
        if (sWhat.Equals("IdleChannel"))
        {
            m_sIdleChannel = sLine.Token(2);
            SetNV("idlechannel", m_sIdleChannel);
        }
        else if (sWhat.Equals("AdminUser"))
        {
            m_sAdminUser = sLine.Token(2);
            SetNV("adminuser", m_sAdminUser);
        }
    }

    void Show(const CString&)
    {
        if(!m_pUser->IsAdmin())
        {
            PutModule("Access denied");
            return;
        }
        PutModule("IdleChannel " + m_sIdleChannel);
        PutModule("AdminUser " + m_sAdminUser);
    }
private:
    CString m_sIdleChannel;
    CString m_sAdminUser;

public:
    GLOBALMODCONSTRUCTOR(CIrcppbotMod) {
            AddHelpCommand();
            AddCommand("Info", static_cast<CModCommand::ModCmdFunc>(&CIrcppbotMod::Info));
            AddCommand("Nick", static_cast<CModCommand::ModCmdFunc>(&CIrcppbotMod::Nick));
            AddCommand("Set", static_cast<CModCommand::ModCmdFunc>(&CIrcppbotMod::Set));
            AddCommand("Show", static_cast<CModCommand::ModCmdFunc>(&CIrcppbotMod::Show));
    }

    virtual ~CIrcppbotMod() {}

    virtual bool OnLoad(const CString& sArgs, CString& sErrorMsg) {
        m_sIdleChannel = GetNV("idlechannel");
        m_sAdminUser = GetNV("adminuser");
        return true;
    }

    virtual void OnJoin(const CNick& mNick, CChan& Channel) {
        if (Channel.GetName() == m_sIdleChannel)
        {
            CUser *pUserTmp = CZNC::Get().FindUser(m_sAdminUser);
            if (pUserTmp == m_pUser)
            {
                const map<CString, CUser*>& msUsers = CZNC::Get().GetUserMap();
                for (map<CString, CUser*>::const_iterator it = msUsers.begin(); it != msUsers.end(); ++it) {
                    const CNick& pNick = it->second->GetIRCNick();
                    if (pNick.GetNick() == mNick.GetNick())
                    {
                        pUserTmp->PutIRC("PRIVMSG chanserv :voice " + m_sIdleChannel + " " + mNick.GetNick());
                        return;
                    }
                }
            }
        }
    }
};

template<> void TModInfo<CIrcppbotMod>(CModInfo& Info) {
    Info.SetWikiPage("ircppbot");
}

GLOBALMODULEDEFS(CIrcppbotMod, "ircppbot zncmodule")
