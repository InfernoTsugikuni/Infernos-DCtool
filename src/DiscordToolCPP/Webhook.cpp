#define NOMINMAX
#include "json.hpp"
#include <iostream>
#include <limits>
#include "curl.h"
#include <limits>
#include <thread>
#include <chrono>
#include "ConsoleUtils.h"

using json = nlohmann::json;

std::string WebhookUrl;
std::string username;
std::string avatarUrl;
std::string content;

void InputWebhookUrl() {
    std::cout << "Please input your webhook URL: ";
    std::cin >> WebhookUrl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void Payload() {
    std::cout << "Enter the username: ";
    std::getline(std::cin, username);

    std::cout << "Enter the avatar URL: ";
    std::getline(std::cin, avatarUrl);

    std::cout << "Enter the content of the message: ";
    std::getline(std::cin, content);

    json payload = {
        {"username", username},
        {"avatar_url", avatarUrl},
        {"content", content}
    };

    std::cout << "Prepared JSON payload:\n" << payload.dump(4) << std::endl;
}

void SendWebhook() {
    json payload = {
        {"username", username},
        {"avatar_url", avatarUrl},
        {"content", content}
    };

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL." << std::endl;
        return;
    }

    std::string payloadStr = payload.dump();

    curl_easy_setopt(curl, CURLOPT_URL, WebhookUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadStr.size());

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    }
    else {
        std::cout << "Webhook sent successfully!" << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

void SpamWebhook() {
    json payload = {
        {"username", username},
        {"avatar_url", avatarUrl},
        {"content", content}
    };

    std::string payloadStr = payload.dump();

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL." << std::endl;
        return;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    while (true) {
        curl_easy_setopt(curl, CURLOPT_URL, WebhookUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadStr.size());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            std::cout << "Webhook sent successfully! To terminate this, please press Ctrl + C." << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    return size * nmemb;
}

void CheckWebhook() {
    CURL* curl = nullptr;
    CURLcode res;
    long response_code;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL." << std::endl;
        curl_global_cleanup();
        return;
    }

    if (WebhookUrl.empty()) {
        std::cerr << "Webhook URL is empty." << std::endl;
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, WebhookUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return;
    }

    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (res != CURLE_OK) {
        std::cerr << "Failed to retrieve HTTP response code." << std::endl;
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return;
    }

    if (response_code == 200) {
        SetConsoleColor(FOREGROUND_GREEN);
        std::cout << "Webhook is valid." << std::endl;
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    }
    else {
        std::cout << "Webhook is invalid. Response code: " << response_code << std::endl;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
}
