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
#include <cstdlib>
#include <stdio.h>
#include <fcntl.h>
using namespace std;

const int PORT = 21;
const int BUFFLEN = 1000;
const int DATABUFFLEN = 1000;
const char* const DELIMITER = "\r\n";

typedef vector<string> Command;

class FTPClient {
private:
	WSADATA startup;
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
	
	int recvControl(int stateCode, string errorInfo = "");
	int commandLine(std::string cmd);
	void removeSpace(std::string&);
	int listenServer(unsigned short&);

	int open(string);
	int user(string);
	int pass(string);
	int ls();
	int cd(string);
	int pasv();
	int actv();
	void put(string, string);
	void get(string, string);
	void del(string);
	void rmdir(string);
	void mkdir(string);
	void pwd();
	void help(string);
	int lcd(string);

	int ConnectServer(int portConnect);
public:
	FTPClient();
	~FTPClient();
	int DisconnectServer();
	vector<vector<string>> filelist;

	int togglePassiveMode(const Command&);
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
	int lcdUtil(const Command&);
};