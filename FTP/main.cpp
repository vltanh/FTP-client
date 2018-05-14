#include "FTPClient.h"
#pragma comment(lib,"Ws2_32.lib")

vector<string> getCommand(string cmd) {
	vector<string> ret;
	istringstream split(cmd);
	do {
		string part;
		split >> part;
		ret.push_back(part);
	} while (split);
	while (ret.size() > 0 && ret.back() == "") ret.pop_back();
	return ret;
}

void main() {
	FTPClient ftp;
	string type;
	bool isEndLoop = false;
	
	while (!isEndLoop) {
		cout << "ftp> ";
		getline(cin, type);
		vector<string> cmd = getCommand(type);

		if (cmd.size() == 0)
			continue;

		if (cmd[0] == "open") {
			if (cmd.size() > 2) {
				cout << "Invalid command. Usage: open IP_address" << endl;
			}
			else {
				ftp.Login(cmd);
			}
		}
		else if (cmd[0] == "user") {
			if (cmd.size() > 2) {
				cout << "Usage: user username" << endl;
			}
			else {
				string username = (cmd.size() == 2) ? cmd[1] : "";
				ftp.user(username);
			}
		}
		else if (cmd[0] == "pass") {
			if (cmd.size() > 2) {
				cout << "Usage: pass password" << endl;
			}
			else {
				string password = (cmd.size() == 2) ? cmd[1] : "";
				ftp.pass(password);
			}
		}
		else if (cmd[0] == "pasv") {
			ftp.convertPasv();
		}
		else if (cmd[0] == "pwd") {
			ftp.pwdDir();
		}
		else if (cmd[0] == "cd") {
			ftp.cd();
		}
		else if (cmd[0] == "delete") {
			ftp.deleteFile();
		}
		else if (cmd[0] == "mkdir") {
			ftp.mkdir();
		}
		else if (cmd[0] == "rmdir") {
			ftp.rmdir();
		}
		else if (cmd[0] == "ls") {
			ftp.ls();
		}
		else if (cmd[0] == "put") {
			ftp.put();
		}
		else if (cmd[0] == "get") {
			ftp.get();
		}
		else if (cmd[0] == "help") {
			ftp.help();
		}
		else if (cmd[0] == "quit") {
			ftp.DisconnectServer();
			isEndLoop = true;
		}
		else {
			cout << "Invalid command." << endl;
		}
	}
	
}