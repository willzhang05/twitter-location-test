#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <curl/curl.h>

const char RAND_TABLE[63] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

std::string find_and_replace(std::string &s, const std::string &to_replace, const std::string &replace_with) {
	return s.replace(s.find(to_replace), to_replace.length(), replace_with);
}

std::string gen_alphanum(int len) {
	std::string buff;
	std::srand(time(0));
	for(int i = 0; i < len; i++) {
		buff += RAND_TABLE[std::rand() % 62];
	}
	return buff;
}


int main () {
	std::string line;
	std::string appinfo [4];
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
	std::string nonce = gen_alphanum(42);
	printf("%s\n", nonce.c_str());
	/*
	std::string command = "curl --get 'https://api.twitter.com/1.1/users/lookup.json' --data 'screen_name=" + username + "' --header 'Authorization: OAuth oauth_consumer_key=\"" + appinfo[0] + "\", oauth_nonce=\"14f3ace6df27f3ea8322ba20e3e509c5\", oauth_signature=\"h18hZk1pMPAc8uaxBNLqvc1fQLU\%3D\", oauth_signature_method=\"HMAC-SHA1\", oauth_timestamp=\"1457318794\", oauth_token=\"" + appinfo[2] + "\", oauth_version=\"1.0\"' --verbose > lookup.json";
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
