#include "nitro_generator.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <random>
#include <atomic>
#include <condition_variable>
#include "curl.h"
#include <windows.h>
#include "json.hpp"
#include <chrono>
#include <queue>
#include <mutex>

using json = nlohmann::json;
using namespace std;

namespace NitroGenerator {
    constexpr int DISCORD_API_LIMIT = 50; 
    constexpr int MAX_THREADS = 8;        
    std::mutex queue_mutex;
    std::condition_variable cv;
    std::queue<std::string> code_queue;
    std::atomic<bool> stop_threads(false);

    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        return size * nmemb; 
    }

    void initialize() {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    void cleanup() {
        curl_global_cleanup();
    }

    std::string generateNitroCode() {
        static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::string code;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);

        for (int i = 0; i < 16; ++i) {
            code += alphanum[dis(gen)];
        }
        return code;
    }

    void saveToFile(const std::string& save_file, const std::string& nitro_url) {
        static std::mutex file_mutex;
        std::lock_guard<std::mutex> lock(file_mutex);
        std::ofstream file(save_file, std::ios::app);
        if (file.is_open()) {
            file << nitro_url << std::endl;
        }
        else {
            std::cerr << "Error opening file: " << save_file << std::endl;
        }
    }

    void checkNitroCode(const std::string& save_file) {
        while (!stop_threads) {
            std::string code_nitro;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                cv.wait(lock, [] { return !code_queue.empty() || stop_threads; });
                if (stop_threads && code_queue.empty()) break;

                code_nitro = code_queue.front();
                code_queue.pop();
            }

            std::string nitro_url = "https://discord.gift/" + code_nitro;

            CURL* curl = curl_easy_init();
            if (curl) {
                std::string api_url = "https://discordapp.com/api/v6/entitlements/gift-codes/" + code_nitro +
                    "?with_application=false&with_subscription_plan=true";
                curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);

                CURLcode res = curl_easy_perform(curl);
                if (res == CURLE_OK) {
                    long response_code;
                    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

                    if (response_code == 200) {
                        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
                        std::cout << "[Valid] Nitro: " << nitro_url << std::endl;
                        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                        std::cout << "This will take a long, long time so make sure you are quick to redeem it." << std::endl;
                        std::cout << "To stop it, press Ctrl + C." << std::endl;

                        if (!save_file.empty()) saveToFile(save_file, nitro_url);
                    }
                    else if (response_code == 429) { 
                        double retry_after = 1.0;
                        json response_data;
                        std::cerr << "[Rate Limited] Pausing for " << retry_after << " seconds." << std::endl;
                        std::cout << "This will take a long, long time so make sure you are quick to redeem it." << std::endl;
                        std::cout << "To stop it, press Ctrl + C." << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(retry_after * 1000)));
                    }
                    else {
                        std::cout << "[Invalid] Nitro: " << nitro_url << std::endl;
                    }
                }
                else {
                    std::cerr << "Request error: " << curl_easy_strerror(res) << std::endl;
                }

                curl_easy_cleanup(curl);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / DISCORD_API_LIMIT));
        }
    }

    void runChecks(int thread_count, const std::string& save_file) {
        std::vector<std::thread> threads;

        for (int i = 0; i < thread_count * 10; ++i) {
            code_queue.push(generateNitroCode());
        }

        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back(checkNitroCode, save_file);
        }

        std::thread generator([thread_count]() {
            while (!stop_threads) {
                {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    for (int i = 0; i < thread_count * 5; ++i) {
                        code_queue.push(generateNitroCode());
                    }
                }
                cv.notify_all();
                std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
            }
            });

        for (auto& t : threads) {
            t.join();
        }

        stop_threads = true;
        cv.notify_all();
        generator.join();
    }
}
