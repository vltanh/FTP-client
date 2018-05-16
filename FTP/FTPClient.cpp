#include "FTPClient.h"

FTPClient::FTPClient() {
	m_IPAddr = m_Username = m_Password = "";
	m_passiveMode = false;
	// initializate socket
	// Use version socket: 2.2
	if (WSAStartup(MAKEWORD(2, 2), &startup) != 0) {
		throw string("Init Failed: ") + to_string(GetLastError());
		//system("pause");
	}
}

FTPClient::~FTPClient() {
	DisconnectServer();
}

/*
-- Function: get state code from response information
*/
int FTPClient::getStateCode() {
	int number = 0;
	char *temp = m_buff;
	while (temp != nullptr) {
		number = 10 * number + (*temp) - '0';
		temp++;
		if (*temp == ' ' || *temp == '-')
			break;
	}
	return number;
}

/*
-- This function to get port number for Passive Mode
*/
int FTPClient::getPort() {
	int number1 = 0, number2 = 0;
	char *temp = m_buff;
	int	count = 0;
	while (1) {
		if ((*temp) == ',')
			count++;
		temp++;
		if ((*temp) == ')')
			break;
		if (count == 4 && (*temp) != ',') {
			if (*temp >= '0' && *temp <= '9')
				number1 = 10 * number1 + (*temp) - '0';
		}
		if (count == 5) {
			if (*temp >= '0' && *temp <= '9')
				number2 = 10 * number2 + (*temp) - '0';
		}
	}
	
	return number1 * 256 + number2;
}

void FTPClient::getFileSize(string filename) {
	try {
		commandLine("SIZE " + filename);
		recvControl(213);
	}
	catch (string error) {
		throw error;
	}
	catch (int e) {
		throw e;
	}
	char *temp = m_buff;
	while (temp != nullptr && *temp != ' ')
		temp++;
	temp++;
	int number = 0;
	while (temp != nullptr && *temp != '\r') {
		number *= 10;
		number += (*temp - '0');
		temp++;
	}
	memset(m_buff, 0, BUFFLEN);
}

int FTPClient::recvControl(int stateCode, string errorInfo) {
	Sleep(300);
	if (m_nextInfo.size() == 0) {
		int code;
		Sleep(50);
		memset(m_buff, 0, BUFFLEN);
		m_recvInfo.clear();
		int infolen = recv(m_controlSocket, m_buff, BUFFLEN, 0);

		if (infolen == -1) {
			throw string("Not connected.");
			return -1;
		}
		else if (infolen == BUFFLEN) {
			throw string("ERROR! Too long information to receive!");
			return -1;
		}

		m_buff[infolen] = '\0';
		// get the code 
		code = getStateCode();
		m_recvInfo = m_buff;
		cout << m_recvInfo;

		int temp = m_recvInfo.find("\r\n226");
		
		if (temp >= 0) {
			m_nextInfo = m_recvInfo.substr(temp + 2);
		}
				
		if (code == stateCode)
			return 0;
		else {
			if (code >= 500 && code <= 599)
				throw - 1;
			return -1;
		}
	}
	else {
		m_recvInfo = m_nextInfo;
		m_nextInfo.clear();
		return 0;
	}
}

int FTPClient::commandLine(string cmd) {
	cmd += "\r\n";
	int cmdlen = cmd.size();
	//cout << cmd;
	send(m_controlSocket, cmd.c_str(), cmdlen, 0);
	return 0;
}

void FTPClient::removeSpace(string &str) {
	int p, q;
	p = str.find(' ');
	while (p >= 0) {
		for (q = p + 1; str[q] == ' '; q++);
		str.erase(p + 1, q - p - 1);
		p = str.find(' ', p + 1);
	}
}

int FTPClient::ConnectServer(int portConnect) {
	int connection;
	// Create Socket
	m_controlSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_controlSocket == INVALID_SOCKET) {
		throw string("Creating Control Socket Failed. ") + to_string(GetLastError());
		//system("pause");
		return -1;
	}
	// Build server access parameter structure
	m_serverAddr.sin_family = AF_INET;
	// Get address
	m_serverAddr.sin_addr.S_un.S_addr = inet_addr(m_IPAddr.c_str()); 
	// Get port. htons: converts a port number in host byte order to 
	// a port number in network byte order
	m_serverAddr.sin_port = htons(portConnect);
	memset(m_serverAddr.sin_zero, 0, sizeof(m_serverAddr.sin_zero));
	
	// Create a connection ...
	connection = connect(m_controlSocket, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
	if (connection == SOCKET_ERROR) {
		throw string("Control Socket Connecting Failed. ") + to_string(GetLastError());
		//system("pause");
		return -1;
	}
	cout << "Control Socket Connecting Succeeded. " << endl;

	// Receive return status information
	recvControl(220);

	return 0;
}

