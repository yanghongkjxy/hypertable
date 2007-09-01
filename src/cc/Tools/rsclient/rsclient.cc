/**
 * Copyright (C) 2007 Doug Judd (Zvents, Inc.)
 * 
 * This file is part of Hypertable.
 * 
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 * 
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

extern "C" {
#include <readline/readline.h>
#include <readline/history.h>
}

#include "Common/InteractiveCommand.h"
#include "Common/Properties.h"
#include "Common/System.h"
#include "Common/Usage.h"

#include "AsyncComm/Comm.h"
#include "AsyncComm/ConnectionManager.h"

#include "Hypertable/Lib/Manager.h"
#include "Hypertable/Lib/RangeServerClient.h"

#include "CommandLoadRange.h"

using namespace hypertable;
using namespace std;

namespace {

  char *line_read = 0;

  char *rl_gets () {

    if (line_read) {
      free (line_read);
      line_read = (char *)NULL;
    }

    /* Get a line from the user. */
    line_read = readline ("rsclient> ");

    /* If the line has any text in it, save it on the history. */
    if (line_read && *line_read)
      add_history (line_read);

    return line_read;
  }

  /**
   * Parses the --server= command line argument.
   */
  void ParseServerArgument(const char *arg, std::string &host, uint16_t *portPtr) {
    const char *ptr = strchr(arg, ':');
    *portPtr = 0;
    if (ptr) {
      host = string(arg, ptr-arg);
      *portPtr = (uint16_t)atoi(ptr+1);
    }
    else
      host = arg;
    return;
  }


  /**
   *
   */
  void BuildInetAddress(struct sockaddr_in &addr, PropertiesPtr &propsPtr, std::string &userSuppliedHost, uint16_t userSuppliedPort) {
    std::string host;
    uint16_t port;

    if (userSuppliedHost == "")
      host = "localhost";
    else
      host = userSuppliedHost;

    if (userSuppliedPort == 0)
      port = (uint16_t)propsPtr->getPropertyInt("Hypertable.RangeServer.port", 38549);
    else
      port = userSuppliedPort;

    memset(&addr, 0, sizeof(struct sockaddr_in));
    {
      struct hostent *he = gethostbyname(host.c_str());
      if (he == 0) {
	herror("gethostbyname()");
	exit(1);
      }
      memcpy(&addr.sin_addr.s_addr, he->h_addr_list[0], sizeof(uint32_t));
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
  }


  const char *usage[] = {
    "usage: hypertable [OPTIONS]",
    "",
    "OPTIONS:",
    "",
    "  --config=<file>   Read configuration from <file>.  The default config",
    "     file is \"conf/hypertable.cfg\" relative to the toplevel install",
    "     directory",
    "",
    "  --server=<host>[:<port>]  Connect to Range server at machine <host>",
    "     and port <port>.  By default it connects to localhost:38549.",
    "",
    "  --help  Display this help text and exit.",
    "",
    "This program provides a command line interface to a Range Server.",
    (const char *)0
  };
  
}


/**
 *
 */
int main(int argc, char **argv) {
  const char *line;
  string configFile = "";
  string host = "";
  uint16_t port = 0;
  struct sockaddr_in addr;
  vector<InteractiveCommand *>  commands;
  Comm *comm;
  ConnectionManager *connManager;
  PropertiesPtr propsPtr;
  RangeServerClient *rsClient;
  MasterClient *masterClient;

  System::Initialize(argv[0]);
  ReactorFactory::Initialize((uint16_t)System::GetProcessorCount());

  for (int i=1; i<argc; i++) {
    if (!strncmp(argv[i], "--config=", 9))
      configFile = &argv[i][9];
    if (!strncmp(argv[i], "--server=", 9))
      ParseServerArgument(&argv[i][9], host, &port);
    else
      Usage::DumpAndExit(usage);
  }

  if (configFile == "")
    configFile = System::installDir + "/conf/hypertable.cfg";

  propsPtr.reset( new Properties(configFile) );

  BuildInetAddress(addr, propsPtr, host, port);  

  comm = new Comm();
  connManager = new ConnectionManager(comm);

  // Create Range Server client object
  rsClient = new RangeServerClient(connManager);

  // Connect to Range Server
  connManager->Add(addr, 30, "Range Server");
  if (!connManager->WaitForConnection(addr, 15))
    cerr << "Timed out waiting for for connection to Range Server.  Exiting ..." << endl;

  // Connect to Master
  masterClient = new MasterClient(connManager, propsPtr);
  if (!masterClient->WaitForConnection(15))
    cerr << "Timed out waiting for for connection to Master.  Exiting ..." << endl;
  
  commands.push_back( new CommandLoadRange(masterClient, rsClient) );

  /**
  commands.push_back( new CommandGetSchema(manager) );
  **/

  cout << "Welcome to the Range Server command interpreter." << endl;
  cout << "Type 'help' for a description of commands." << endl;
  cout << endl << flush;

  using_history();
  while ((line = rl_gets()) != 0) {
    size_t i;

    if (*line == 0)
      continue;

    for (i=0; i<commands.size(); i++) {
      if (commands[i]->Matches(line)) {
	commands[i]->ParseCommandLine(line);
	commands[i]->run();
	break;
      }
    }

    if (i == commands.size()) {
      if (!strcmp(line, "quit") || !strcmp(line, "exit"))
	exit(0);
      else if (!strcmp(line, "help")) {
	cout << endl;
	for (i=0; i<commands.size(); i++) {
	  Usage::Dump(commands[i]->Usage());
	  cout << endl;
	}
      }
      else
	cout << "Unrecognized command." << endl;
    }

  }

  return 0;
}
