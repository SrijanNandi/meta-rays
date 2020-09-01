#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include <string>

std::string read_ap_mac();
std::string ExecUciCommand(const std::string &cmd);
std::string ExecSystemCommand(const std::string &cmd);