int FTPClient::DisconnectServer() {
	try {
		commandLine("QUIT");
		recvControl(221);
	}
	catch (string error) {
		if (error != "Not connected.") {
			m_IPAddr = m_Username = m_Password = m_Info = "";
			filelist.clear();
			memset(m_buff, 0, BUFFLEN);
			memset(m_databuff, 0, DATABUFFLEN);
			closesocket(m_dataSocket);
			closesocket(m_controlSocket);
			WSACleanup();
			throw error;
		}
	}
	return 0;

}

//  --------------------
// | Supported Commands |
//  --------------------

// --- open: connect and login to server
int FTPClient::openUtil(const Command& cmd) {
	if (cmd.size() > 2) {
		throw string("Invalid command. Usage: open IP_address");
	}
	else {
		string ip;
		if (cmd.size() > 1) {
			ip = cmd[1];
		}
		else {
			cout << "To ";
			getline(cin, ip);
		}
		if (ip == "localhost")
			ip = "127.0.0.1";
		this->m_IPAddr = ip;

		try {
			open(ip);
		}
		catch (string error) {
			throw error;
		}
	}
	return 0;
}

int FTPClient::open(string ip) {
	if (ConnectServer(PORT) == -1) {
		throw string("Cannot connect to server.");
	}

	// Get Username
	string username;
	cout << "Username: ";
	getline(cin, username);
	user(username);

	return 0;
}

// --- user: input username
int FTPClient::userUtil(const Command& cmd) {
	if (cmd.size() > 2) {
		cout << "Usage: user username" << endl;
	}
	else {
		string username = (cmd.size() == 2) ? cmd[1] : "";
		try {
			user(username);
		}
		catch (string error) {
			throw error;
		}
	}
	return 0;
}

int FTPClient::user(string user) {
	this->m_Username = user;
	commandLine("USER " + m_Username);
	if (recvControl(331) == -1) {
		throw string("Login failed.");
		return -1;
	}

	string password;
	cout << "Password: ";
	char ch;
	ch = _getch();
	while (ch != 13) {
		if (ch != 8) {
			password.push_back(ch);
		}
		else {
			if (password.size() > 0) 
				password.pop_back();
		}
		ch = _getch();
	}
	cout << endl;
	try {
		pass(password);
	}
	catch (string error) {
		throw error;
	}
}

// --- pass: input password
int FTPClient::passUtil(const Command& cmd) {
	if (cmd.size() > 2) {
		cout << "Usage: pass password" << endl;
	}
	else {
		string password = (cmd.size() == 2) ? cmd[1] : "";
		try {
			pass(password);
		}
		catch (string error) {
			throw error;
		}
	}
	return 0;
}

int FTPClient::pass(string pass) {
	this->m_Password = pass;
	commandLine("PASS " + m_Password);
	if (recvControl(230) == -1) {
		throw string("Login failed.");
		return -1;
	}
	return 0;
}

// --- ls: list the files/folders of the current directory
int FTPClient::lsUtil(const Command& cmd) {
	if (cmd.size() > 1) {
		throw "Invalid command. Usage: ls";
	}
	else {
		try {
			ls();
		}
		catch (string error) {
			throw error;
		}
	}
}

