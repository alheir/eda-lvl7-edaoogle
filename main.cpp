/**
 * @file main.cpp
 * @author Marc S. Ressl
 * @brief
 * @version 0.1
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <iostream>

#include <microhttpd.h>

#include "CommandLineParser.h"
#include "HttpServer.h"
#include "EDAoogleHttpRequestHandler.h"

using namespace std;

int main(int argc, const char *argv[])
{
    CommandLineParser parser(argc, argv);

    // Configuration
    int port = 8000;
    string homePath = "www";

    // Parse command line
    if (parser.hasOption("--help"))
    {
        cout << "edahttpd 0.1" << endl
             << endl;
        cout << "Usage: edahttpd [-p PORT] [-h HOME_PATH]" << endl;

        return 0;
    }

    if (parser.hasOption("-p"))
        port = stoi(parser.getOption("-p"));

    if (parser.hasOption("-h"))
        homePath = parser.getOption("-h");

    // Start server
    HttpServer server(port);

    EDAoogleHttpRequestHandler edaOogleHttpRequestHandler(homePath);
    server.setHttpRequestHandler(&edaOogleHttpRequestHandler);

    if (server.isRunning())
    {
        cout << "Running server..." << endl;

        // Wait for keyboard entry
        char value;
        cin >> value;

        cout << "Stopping server..." << endl;
    }
}
