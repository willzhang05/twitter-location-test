#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>

using std::string;
using std::cout;
using std::endl;
using std::strcpy;

struct pair {
    string key, value;
    pair() {}
    pair(string k, string v) {
        key = k;
        value = v;
    }
    bool operator<(const pair &other) const {
        if (key == other.key) {
            return value < other.value;
        }
        return key < other.key;
    }
    string to_string() { return key + "=" + value; }
};

const char ASCII_TABLE[67] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";

string find_and_replace(string &s, const string &to_replace, const string &replace_with) {
    return s.replace(s.find(to_replace), to_replace.length(), replace_with);
}

string gen_alphanum(int len) {
    string buff;
    std::srand(time(0));
    for (int i = 0; i < len; i++) {
        buff += ASCII_TABLE[std::rand() % 62];
    }
    return buff;
}

unsigned char *hmac_sha1(unsigned char *key, unsigned char *data, int key_size, int data_size) {
    unsigned char *digest;
    digest = HMAC(EVP_sha1(), key, key_size, data, data_size, NULL, NULL);
    /*for (int i = 0; i < 20; i++) {
        printf("%x", digest[i]);
    }*/
    // printf("\n");
    return digest;
}

string base64(unsigned char *data, int data_size) {
    BIO *b64, *bmem;
    char *buff;
    b64 = BIO_new(BIO_f_base64());               // BIO to perform b64 encode
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);  // No newlines in data
    bmem = BIO_new(BIO_s_mem());                 // BIO to hold result
    BIO_push(b64, bmem);                         // Chains b64 to bmem
    BIO_write(b64, data, data_size + 1);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    long len = BIO_get_mem_data(bmem, &buff);
    string ret = string(buff, len - 1);
    while (ret.size() < (unsigned long)len) {
        ret += "=";
    }
    BIO_free_all(b64);
    return ret;
}

int get_json(string &username, string &outfile) {
    pair app_info[8] = {pair("screen_name", username),
                        pair("oauth_consumer_key", ""),
                        pair("oauth_nonce", gen_alphanum(42)),
                        pair("oauth_signature_method", "HMAC-SHA1"),
                        pair("oauth_timestamp", std::to_string(time(0))),
                        pair("oauth_token", ""),
                        pair("oauth_version", "1.0"),
                        pair("oauth_signature", "")};
    string secrets[2];
    string line;
    std::ifstream infile("twitter.conf");
    while (std::getline(infile, line)) {
        if (line.find("ckey") != string::npos) {
            find_and_replace(line, "ckey=", "");
            app_info[1].value = line;
        } else if (line.find("csecret") != string::npos) {
            find_and_replace(line, "csecret=", "");
            secrets[0] = line;
        } else if (line.find("atoken") != string::npos) {
            find_and_replace(line, "atoken=", "");
            app_info[5].value = line;
        } else if (line.find("asecret") != string::npos) {
            find_and_replace(line, "asecret=", "");
            secrets[1] = line;
        } else {
            continue;
        }
    }
    pair encode_info[7];
    CURL *curl = curl_easy_init();
    char *temp0;
    for (int i = 0; i < 7; i++) {
        temp0 = curl_easy_escape(curl, app_info[i].key.c_str(), app_info[i].key.size());
        encode_info[i] = pair(string(temp0), "");
        curl_free(temp0);
        temp0 = curl_easy_escape(curl, app_info[i].value.c_str(), app_info[i].value.size());
        encode_info[i].value = string(temp0);
        curl_free(temp0);
    }
    std::sort(encode_info, encode_info + 7);
    string out = encode_info[0].to_string();
    for (int i = 1; i < 7; i++) {
        out += "&" + encode_info[i].to_string();
    }
    string url = "https://api.twitter.com/1.1/users/lookup.json";
    temp0 = curl_easy_escape(curl, url.c_str(), url.size());
    char *temp1 = curl_easy_escape(curl, out.c_str(), out.size());
    out = "GET&" + string(temp0) + "&" + string(temp1);
    curl_free(temp0);
    curl_free(temp1);

    temp0 = curl_easy_escape(curl, secrets[0].c_str(), secrets[0].size());
    temp1 = curl_easy_escape(curl, secrets[1].c_str(), secrets[1].size());
    string sign_key = string(temp0) + "&" + string(temp1);
    curl_free(temp0);
    curl_free(temp1);

    unsigned char *sha1hash = hmac_sha1((unsigned char *)sign_key.c_str(),
                                        (unsigned char *)out.c_str(), sign_key.size(), out.size());
    /*unsigned char test[20] = {0xB6, 0x79, 0xC0, 0xAF, 0x18, 0xF4, 0xE9, 0xC5, 0x87, 0xAB,
                                    0x8E, 0x20, 0x0A, 0xCD, 0x4E, 0x48, 0xA9, 0x3F, 0x8C, 0xB6};*/
    app_info[7].value = base64(sha1hash, 20);  // test); //sha1hash);
    temp0 = curl_easy_escape(curl, app_info[7].value.c_str(), app_info[7].value.size());
    app_info[7].value = string(temp0);

    curl_free(temp0);
    string command =
        "curl --get \'" + url + "\' --data \'screen_name=" + app_info[0].value +
        "\' --header \'Authorization: OAuth oauth_consumer_key=\"" + app_info[1].value +
        "\", oauth_nonce=\"" + app_info[2].value + "\", oauth_signature=\"" + app_info[7].value +
        "\", oauth_signature_method=\"" + app_info[3].value + "\", oauth_timestamp=\"" +
        app_info[4].value + "\", oauth_token=\"" + app_info[5].value + "\", oauth_version=\"" +
        app_info[6].value + "\"\' --verbose >" + outfile;

    cout << command << endl;
    std::system(command.c_str());
    curl_easy_cleanup(curl);

    std::ifstream json(outfile);
    if (std::getline(infile, line)) {
        return 0;
    }
    return 1;
}

int main() {
    string username;
    cout << "Enter Twitter Username: ";
    getline(std::cin, username);
    string outfile = "lookup.json";
    if (get_json(username, outfile)) {
        std::system(("./format.sh " + outfile).c_str());
        std::system(("cat " + outfile).c_str());
        printf("\n");
    }
}
