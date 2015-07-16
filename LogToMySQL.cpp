#include <znc/znc.h>
#include <znc/Modules.h>
#include <znc/Chan.h>
#include <znc/IRCNetwork.h>
#include <znc/User.h>
 
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <map>


using namespace std;

class CMySQLLog : public CModule {
public:
	MODCONSTRUCTOR(CMySQLLog) {}

	virtual bool OnLoad(const CString& sArgsi, CString& sMessage) override
	{
		CString configOK = SetConfigFromArgs(sArgsi);

		if (configOK != "success")
		{
			sMessage = configOK;
			return false;
		}
		sMessage = "Success!";
		Connect();
		return true;
	}

	CString SetConfigFromArgs(CString sArgsi)
	{
		map<CString, CString> config;
		VCString args;
		sArgsi.Split(";", args);
		vector<CString> requiredconfigitems = {"database", "host", "username", "password"};

		for (unsigned int i = 0; i < args.size(); i++)
		{
			CString settingname;
			CString setting;
			settingname = args[i].Token(0, false, ":");
			setting = args[i].Token(1, true, ":");
			if (settingname != "" && setting != "")
			{
				conInfo[settingname] = setting;
			}
		}

		for (vector<CString>::iterator it = requiredconfigitems.begin(); it != requiredconfigitems.end(); ++it)
		{
			if (conInfo.find(*it) == conInfo.end())
			{
				return CString("'" + *it + "' not found");
			}
		}
		return "success";
	}

	EModRet OnChanMsg(CNick& Nick, CChan& Channel, CString& sMessage) override {
		if (!connected)
		{
			PutModule("Not connected");
			Connect();
			return CONTINUE;
		}

		InsertToDB(
			vector<CString> {
				GetUser()->GetUserName(),
				"chan",
				Channel.GetName(),
				Nick.GetNick(),
				CString((Nick.GetIdent() + "@" + Nick.GetHost())),
				CString(time(NULL)),
				sMessage.StripControls()
			}
		);

		return CONTINUE;
	}

	EModRet OnPrivMsg(CNick& Nick, CString& sMessage) override {
		InsertToDB(
			vector<CString> {
				GetUser()->GetUserName(),
				"privmsg",
				"",
				Nick.GetNick(),
				CString((Nick.GetIdent() + "@" + Nick.GetHost())),
				CString(time(NULL)),
				sMessage.StripControls()
			}
		);
		return CONTINUE;
	}

	void OnJoin(const CNick& Nick, CChan& Channel) override {
		InsertToDB(
			vector<CString> {
				GetUser()->GetUserName(),
				"chan",
				Channel.GetName(),
				Nick.GetNick(),
				CString((Nick.GetIdent() + "@" + Nick.GetHost())),
				CString(time(NULL)),
				"*** Joins: " + Nick.GetNick() + " (" + CString(Nick.GetIdent() + "@" + Nick.GetHost()) + ")"
			}
		);
	}

	void OnKick(const CNick& OpNick, const CString& sKickedNick, CChan& Channel, const CString& sMessage)
	{
		InsertToDB(
			vector<CString> {
				GetUser()->GetUserName(),
				"chan",
				Channel.GetName(),
				sKickedNick,
				"",
				CString(time(NULL)),
				"*** " + sKickedNick + " was kicked by " + OpNick.GetNick() + " (" + sMessage + ")"
			}
		);
	}

	void OnQuit(const CNick& Nick, const CString& sMessage, const vector<CChan*>& vChans)
	{
		for (vector<CChan*>::const_iterator it = vChans.begin(); it != vChans.end(); ++it)
		{
			InsertToDB(
				vector <CString> {
					GetUser()->GetUserName(),
					"chan",
					(*it)->GetName(),
					Nick.GetNick(),
					CString((Nick.GetIdent() + "@" + Nick.GetHost())),
					CString(time(NULL)),
					"*** Quits: " + Nick.GetNick() + " (" + Nick.GetIdent() + "@" + Nick.GetHost() + ") (" + sMessage + ")"
				}
			);
		}
	}

