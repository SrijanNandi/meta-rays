#define _GLIBCXX_USE_C99 1
// using namespace Tins;
#include "mqtt_client.h"
#include <unistd.h>
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
  cout << "Sniffing application started " << endl;
  cout << "Waiting for message of broker... " << '\n' << endl;

  mqtt_setup_publisher();

  // get_interface_on_bootup();

  // while (1) {
  //   sleep(600);
  // }
}
