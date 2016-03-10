#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <curl/curl.h>

const char ASCII_TABLE[67] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";

std::string find_and_replace(std::string &s, const std::string &to_replace, const std::string &replace_with) {
	return s.replace(s.find(to_replace), to_replace.length(), replace_with);
}

std::string gen_alphanum(int len) {
	std::string buff;
	std::srand(time(0));
	for(int i = 0; i < len; i++) {
		buff += ASCII_TABLE[std::rand() % 62];
	}
	return buff;
}


int main () {
	std::string username;
	std::cout << "Enter Twitter Username: ";
	getline(std::cin, username);
	std::cout << "\n";
		
	std::string app_info [8] = {username, "", gen_alphanum(42), "", "HMAC_SHA1", std::to_string(time(0)), "", "1.0"};
	std::string url = "https://api.twitter.com/1.1/users/lookup.json";
	
	std::string line;
	std::ifstream infile("twitter.conf");
	while(std::getline(infile, line)) {
		if(line.find("ckey") != std::string::npos) {
			find_and_replace(line, "ckey=", "");
			app_info[1] = line;
		} else if(line.find("atoken") != std::string::npos) {
			find_and_replace(line, "atoken=", "");
			app_info[7] = line;
		} else {
			continue;
		}
		printf("%s\n", line.c_str());
	}
	
	std::string buff;
	CURL *curl;
	for(int i = 0; i < 8; i++) {
		buff = std::string(curl_easy_escape(curl, app_info[i].c_str(), app_info[i].length()), app_info[i].length()).c_str();
		std::cout << buff + "\n";
	}

	/*std::string command = "curl --get 'https://api.twitter.com/1.1/users/lookup.json' --data 'screen_name=" + username + "' --header 'Authorization: OAuth oauth_consumer_key=\"" + appinfo[0] + "\", oauth_nonce=\"" + nonce + "\", oauth_signature=\"h18hZk1pMPAc8uaxBNLqvc1fQLU\%3D\", oauth_signature_method=\"HMAC-SHA1\", oauth_timestamp=\"1457318794\", oauth_token=\"" + appinfo[2] + "\", oauth_version=\"1.0\"' --verbose > lookup.json";
	std::system(command.c_str());
	*/
	//std::system("cat lookup.json");
	/*
	std::string url = "https://twitter.com/" + username + "#page-container";
	CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }*/
    return 0;
}
