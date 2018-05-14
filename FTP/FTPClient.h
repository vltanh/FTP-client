#pragma once

#include <WinSock2.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <conio.h>
using namespace std;

const int PORT = 21;
const int BUFFLEN = 1000;
const int DATABUFFLEN = 1000;
const char* const DELIMITER = "\r\n";

class FTPClient {
private:
	SOCKADDR_IN m_serverAddr;
	string m_IPAddr;
	string m_Username;
	string m_Password;
	string m_Info;
	char* m_buff = new char[BUFFLEN];
	char* m_databuff = new char[DATABUFFLEN];
	SOCKET m_controlSocket; // Connection Control
	SOCKET m_dataSocket; // Connection Data
	string m_recvInfo;
	string m_nextInfo;
	bool m_passiveMode; // to check passive mode or not
	
	int getStateCode();
	int getPort();
	void getFileSize(string filename);
	int listPwd();
	
	int recvControl(int stateCode, string errorInfo = "0");
	int commandLine(std::string cmd);
	void removeSpace(std::string&);
public:
	FTPClient();
	~FTPClient();
	int ConnectServer(int portConnect);
	int DisconnectServer();
	void Login();
	void ls(); // list the list of folder, file on server
	void cd(); // change dir
	int convertPasv();
	void put(); // upload a file to server
	void get(); // download a file from server
	void deleteFile(); // delete a file on server
	void rmdir(); // delete an empty folder on server
	void mkdir(); // create an empty folder on server
	void pwdDir();
	void help();
	vector<vector<string>> filelist;
};