	void OnNick(const CNick& OldNick, const CString& sNewNick, const vector<CChan*>& vChans)
	{
		for (vector<CChan*>::const_iterator it = vChans.begin(); it != vChans.end(); ++it)
		{
			InsertToDB(
				vector <CString> {
					GetUser()->GetUserName(),
					"chan",
					(*it)->GetName(),
					OldNick.GetNick(),
					CString((OldNick.GetIdent() + "@" + OldNick.GetHost())),
					CString(time(NULL)),
					"*** " + OldNick.GetNick() + " is now known as " + sNewNick
				}
			);
		}
	}

	void OnPart(const CNick& Nick, CChan& Channel, const CString& sMessage)
	{
		InsertToDB(
			vector<CString> {
				GetUser()->GetUserName(),
				"chan",
				Channel.GetName(),
				Nick.GetNick(),
				"",
				CString(time(NULL)),
				"*** Parts: " + Nick.GetNick() + " (" + Nick.GetIdent() + "@" + Nick.GetHost() + ") (" + sMessage + ")"
			}
		);
	}

	EModRet OnUserNotice(CString& sTarget, CString& sMessage)
	{
		CIRCNetwork* pNetwork = GetNetwork();
		if (pNetwork)
		{
			InsertToDB(
				vector<CString> {
					GetUser()->GetUserName(),
					"notice",
					"",
					pNetwork->GetCurNick(),
					"",
					CString(time(NULL)),
					"->" + sTarget + " - " + pNetwork->GetCurNick() + "- " + sMessage
				}
			);
		}

		return CONTINUE;
	}

	EModRet OnPrivNotice(CNick& Nick, CString& sMessage)
	{
		InsertToDB(
			vector<CString> {
				GetUser()->GetUserName(),
				"notice",
				"",
				Nick.GetNick(),
				CString((Nick.GetIdent() + "@" + Nick.GetHost())),
				CString(time(NULL)),
				"-" + Nick.GetNick() + "- " + sMessage
			}
		);

		return CONTINUE;
	}

	EModRet OnChanNotice(CNick& Nick, CChan& Channel, CString& sMessage)
	{
		InsertToDB(
			vector<CString> {
				GetUser()->GetUserName(),
				"notice",
				Channel.GetName(),
				Nick.GetNick(),
				CString((Nick.GetIdent() + "@" + Nick.GetHost())),
				CString(time(NULL)),
				"-" + Nick.GetNick() + "- " + sMessage
			}
		);

		return CONTINUE;
	}

	virtual EModRet OnModuleUnloading(CModule* pModule, bool& bSuccess, CString& sRetMsg)
	{
		delete stmt;
		delete res;
		try {
			con->close();
			delete con;
		} catch (sql::SQLException &e) { }
		return CONTINUE;
	}

	virtual void InsertToDB(vector<CString> params)
	{
		CString query = "INSERT INTO chatlogs (znc_user, type, channel, nick, identhost, timestamp, message) VALUES (?, ?, ?, ?, ?, ?, ?)";
		unsigned int requiredParams = 0;
		for (unsigned int i = 0; i < query.size(); i++)
		{
			if (query[i] == '?') requiredParams++;
		}

		if (requiredParams != params.size())
		{
			PutModule("'" + query + "' failed: Received " + CString(params.size()) + " params");
			return;
		}

		try {
			prep_stmt = con->prepareStatement(query);
			for (unsigned int i = 0; i < params.size(); i++)
			{
				prep_stmt->setString(i + 1, params[i]);
			}
			prep_stmt->execute();
		} catch (sql::SQLException &e) {
			PutModule(CString(e.what()));
		}
	}

	virtual void Connect()
	{
		if (tries == 3)
		{
			return;
		}

		try {
			PutModule("connecting to " + conInfo["host"]);
			driver = get_driver_instance();
			con = driver->connect(conInfo["host"], conInfo["username"], conInfo["password"]);
			con->setSchema(conInfo["database"]);
			connected = true;
		} catch (sql::SQLException &e) {
			PutModule(CString(e.getErrorCode()));
			tries++;
		}
	}

private:
	sql::Driver *driver;
	sql::Connection *con;
	sql::Statement *stmt;
	sql::ResultSet *res;
	sql::PreparedStatement *prep_stmt;
	map<CString, CString> conInfo;
	bool connected = false;
	int tries = 0;
};

template<> void TModInfo<CMySQLLog>(CModInfo& Info) {
	Info.SetWikiPage("");
}

USERMODULEDEFS(CMySQLLog, "Log to MySQL");