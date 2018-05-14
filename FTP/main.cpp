#include "FTPClient.h"
#pragma comment(lib,"Ws2_32.lib")

void main() {
	FTPClient ftp;
	string type;
	bool isEndLoop = false;
	
	while (!isEndLoop) {
		cout << "ftp> ";
		getline(cin, type);
		if (type == "open") {
			ftp.Login();
			ftp.ConnectServer(PORT);
		}
		else if (type == "pasv") {
			ftp.convertPasv();
		}
		else if (type == "pwd") {
			ftp.pwdDir();
		}
		else if (type == "cd") {
			ftp.cd();
		}
		else if (type == "delete") {
			ftp.deleteFile();
		}
		else if (type == "mkdir") {
			ftp.mkdir();
		}
		else if (type == "rmdir") {
			ftp.rmdir();
		}
		else if (type == "ls") {
			ftp.ls();
		}
		else if (type == "put") {
			ftp.put();
		}
		else if (type == "get") {
			ftp.get();
		}
		else if (type == "help") {
			ftp.help();
		}
		else if (type == "quit") {
			ftp.DisconnectServer();
			isEndLoop = true;
		}
		else {
			cout << "Invalid command." << endl;
		}
	}
	
}