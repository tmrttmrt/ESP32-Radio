void fileLogBegin();
void fileLogSend(const char *s);
void printFileToClient(String filename, WiFiClient *const cli);
void printAllFilesToClient(WiFiClient *const cli);