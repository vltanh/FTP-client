#include "FTPClient.h"

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
	commandLine("SIZE " + filename);
	recvControl(213);
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

int FTPClient::listPwd() {
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
	return 0;
}

int FTPClient::convertPasv() {
	int dataPort, ret;
	// Chuyển sang PASSIVE
	commandLine("PASV");
	recvControl(227);
	// 227 Entering Passive Mode (h1, h2, h3, h4, p1, p2)
	// h1, h2, h3, h4 is host address (Ex: 127.0.0.1)
	// p1 * 256 + p2 is data port
	// so, value mode is (127, 0, 0, 1, p1, p2)
	dataPort = getPort(); 
	
	// Create a connection in passive mode	
	m_dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	m_serverAddr.sin_port = htons(dataPort); // Change the port value in connection parameters
	ret = connect(m_dataSocket, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
	
	if (ret == SOCKET_ERROR) {
		cout << "Connecting In Passive Mode Failed: " << GetLastError() << endl;
		return -1;
	}
	cout << "Connecting In Passive Mode Successed." << endl;
	m_passiveMode = true;
	return 0;
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
			cout << "Not connected." << endl;
			return -1;
		}

		if (infolen == BUFFLEN) {
			cout << "ERROR! Too long information to receive!" << endl;
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
			cout << errorInfo;
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

FTPClient::FTPClient() {
	m_IPAddr, m_Username, m_Password = "";
	m_passiveMode = false;
}

FTPClient::~FTPClient() {
	DisconnectServer();
}

int FTPClient::ConnectServer(int portConnect) {
	WSADATA startup;
	int connection;
	// initializate socket
	// Use version socket: 2.2
	if (WSAStartup(MAKEWORD(2, 2), &startup) != 0) {
		cout << "Init Failed: " << GetLastError() << endl;
		system("pause");
		return -1;
	}
	// Create Socket
	m_controlSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_controlSocket == INVALID_SOCKET) {
		cout << "Creating Control Socket Failed. " << GetLastError() << endl;
		system("pause");
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
		cout << "Control Socket Connecting Failed. " << GetLastError() << endl;
		//system("pause");
		return -1;
	}
	cout << "Control Socket Connecting Succeeded. " << endl;

	// Receive return status information
	recvControl(220);

	return 0;
}

int FTPClient::DisconnectServer() {
	commandLine("QUIT");
	recvControl(221);
	m_IPAddr, m_Username, m_Password, m_Info = "";
	filelist.clear();
	memset(m_buff, 0, BUFFLEN);
	memset(m_databuff, 0, DATABUFFLEN);
	closesocket(m_dataSocket);
	closesocket(m_controlSocket);
	WSACleanup();
	return 0;

}

int FTPClient::Login(const vector<string>& cmd) {
	// Get IP
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

	if (ConnectServer(PORT) == -1) {
		return -1;
	}

	// Get Username
	string username;
	cout << "Username: ";
	getline(cin, username);
	user(username);

	return 0;
}

int FTPClient::user(string user) {
	this->m_Username = user;
	commandLine("USER " + m_Username);
	if (recvControl(331) == -1) {
		cout << "Login failed." << endl;
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
	pass(password);
}

int FTPClient::pass(string pass) {
	this->m_Password = pass;
	commandLine("PASS " + m_Password);
	if (recvControl(230) == -1) {
		return -1;
	}
}

/*
--This function to navigate to a different directory on server
*/
void FTPClient::cd() {
	string dir;
	cout << "Directory? ";
	getline(cin, dir);
	memset(m_buff, 0, BUFFLEN);
	commandLine("CWD " + dir);
	recvControl(250);
}

void FTPClient::ls() {
	listPwd();
}

void FTPClient::pwdDir() {
	commandLine("PWD ");
	recvControl(257);
}
/*
-- This function to upload file to server
-- Input: @param: the name of local file in server; 
		maybe @param is a directory
-- Output: file stored in server directory
-- Exemple: C:/Users/DELL/Downloads/FTP/test3.txt
*/
void FTPClient::put() {
	string localName;
	cout << "Directory? ";
	getline(cin, localName);
	FILE *file = fopen(localName.c_str(), "rb");
	if (!file) {
		cout << "File not found! \n";
		return;
	}

	string localFileName;
	// Find last "/"
	int temp = localName.find_last_of("/");
	// To get name of file that is sent to server
	localFileName = localName.substr(temp + 1);
	// Convert to passive mode
	// convertPasv();
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
	recvControl(226);
}

/*
-- This function to download file from server
-- Input: @param1: the name of file in server;
		  @param2: the dir to save downloaded file
-- Output: file stored in local dir
-- Example: "\\test1.txt", "C:/Users/DELL/Downloads"
*/
void FTPClient::get() {
	string serverName; string localDir;
	cout << "Directory? Server:  ";
	getline(cin, serverName);
	cout << "Local: "; getline(cin, localDir);
	string localFilename = localDir + "/" + serverName;
	ofstream openFile;
	openFile.open(localFilename, ios_base::binary);
	//convertPasv();
	getFileSize(serverName);
	commandLine("RETR " + serverName);
	recvControl(150);
	memset(m_databuff, 0, DATABUFFLEN);
	int ret = recv(m_dataSocket, m_databuff, DATABUFFLEN, 0);
	while (ret > 0) {
		openFile.write(m_databuff, ret);
		ret = recv(m_dataSocket, m_databuff, DATABUFFLEN, 0);
	}
	openFile.close();
	closesocket(m_dataSocket);
	recvControl(226);
}
/*
-- This function to delete a file on server
-- @param: a directory to that file
-- example: \\test1.txt
*/
void FTPClient::deleteFile() {
	string serverName;
	cout << "Directory? ";
	getline(cin, serverName);
	commandLine("DELE " + serverName);
	recvControl(250);
}

/*
-- This function to delete an empty folder on server
-- @param: the name of folder that is deleted on server
-- Example: Test
*/
void FTPClient::rmdir() {
	string serverDir;
	cout << "Directory? ";
	getline(cin, serverDir);
	commandLine("RMD " + serverDir);
	recvControl(250);
}

/*
-- This function to create an empty folder on server
-- @param: the name of folder that is created on server
-- Example: Test;
*/
void FTPClient::mkdir() {
	string serverNameFolder;
	cout << "Directory? ";
	getline(cin, serverNameFolder);
	commandLine("MKD " + serverNameFolder);
	recvControl(250);
}

void FTPClient::help() {
	cout << setw(10) << left << "Command";
	cout << setw(25) << left << "Description" << endl;
	cout << setfill('-');
	cout << setw(75) << '-' << endl;
	cout << setfill(' ');
	cout << setw(10) << left << "ls";
	cout << setw(25) << left << "Returns information of a file or directory if specified." << endl;
	cout << setw(10) << left << "put";
	cout << setw(25) << left << "Upload a file to server." << endl;
	cout << setw(10) << left << "get";
	cout << setw(25) << left << "Download a file from server." << endl;
	cout << setw(10) << left << "cd";
	cout << setw(25) << left << "Change working directory." << endl;
	cout << setw(10) << left << "delete";
	cout << setw(25) << left << "Delete a file on server." << endl;
	cout << setw(10) << left << "mkdir";
	cout << setw(25) << left << "Create an empty folder on server." << endl;
	cout << setw(10) << left << "rmdir";
	cout << setw(25) << left << "Delete an empty folder on server." << endl;
	cout << setw(10) << left << "pwd";
	cout << setw(25) << left << "Print working directory. Returns the current directory of the host." << endl;
	cout << setw(10) << left << "pasv";
	cout << setw(25) << left << "Convert Passive Mode." << endl;
	cout << setw(10) << left << "quit";
	cout << setw(25) << left << "Disconnect." << endl;
}

