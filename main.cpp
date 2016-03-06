#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>

std::string find_and_replace(std::string &s, const std::string &to_replace, const std::string &replace_with) {
	return s.replace(s.find(to_replace), to_replace.length(), replace_with);
}

int main () {
	std::string line;
	std::string appinfo [4];
	int type = 0;
	std::ifstream infile("twitter.conf");
	while(std::getline(infile, line)) {
		if(line.find("ckey") != std::string::npos) {
			find_and_replace(line, "ckey=", "");
			appinfo[0] = line;
		} else if(line.find("csecret") != std::string::npos) {
			find_and_replace(line, "csecret=", "");
			appinfo[1] = line;
		} else if(line.find("atoken") != std::string::npos) {
			find_and_replace(line, "atoken=", "");
			appinfo[2] = line;
		} else if(line.find("asecret") != std::string::npos) {
			find_and_replace(line, "asecret=", "");
			appinfo[3] = line;
		} else {
			continue;
		}
		printf("%s\n", line.c_str());
	}
    std::string username;
	std::cout << "Enter Twitter Username: ";
	getline(std::cin, username);
	std::cout << "\n";
	std::string url = "https://twitter.com/" + username + "#page-container";
	CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return 0;
}
