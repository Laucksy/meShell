#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void handleSIGINT(int sig);
void handleSIGTSTP(int sig);
void handleSIGCONT(int sig);
void handleSIGCHLD(int sig);
