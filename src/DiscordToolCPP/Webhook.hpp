#pragma once
#ifndef WEBHOOK_HPP
#define WEBHOOK_HPP

#include <string>

//Get the webhook URL
extern std::string WebhookUrl;

//Webhook parameters
extern std::string username;
extern std::string avatarUrl;
extern std::string content;

//Get the webhook
void InputWebhookUrl();

//Send the webhook payload
void SendWebhook();

//Get the webhook JSON payload 
void Payload();

//Webhook spammer
void SpamWebhook();

//Webhook checker
void CheckWebhook();

#endif // !WEBHOOK_HPP
