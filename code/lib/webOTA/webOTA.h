#pragma once
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <functional>

extern WebServer server;

void setupWebOTA(const char* host, std::function<void(const char*)> logFunc);
void handleWebServer();