int FTPClient::ls() {
	try {
		if (m_passiveMode) pasv();
		else actv();

		commandLine("LIST -al");
		recvControl(150);
		memset(m_databuff, 0, DATABUFFLEN);
		string fulllist;
		int ret = recv(m_dataSocket, m_databuff, DATABUFFLEN - 1, 0);
		while (ret > 0) {
			m_databuff[ret] = '\0';
			fulllist += m_databuff;
			ret = recv(m_dataSocket, m_databuff, DATABUFFLEN - 1, 0);
		}

		//removeSpace(fulllist);

		int lastp, lastq, p, q;
		vector<string> eachrow;
		string rawrow;
		string item;
		filelist.clear();
		p = fulllist.find("\r\n");
		lastp = 0;
		while (p >= 0) {
			eachrow.clear();
			rawrow = fulllist.substr(lastp, p - lastp);

			q = rawrow.find(' ');
			lastq = 0;
			for (int i = 0; i < 8; i++) {
				item = rawrow.substr(lastq, q - lastq);
				eachrow.push_back(item);
				lastq = q + 1;
				q = rawrow.find(' ', lastq);
			}
			item = rawrow.substr(lastq);
			eachrow.push_back(item);
			filelist.push_back(eachrow);

			lastp = p + 2;
			p = fulllist.find("\r\n", lastp);
		}
		for (auto const& dir : filelist) {
			for (auto const& temp : dir)
				cout << temp << " ";
			cout << endl;
		}
		closesocket(m_dataSocket);
		recvControl(226);
	}
	catch (string error) {
		throw error;
	}
	return 0;
}

// --- cd: change working directory server side
int FTPClient::cdUtil(const Command& cmd) {
	if (cmd.size() > 2) {
		throw string("Invalid command. Usage: cd directory");
	}
	else {
		string dir;
		if (cmd.size() > 1) {
			dir = cmd[1];
		}
		else {
			cout << "Directory? ";
			getline(cin, dir);
		}
		try {
			cd(dir);
		}
		catch (string error) {
			throw error;
		}
	}
}

int FTPClient::cd(string dir) {
	try {
		memset(m_buff, 0, BUFFLEN);
		commandLine("CWD " + dir);
		recvControl(250);
	}
	catch (string error) {
		throw error;
	}
}

// --- pwd: show working directory server side
int FTPClient::pwdUtil(const Command& cmd) {
	if (cmd.size() > 1) {
		throw "Invalid command. Usage: pwd";
	}
	else {
		try {
			pwd();
		}
		catch (string error) {
			throw error;
		}
	}
}

void FTPClient::pwd() {
	try {
		commandLine("PWD ");
		recvControl(257);
	}
	catch (string error) {
		throw error;
	}
}

/*
-- This function to upload file to server
-- Input: @param: the name of local file in server; 
		maybe @param is a directory
-- Output: file stored in server directory
-- Exemple: C:/Users/DELL/Downloads/FTP/test3.txt
*/
int FTPClient::putUtil(const Command& cmd) {
	if (cmd.size() > 3) {
		throw string("Invalid command. Usage: put directory name");
	}
	else {
		string localName, localFileName;
		if (cmd.size() == 1) {
			cout << "Local file: ";
			getline(cin, localName);

			cout << "Remote file: ";
			getline(cin, localFileName);
		}
		else if (cmd.size() == 2) {
			localName = cmd[1];

			cout << "Remote file: ";
			getline(cin, localFileName);
		}
		else {
			localName = cmd[1];
			localFileName = cmd[2];
		}

		while (localName.find("\"") != -1) {
			localName.erase(localName.begin() + localName.find("\""));
		}

		if (localFileName == "") {
			int temp = max(int(localName.find_last_of("/")), int(localName.find_last_of("\\")));
			localFileName = localName.substr(temp + 1);
		}

		try {
			put(localName, localFileName);
		}
		catch (string error) {
			throw error;
		}
	}
	return 0;
}

void FTPClient::put(string localName, string localFileName) {
	try {
		FILE *file = fopen(localName.c_str(), "rb");
		if (!file) {
			throw string("File not found!");
		}

		pasv();
		commandLine("STOR " + localFileName);
		recvControl(150);
		int count;
		while (!feof(file)) {
			count = fread(m_databuff, 1, DATABUFFLEN, file);
			send(m_dataSocket, m_databuff, count, 0);
		}
		memset(m_databuff, 0, DATABUFFLEN);
		send(m_dataSocket, m_databuff, 1, 0);
		fclose(file);
		closesocket(m_dataSocket);
		if (recvControl(226) == -1)
			throw string("Error!");
	}
	catch (string error) {
		throw error;
	}
}

