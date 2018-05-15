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
		try {
			cout << "ftp> ";
			getline(cin, type);
			vector<string> cmd = getCommand(type);

			if (cmd.size() == 0)
				continue;

			if (cmd[0] == "open") {
				ftp.openUtil(cmd);
			}
			else if (cmd[0] == "user") {
				ftp.userUtil(cmd);
			}
			else if (cmd[0] == "pass") {
				ftp.passUtil(cmd);
			}
			else if (cmd[0] == "ls") {
				ftp.lsUtil(cmd);
			}
			else if (cmd[0] == "cd") {
				ftp.cdUtil(cmd);
			}
			else if (cmd[0] == "pwd") {
				ftp.pwdUtil(cmd);
			}
			else if (cmd[0] == "mkdir") {
				ftp.mkdirUtil(cmd);
			}
			else if (cmd[0] == "rmdir") {
				ftp.rmdirUtil(cmd);
			}
			else if (cmd[0] == "delete") {
				ftp.delUtil(cmd);
			}
			else if (cmd[0] == "pasv") {
				ftp.pasv();
			}
			else if (cmd[0] == "put") {
				ftp.putUtil(cmd);
			}
			else if (cmd[0] == "get") {
				ftp.getUtil(cmd);
			}
			else if (cmd[0] == "help") {
				ftp.helpUtil(cmd);
			}
			else if (cmd[0] == "quit") {
				ftp.DisconnectServer();
				isEndLoop = true;
			}
			else {
				cout << "Invalid command." << endl;
			}
		}
		catch (string error) {
			cout << error << endl;
		}
		catch (int e) {}
	}
	
}