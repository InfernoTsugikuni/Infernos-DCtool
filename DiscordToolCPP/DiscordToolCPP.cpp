#define NOMINMAX 
#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <limits>
#include "ConsoleUtils.h"
#include "Functions.h"
#include "Webhook.hpp"
#include <cctype>
#include <thread>
#include <exception>
#include "nitro_generator.hpp"

void option() {
    while (true) { 
        banner();
        int choice;

        std::cout << "Pick an option" << std::endl;
        std::cout << "1. Webhook Message" << std::endl;
        std::cout << "2. Webhook Spammer" << std::endl;
        std::cout << "3. Webhook checker" << std::endl;
        std::cout << "4. Discord nitro generator (will take a long time like about a month at least nonstop)" << std::endl;
        std::cout << "5. Exit" << std::endl;
        std::cout << "6. Github" << std::endl;
        std::cout << "Enter your choice (1-6): ";

        std::cin >> choice;

        if (std::cin.fail() || choice < 1 || choice > 6) {
            std::cin.clear(); // Clear the error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
            system("cls"); 
            std::cout << "Invalid input. Please enter a number between 1 and 6." << std::endl;
            std::cout << "\n";
            continue; 
        }

        switch (choice) {
        case 1:
            system("cls");
            InputWebhookUrl();
            Payload();
            SendWebhook();
            break;

        case 2:
            system("cls");
            InputWebhookUrl();
            Payload();
            SpamWebhook();
            break;

        case 3:
            system("cls");
            InputWebhookUrl();
            CheckWebhook();  
            break;

        case 4: {
            system("cls");

            std::string save_file = "valid_nitro_codes.txt";

            unsigned int max_threads = std::thread::hardware_concurrency();
            if (max_threads == 0) {
                max_threads = 4;
            }
            unsigned int thread_count = max_threads / 2; 
            std::cout << "Detected " << max_threads << " hardware threads. Using " << thread_count << " threads for generation." << std::endl;

            NitroGenerator::initialize(); // Initialize libcurl

            std::cout << "Starting Discord Nitro code generation with " << thread_count << " threads..." << std::endl;

            try {
                NitroGenerator::runChecks(thread_count, save_file);
            }
            catch (const std::exception& e) {
                std::cerr << "An error occurred: " << e.what() << std::endl;
            }

            NitroGenerator::cleanup(); 
            break;
        }


        case 5: {
            HANDLE hProcess = GetCurrentProcess(); 
            TerminateProcess(hProcess, 0);        
            break; 
        }

        case 6:
            system("cls");
            std::cout << "Opening GitHub...\n";
            system("start https://github.com/InfernoTsugikuni");
            break;
        }

    }
}

int main(){
    SetConsoleTitle(L"Inferno's DCtool");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    system("start https://infernotsugikuni.work/");

    option();
    std::cin.get();
    return 0;
}
