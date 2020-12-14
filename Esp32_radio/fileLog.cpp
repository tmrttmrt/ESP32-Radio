#include <string.h>
#include <FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <WiFi.h>
#include <SpiffsFilePrint.h>
#include "fileLog.h"

#define NLOGFILES 2
#define LOGFILESIZE 20000
#define BLEN 255
#define LOGQUEUELEN 50

char*       dbgprint( const char* format, ... ) ;
char*       dbgprintS( const char* format, ... ) ;

static TaskHandle_t      fileLogTaskH;                            
static QueueHandle_t     fileLogQueueH;
static SpiffsFilePrint *filePrint = NULL;
SemaphoreHandle_t filePrintSem = NULL; 

const char baseName[] = "/logfile"; 
const char extName[] = ".log"; 
const char midName[] = ".current"; 

void printFileToClient(String filename, WiFiClient *const cli) 
{
  uint8_t *buff[BLEN];
  size_t blen=BLEN;
  size_t br=0;
  
  dbgprint("Sending %s",filename.c_str());
  cli->println("");
  cli->println(filename);
  xSemaphoreTake ( filePrintSem, portMAX_DELAY );
  File file = SPIFFS.open(filename, FILE_READ);
  while (0<(br=file.read((uint8_t *)buff,blen))) {
    if(br>0){
        if(0 == cli->write((uint8_t *) buff,br)){
          file.close();
          xSemaphoreGive(filePrintSem);
          return;
      }
    }
    vTaskDelay(10/portTICK_PERIOD_MS);
  }
  cli->println();
  file.close();
  SPIFFS.remove(filename);
  xSemaphoreGive(filePrintSem);
}

void printAllFilesToClient(WiFiClient *const cli)
{
  int i;
  
  for(i=0; i< NLOGFILES; i++){
    char buff[sizeof(baseName)+sizeof(midName)+sizeof(extName)+4];
    char *mp;
    
    strcpy(buff,baseName);
    itoa(i,buff+sizeof(baseName)-1,10);
    mp=buff+strlen(buff);
    strcpy(mp,midName);
    strcat(buff,extName);
    printFileToClient(buff,cli);
    strcpy(mp,extName);
    printFileToClient(buff,cli);
  }
}


void fileLogSend(const char *s){
  char *dup = strdup(s);
  xQueueSend(fileLogQueueH, ( void * ) &dup, portMAX_DELAY);
}

void fileLogtask(void * parameter)
{
  char *s;
  while (true){
    while(LOGQUEUELEN/2 > uxQueueMessagesWaiting(fileLogQueueH))
    {  
      vTaskDelay(50/portTICK_PERIOD_MS);
    }
    xSemaphoreTake ( filePrintSem, portMAX_DELAY );
    if(filePrint) filePrint->open();
    while(xQueueReceive(fileLogQueueH, &s, 10)){
        if(filePrint) filePrint->println(s);                   // Write several messages to a log file
        free(s);
    }
    if(filePrint) filePrint->close();
    xSemaphoreGive(filePrintSem);
  }
}

void fileLogBegin()
{
  
  if (!SPIFFS.begin()) {
      Serial.println("SPIFFS Mount Failed, we format it");
      SPIFFS.format();
  }
    
  filePrint = new SpiffsFilePrint("/logfile", NLOGFILES, LOGFILESIZE);
  
  fileLogQueueH = xQueueCreate (LOGQUEUELEN, sizeof(char *));
  filePrintSem = xSemaphoreCreateMutex();
  
  xTaskCreate (
    fileLogtask,                                              // Task to handle special functions.
    "File log task",                                            // name of task.
    0x1000,                                                 // Stack size of task
    NULL,                                                 // parameter of the task
    2,                                                    // priority of the task
    &fileLogTaskH ) ;                                         // Task handle to keep track of created task
}


