#include <stdio.h>
#include "korelin.h"



// 这个 main 函数只用于测试
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("* Welcome to Korelin\n"
       "* (c) 2026 Nexogic. Licensed under the MIT Open Source License.\n"
       "* Korelin SDK Version: %s\n"
       "\nUsage:\n\n"
       "  kric <command> [arguments]\n"
       "\nCommands:\n"
       "  build <file_name>    Compile your code to Korelin bytecode.\n"
       "  run <file_name>      Execute your .kri/.kric/.kar code.\n"
       "  init <project_name>  Initialize a new Korelin project.\n"
       "  version              Show the Korelin SDK version.\n"
       "  path                 Show the Korelin installation path.\n", KORELIN_VERSION);
    }
}