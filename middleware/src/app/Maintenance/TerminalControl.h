#ifndef TerminalControl_h
#define TerminalControl_h


#ifdef __cplusplus
extern "C" {
#endif

// Terminal: SSH
int getSSHState();
int setSSHState(int);
void controlSSHDebug(void);
void saveSSHInfo(const char* user, const char* pswd);
void resetSSHInfo(void);
int checkSSHInfo(const char* user, const char* pswd);
void getSSHInfo(char* user, int userlen, char* pswd, int pswdlen);

// Terminal: SerialProt
int getSerialPortPrintState();
int setSerialPortPrintState(int flag);


#ifdef __cplusplus
} // extern "C"
#endif 

#endif // TerminalControl_h

