#include "commonlib.h"
#include <iostream>
#include <memory>
#include <fstream>
#include <algorithm>

using namespace std;

string read_ap_mac() {
  string apMacAddress = "";
  string line;
  string interface_name;
  interface_name = ExecUciCommand("uci get ray.ray.wanInterface");  

  string file_name = "/sys/class/net/" + interface_name + "/address";
  cout << "File Name: " << file_name << endl;
  ifstream myfile(file_name.c_str());
  if (myfile.is_open()) {
    while (getline(myfile, line)) {
      // file_data += line;
      apMacAddress = line;
    }
    myfile.close();
  } else {
    cout << "Unable to open file" << endl;
    // return -1;
  }
  // return 0;
  return apMacAddress.empty() ? "12:34:56:78" : apMacAddress;
}

std::string ExecSystemCommand(const std::string &cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"),
                                                pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

std::string ExecUciCommand(const std::string &cmd) {
  string result = ExecSystemCommand(cmd);
  result.erase(
      remove(result.begin(), result.end(), '\n'),
      result.end());
  return result;
}