/*
-- This function to download file from server
-- Input: @param1: the name of file in server;
		  @param2: the dir to save downloaded file
-- Output: file stored in local dir
-- Example: "\\test1.txt", "C:/Users/DELL/Downloads"
*/
int FTPClient::getUtil(const Command& cmd) {
	if (cmd.size() > 3) {
		throw string("Invalid command. Usage: put directory name");
	}
	else {
		string localName, remoteName;
		if (cmd.size() == 1) {
			cout << "Remote file: ";
			getline(cin, remoteName);

			cout << "Local file: ";
			getline(cin, localName);
		}
		else if (cmd.size() == 2) {
			remoteName = cmd[1];

			cout << "Local file: ";
			getline(cin, localName);
		}
		else {
			remoteName = cmd[1];
			localName = cmd[2];
		}
		
		if (remoteName == "") {
			throw string("Invalid input.");
		}

		if (localName == "") {
			localName = remoteName;
		}
		localName = string(_getcwd(NULL, 0)) + "\\" + localName;

		try {
			get(remoteName, localName);
		}
		catch (string error) {
			throw error;
		}
		catch (int e) {
			throw e;
		}
	}
	return 0;
}

void FTPClient::get(string remoteName, string localName) {
	try {
		pasv();
		getFileSize(remoteName);
		commandLine("RETR " + remoteName);
		recvControl(150);
	}
	catch (string error) {
		throw error;
	}
	catch (int e) {
		throw e;
	}
	memset(m_databuff, 0, DATABUFFLEN);
	ofstream openFile;
	openFile.open(localName, ios_base::binary);
	int ret = recv(m_dataSocket, m_databuff, DATABUFFLEN, 0);
	while (ret > 0) {
		openFile.write(m_databuff, ret);
		ret = recv(m_dataSocket, m_databuff, DATABUFFLEN, 0);
	}
	openFile.close();
	closesocket(m_dataSocket);
	try {
		recvControl(226);
	}
	catch (string e) {
		throw e;
	}
}
/*
-- This function to delete a file on server
-- @param: a directory to that file
-- example: \\test1.txt
*/
int FTPClient::delUtil(const Command& cmd) {
	if (cmd.size() > 2) {
		throw string("Invalid command. Usage: delete directory");
	}
	else {
		string name;
		if (cmd.size() > 1) {
			name = cmd[1];
		}
		else {
			cout << "Directory? ";
			getline(cin, name);
		}
		try {
			del(name);
		}
		catch (string error) {
			throw error;
		}
	}
}

void FTPClient::del(string dir) {
	commandLine("DELE " + dir);
	recvControl(250);
}

//-- This function to create an empty folder on server
//-- @param: the name of folder that is created on server
//-- Example: Test;
int FTPClient::mkdirUtil(const Command& cmd) {
	if (cmd.size() > 2) {
		throw string("Invalid command. Usage: mkdir directory");
	}
	else {
		string name;
		if (cmd.size() > 1) {
			name = cmd[1];
		}
		else {
			cout << "Directory? ";
			getline(cin, name);
		}
		try {
			mkdir(name);
		}
		catch (string error) {
			throw error;
		}
	}
}

void FTPClient::mkdir(string name) {
	try {
		commandLine("MKD " + name);
		recvControl(250);
	}
	catch (string error) {
		throw error;
	}
}

//-- This function to delete an empty folder on server
//-- @param: the name of folder that is deleted on server
//-- Example: Test
int FTPClient::rmdirUtil(const Command& cmd) {
	if (cmd.size() > 2) {
		throw string("Invalid command. Usage: rmdir directory");
	}
	else {
		string name;
		if (cmd.size() > 1) {
			name = cmd[1];
		}
		else {
			cout << "Directory? ";
			getline(cin, name);
		}
		try {
			rmdir(name);
		}
		catch (string error) {
			throw error;
		}
	}
}

void FTPClient::rmdir(string name) {
	commandLine("RMD " + name);
	recvControl(250);
}

// -- This command sends Help menu
// -- @param: command that the user need help with
// -- Example: help / help ls
int FTPClient::helpUtil(const Command& cmd) {
	if (cmd.size() > 2) {
		throw string("Invalid command. Usage: help command");
	}
	else {
		if (cmd.size() == 1) {
			string h;
			cout << "   ls \t put \t get \t cd \t delete \n   pwd \t pasv \t rmdir \t mkdir \t quit \n";
			cout << "Choose the command you need help with: ";
			getline(cin, h);
			help(h);
		}
		else {
			help(cmd[1]);
		}
	}
	return 0;
}

