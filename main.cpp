#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>

int main () {
	std::string line;
	std::string appinfo [4];
	int type = 0;
	std::ifstream infile("twitter.conf");
	if(infile.is_open()) {
		while(!infile.eof()) {
			std::getline(infile, line);
			if(
			printf("%s\n", line.c_str());
		}
	}
	infile.close();
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://wzhang.me");
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return 0;
}
