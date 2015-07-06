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
		InsertToDb("chan", Nick.GetNick(), CString((Nick.GetIdent() + "@" + Nick.GetHost())), sMessage);
		return CONTINUE;
	}

	virtual EModRet OnModuleUnloading(CModule* pModule, bool& bSuccess, CString& sRetMsg)
	{
		delete con;
		delete stmt;
		delete res;
		try {
			con->close();
			delete con;
		} catch (sql::SQLException &e) { }
		return CONTINUE;
	}

	virtual void InsertToDb(CString type, CString Nick, CString identhost, CString& sMessage)
	{
		try {
			prep_stmt = con->prepareStatement("INSERT INTO chatlogs (znc_user, type, sender, identhost, timestamp, message) VALUES (?, ?, ?, ?, ?, ?)");
			prep_stmt->setString(1, GetUser()->GetUserName());
			prep_stmt->setString(2, type);
			prep_stmt->setString(3, Nick);
			prep_stmt->setString(4, identhost);
			prep_stmt->setString(5, CString(time(NULL)));
			prep_stmt->setString(6, sMessage.StripControls());
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