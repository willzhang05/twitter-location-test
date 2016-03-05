#include <stdio.h>
#include <curl/curl.h>

int main () {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "curl.haxx.se");
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return 0;
}
