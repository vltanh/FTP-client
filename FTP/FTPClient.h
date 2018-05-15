#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <conio.h>
#include <stack>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

const int PORT = 21;
const int BUFFLEN = 1000;
const int DATABUFFLEN = 1000;
const char* const DELIMITER = "\r\n";

typedef vector<string> Command;

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
	bool m_connected; // check connected to server or not
	bool m_login; // check login completed or not
	string m_localDir = "/";
	
	int getStateCode();
	int getPort();
	void getFileSize(string filename);
	int listPwd();
	
	int recvControl(int stateCode, string errorInfo = "");
	int commandLine(std::string cmd);
	void removeSpace(std::string&);
public:
	FTPClient();
	~FTPClient();
	int ConnectServer(int portConnect);
	int DisconnectServer();
	int open(string);
	int user(string);
	int pass(string);
	int ls();
	int cd(string); // change dir
	int pasv();
	void put(string, string); // upload a file to server
	void get(string, string); // download a file from server
	void del(string); // delete a file on server
	void rmdir(string); // delete an empty folder on server
	void mkdir(string); // create an empty folder on server
	void pwd();
	void help(string);
	vector<vector<string>> filelist;

	int openUtil(const Command&);
	int userUtil(const Command&);
	int passUtil(const Command&);
	int lsUtil(const Command&);
	int cdUtil(const Command&);
	int pasvUtil(const Command&);
	int putUtil(const Command&);
	int getUtil(const Command&);
	int delUtil(const Command&);
	int rmdirUtil(const Command&);
	int mkdirUtil(const Command&);
	int pwdUtil(const Command&);
	int helpUtil(const Command&);
};

//int FTPClient::putUtil(const Command&);
//int FTPClient::getUtil(const Command&);