#include <string.h>
#include <FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <SpiffsFilePrint.h>
#include "fileLog.h"

#define NLOGFILES 2
#define LOGFILESIZE 20000

char*       dbgprint( const char* format, ... ) ;

static TaskHandle_t      fileLogTaskH;                            
static QueueHandle_t     fileLogQueueH;
static SpiffsFilePrint *filePrint = NULL;

const char baseName[] = "/logfile"; 
const char extName[] = ".log"; 
const char midName[] = ".current"; 

void printFileToPStream(String filename, Print *const printStr) 
{
  dbgprint("Sending %s",filename.c_str());
  printStr->println("");
  printStr->println(filename);
  File file = SPIFFS.open(filename, FILE_READ);
  while (file.available()) {
    printStr->write(file.read());
  }
  printStr->println();
  file.close();
}

void printAllFilesToPStream(Print *const printStr)
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
    printFileToPStream(buff,printStr);
    strcpy(mp,extName);
    printFileToPStream(buff,printStr);
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
    if(xQueueReceive(fileLogQueueH, &s, portMAX_DELAY)){
     if(filePrint){
        filePrint->open();  
        filePrint->println(s);                   // Write to log file
        filePrint->close();
      }
      free(s);
      s=NULL;
    }
  }
}

void fileLogBegin()
{
  
  if (!SPIFFS.begin()) {
      Serial.println("SPIFFS Mount Failed, we format it");
      SPIFFS.format();
  }
    
  filePrint = new SpiffsFilePrint("/logfile", NLOGFILES, LOGFILESIZE);
  
  fileLogQueueH = xQueueCreate (50, sizeof(char *));
  
  xTaskCreate (
    fileLogtask,                                              // Task to handle special functions.
    "File log task",                                            // name of task.
    0x1000,                                                 // Stack size of task
    NULL,                                                 // parameter of the task
    2,                                                    // priority of the task
    &fileLogTaskH ) ;                                         // Task handle to keep track of created task
}