void FTPClient::help(string h) {
	vector<string> helpLists = { "ls", "put", "get", "cd", "delete", "mkdir", "rmdir", "pwd", "pasv", "quit" };
	vector<string> helpInfos = {
		"Returns information of a file or directory if specified.",
		"Upload a file to server.",
		"Download a file from server.",
		"Change working directory.",
		"Delete a file on server.",
		"Create an empty folder on server.",
		"Delete an empty folder on server.",
		"Print working directory. Returns the current directory of the host.",
		"Convert Passive Mode.",
		"Disconnect."
	};

	int pos = find(helpLists.begin(), helpLists.end(), h) - helpLists.begin();
	if (pos >= helpLists.size()) {
		cout << "No such command found." << endl;
	}
	else {
		//cout << setw(10) << left << "Command";
		//cout << setw(25) << left << "Description" << endl;
		//cout << setfill('-');
		//cout << setw(75) << '-' << endl;
		//cout << setfill(' ');

		cout << setw(10) << left << helpLists[pos];
		cout << setw(25) << left << helpInfos[pos];
		cout << endl;
	}
}


// 
int FTPClient::pasvUtil(const Command& cmd) {
	if (cmd.size() > 1) {
		throw "Invalid command. Usage: pasv";
	}
	else {
		try {
			pasv();
		}
		catch (string error) {
			throw error;
		}
	}
}

int FTPClient::pasv() {
	try {
		int dataPort, ret;
		// Chuyển sang PASSIVE
		commandLine("PASV");
		if (recvControl(227) == -1) {
			throw string("Connecting In Passive Mode Failed: ") + to_string(GetLastError());
		}
		// 227 Entering Passive Mode (h1, h2, h3, h4, p1, p2)
		// h1, h2, h3, h4 is host address (Ex: 127.0.0.1)
		// p1 * 256 + p2 is data port
		// so, value mode is (127, 0, 0, 1, p1, p2)
		dataPort = getPort();

		// Create a connection in passive mode	
		m_dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_dataSocket == -1) {
			throw string("Connecting In Passive Mode Failed: ") + to_string(GetLastError());
		}
		m_serverAddr.sin_port = htons(dataPort); // Change the port value in connection parameters
		ret = connect(m_dataSocket, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));

		if (ret == SOCKET_ERROR) {
			throw string("Connecting In Passive Mode Failed: ") + to_string(GetLastError());
			return -1;
		}
		cout << "Connecting In Passive Mode Succeeded." << endl;
		m_passiveMode = true;
	}
	catch (string error) {
		throw error;
	}
	return 0;
}

int FTPClient::listenServer(unsigned short& port) {
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		throw string("Cannot create socket.");
	}
	SOCKADDR_IN clientSockAddr;
	memset((void*)&clientSockAddr, 0, sizeof(clientSockAddr));
	clientSockAddr.sin_family = AF_INET;
	clientSockAddr.sin_addr.s_addr = INADDR_ANY;
	clientSockAddr.sin_port = htons(0);
	if (bind(listenSocket, (sockaddr*)&clientSockAddr, sizeof(clientSockAddr)) == SOCKET_ERROR) {
		closesocket(listenSocket);
		throw string("Cannot bind socket.");
	}
	if (listen(listenSocket, 1) == SOCKET_ERROR) {
		closesocket(listenSocket);
		throw string("Cannot listen.");
	}
	int len = sizeof(clientSockAddr);
	getsockname(listenSocket, (sockaddr*)&clientSockAddr, &len);
	port = ntohs(clientSockAddr.sin_port);
	return listenSocket;
}

int FTPClient::actv() {
	SOCKADDR_IN clientSockAddr;
	int len = sizeof(clientSockAddr);
	getsockname(m_controlSocket, (sockaddr*)&clientSockAddr, &len);
	unsigned int ip = clientSockAddr.sin_addr.s_addr;
	unsigned char *ip_char = (unsigned char*)&ip;
	unsigned short port;
	unsigned char *port_char = (unsigned char*)&port;
	try {
		SOCKET listenSocket = listenServer(port);
		commandLine("PORT " + to_string(ip_char[0]) + "," + to_string(ip_char[1]) + "," + to_string(ip_char[2]) + "," + to_string(ip_char[3]) + "," + to_string(port_char[1]) + "," + to_string(port_char[2]));
		if (recvControl(200) == -1) {
			throw string("Connecting In Active Mode Failed: ") + to_string(GetLastError());
		}
		SOCKADDR_IN adr;
		int len = sizeof(adr);
		m_dataSocket = accept(listenSocket, (sockaddr*)&adr, &len);
	}
	catch (string error) {
		throw error;
	}
	catch (int e) {

	}
	
	m_passiveMode = false;
	return 0;
}