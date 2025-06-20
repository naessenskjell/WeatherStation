#include <webota.h>
#include <Arduino.h>

WebServer server(80);

extern const char* OTAUser;
extern const char* OTAPassword;

static String style =
  "<style>"
  "body {"
  "  background: #3498db;"
  "  font-family: sans-serif;"
  "  font-size: 14px;"
  "  color: #777;"
  "  display: flex;"
  "  justify-content: center;"
  "  align-items: center;"
  "  height: 100vh;"
  "  margin: 0;"
  "}"
  "form {"
  "  background: #fff;"
  "  max-width: 258px;"
  "  padding: 30px;"
  "  border-radius: 5px;"
  "  text-align: center;"
  "  box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);"
  "}"
  "input[type='text'] {"
  "  width: 100%;"
  "  height: 44px;"
  "  border: 1px solid #dddddd;"
  "  border-radius: 22px;"
  "  margin: 10px 0;"
  "  padding: 0 15px;"
  "  font-size: 15px;"
  "  background: #f1f1f1;"
  "  box-sizing: border-box;"
  "}"
  ".btn {"
  "  width: 100%;"
  "  height: 44px;"
  "  border: 0;"
  "  border-radius: 22px;"
  "  margin: 10px 0;"
  "  background: #3498db;"
  "  color: #fff;"
  "  font-size: 15px;"
  "  cursor: pointer;"
  "}"
  "</style>";

static String loginIndex = 
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "<title>ESP32 Login</title>"
  "</head>"
  "<body>"
  "<form name='loginForm' action='/login' method='post'>"
  "<h1>ESP32 Login</h1>"
  "<input name='userid' placeholder='User ID'>"
  "<input name='pwd' placeholder='Password' type='password'>"
  "<input type='submit' class='btn' value='Login'>"
  "</form>"
  + style +
  "</body>"
  "</html>";

static String serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
"<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
"<label id='file-input' for='file'>   Choose file...</label>"
"<input type='submit' class=btn value='Update'>"
"<br><br>"
"<div id='prg'></div>"
"<br><div id='prgbar'><div id='bar'></div></div><br></form>"
"<script>"
"function sub(obj){"
"var fileName = obj.value.split('\\\\');"
"document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
"};"
"$('form').submit(function(e){"
"e.preventDefault();"
"var form = $('#upload_form')[0];"
"var data = new FormData(form);"
"$.ajax({"
"url: '/update',"
"type: 'POST',"
"data: data,"
"contentType: false,"
"processData:false,"
"xhr: function() {"
"var xhr = new window.XMLHttpRequest();"
"xhr.upload.addEventListener('progress', function(evt) {"
"if (evt.lengthComputable) {"
"var per = evt.loaded / evt.total;"
"$('#prg').html('progress: ' + Math.round(per*100) + '%');"
"$('#bar').css('width',Math.round(per*100) + '%');"
"}"
"}, false);"
"return xhr;"
"},"
"success:function(d, s) {"
"console.log('success!') "
"},"
"error: function (a, b, c) {"
"}"
"});"
"});"
"</script>" + style;

void setupWebOTA(const char* host, std::function<void(const char*)> logFunc) {
    if (!MDNS.begin(host)) {
        Serial.println("Error setting up MDNS responder!");
        if (logFunc) logFunc("mDNS responder setup failed");
        while (1) delay(1000);
    }
    if (logFunc) logFunc("mDNS responder started");

    // Serve login page
    server.on("/", HTTP_GET, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", loginIndex);
    });
    // Handle login
    server.on("/login", HTTP_POST, []() {
        String user = server.arg("userid");
        String pwd = server.arg("pwd");
        if (user == OTAUser && pwd == OTAPassword) {
            server.sendHeader("Location", "/serverIndex");
            server.send(302, "text/plain", "");
        } else {
            server.send(401, "text/html", "<h1>Unauthorized</h1><p>Invalid credentials</p>");
        }
    });
    server.on("/serverIndex", HTTP_GET, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", serverIndex);
    });
    server.on("/update", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
    }, []() {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("Update: %s\n", upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) {
                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
        }
    });
    server.begin();
    if (logFunc) logFunc("Web OTA server started");
}

void handleWebServer() {
    server.handleClient();
}