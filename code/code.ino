#include <aplayer.h> // Code from https://github.com/prittt/ArduinoBuzzerPlayer

#define DEBUG true

#include <Arduino.h>
#include <aplayer.h>

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <ESP_Mail_Client.h>

#define WIFI_SSID "HUAWEI P20"
#define WIFI_PASSWORD "123456789"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* The sign in credentials */
#define AUTHOR_EMAIL "pozzetto.firenze@gmail.com"
#define AUTHOR_PASSWORD "ibqjxrdfflnfwngy"

/* Recipient's emails and names*/
String emails[] = {"federico.bolelli@unimore.it", "lk_number1@hotmail.it"};
size_t emails_size = sizeof(emails)/sizeof(String);
String names[]  = {"Federico", "Federico"};

#define FLOATINGLOW_PIN   12 // Pin connected to the lower floating
#define FLOATINGHIGHT_PIN 13 // Pin connected to the higher floating
#define BUZZER_PIN        14 // Pin which control the buzzer
APlayer myPlayer(BUZZER_PIN); // Create a player object specifying the buzzer's pin

#define GREENLED_PIN   16 // Pin to control green LED
#define REDLED_PIN      5 // Pin to control red LED
#define YELLOWLED_PIN   4 // Pin to control yellow LED

/* The SMTP Session object used for Email sending */
SMTPSession smtp;

// Butto press function with "debouncing"
# define BUTTONS_PRESS_TIME_SHORT 1000
# define BUTTONS_PRESS_TIME_LONG 3000
bool IsPressed(int button_pin, unsigned long press_duration = BUTTONS_PRESS_TIME_SHORT, bool direct = 1){
    
    if (!direct){
        if(digitalRead(button_pin) == LOW){
          unsigned long start_press = millis();
          while(digitalRead(button_pin) == LOW && (millis() - start_press < press_duration)){wdt_reset();} // The reset on the ESP8266 must be fast (watchdog max-time is quite small). The call to wdt_reset() is to avoid the random crash of the module!
  
          if(digitalRead(button_pin) == LOW && (millis() - start_press >= press_duration)){
            return true;  
          }
        }
    }else{
        if(digitalRead(button_pin) == HIGH){
          unsigned long start_press = millis();
          while(digitalRead(button_pin) == HIGH && (millis() - start_press < press_duration)){wdt_reset();} // The reset on the ESP8266 must be fast (watchdog max-time is quite small). The call to wdt_reset() is to avoid the random crash of the module!
  
          if(digitalRead(button_pin) == HIGH && (millis() - start_press >= press_duration)){
            return true;  
          }
        }
    }
    
    return false;
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

bool ConnectToWifiAndSendEmail(String floating_status, String email_content){
  if (DEBUG){
    Serial.println();
    Serial.print("Connecting to AP");
  }
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    if (DEBUG){
      Serial.print(".");
    }
    delay(200);
  }

  if (DEBUG){
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();
  }
  
  /** Enable the debug via Serial port
   * none debug or 0
   * basic debug or 1
  */
  smtp.debug(DEBUG);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the session config data */
  ESP_Mail_Session session;

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = "Pozzetto Via Firenze";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = floating_status;

  for (size_t i = 0; i < emails_size; ++i) {
    message.addRecipient(names[i], emails[i]);
  }

  /*Send HTML message*/
  String htmlMsg = "<h1>" + email_content + "</h1><p>Sent from ESP board</p>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  /*
  //Send raw text message
  String textMsg = "Hello World! - Sent from ESP board";
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;*/

  /* Set the custom message header */
  //message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

  /* Connect to server with the session config */
  if (!smtp.connect(&session)){
    return false;
  }

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message)){
    return false;
    if (DEBUG){ 
      Serial.println("Error sending Email, " + smtp.errorReason());
    }
  }
  return true;
}

void setup(){
  pinMode(FLOATINGLOW_PIN, INPUT);

  pinMode(GREENLED_PIN, OUTPUT);
  pinMode(REDLED_PIN, OUTPUT);
  pinMode(YELLOWLED_PIN, OUTPUT);

  digitalWrite(GREENLED_PIN, HIGH);
  digitalWrite(REDLED_PIN, LOW);
  digitalWrite(YELLOWLED_PIN, LOW);

  if (DEBUG){
    Serial.begin(115200);
  }
}

enum system_status {
  FINE = 0, 
  WARNING = 1,
  WARNING_RESTORING = 2,
  CRITICAL = 3
};

int current_status = FINE;
int last_email_succesfully_sent = true;

void loop(){

  if (current_status != FINE) {
      myPlayer.play(ALARM);
  }

  bool hight_floating_pressed = IsPressed(FLOATINGHIGHT_PIN);
  bool   low_floating_pressed = IsPressed(FLOATINGLOW_PIN);
  
  if (!last_email_succesfully_sent) {
      switch(current_status) {
        case FINE: ConnectToWifiAndSendEmail("OK", "Tutto è tornato alla normalità."); break;
        case WARNING_RESTORING: ConnectToWifiAndSendEmail("RESTORING?", "Il galleggiante alto è disattivo, ma quello basso è ancora sommerso!"); break;
        case WARNING: ConnectToWifiAndSendEmail("WARNING", "Il galleggiante basso è scattato!"); break;
        case CRITICAL: ConnectToWifiAndSendEmail("CRITICAL", "Il galleggiante alto è scattato!"); break;
      }
  }


  // Check the higher floating 
  if (hight_floating_pressed && current_status != CRITICAL){   
    current_status = CRITICAL; // Update the current status

    /* Change LED status: red and yellow ON; green OFF */
    digitalWrite(YELLOWLED_PIN, HIGH);
    digitalWrite(REDLED_PIN   , HIGH);
    digitalWrite(GREENLED_PIN , LOW );   

    last_email_succesfully_sent = ConnectToWifiAndSendEmail("CRITICAL", "Il galleggiante alto è scattato!");
  } 
  
  if (low_floating_pressed && !hight_floating_pressed && current_status != WARNING && current_status != WARNING_RESTORING){   
    /* Change LED status: red ON; yellow and green OFF */
    digitalWrite(YELLOWLED_PIN, HIGH);
    digitalWrite(GREENLED_PIN , LOW);   
    digitalWrite(REDLED_PIN   , LOW);

    if (current_status == FINE){
      if (DEBUG){
        Serial.println(".");
      }
      last_email_succesfully_sent = ConnectToWifiAndSendEmail("WARNING", "Il galleggiante basso è scattato!");
      current_status = WARNING; // Update the current status
    } else {
      /* Current status is CRTICAL*/
      last_email_succesfully_sent = ConnectToWifiAndSendEmail("RESTORING?", "Il galleggiante alto è disattivo, ma quello basso è ancora sommerso!");
      current_status = WARNING_RESTORING; // Update the current status
    }
     
  }
  
  if (current_status != FINE && !low_floating_pressed && !hight_floating_pressed /* This second check should be useless, but I'll leave to "cope" with a broken lower floating */){  
    current_status = FINE; // Update the current status

    /* Change LED status: green ON; yellow and red OFF */
    digitalWrite(YELLOWLED_PIN, LOW);
    digitalWrite(REDLED_PIN   , LOW);   
    digitalWrite(GREENLED_PIN , HIGH);   

    last_email_succesfully_sent = ConnectToWifiAndSendEmail("OK", "Tutto è tornato alla normalità.");
  } 

  delay(1000);
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}
