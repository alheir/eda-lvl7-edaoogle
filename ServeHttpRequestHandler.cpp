/**
 * @file ServeHttpRequestHandler.h
 * @author Marc S. Ressl
 * @brief Simple HTTP request handler that serves files
 * @version 0.1
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <filesystem>
#include <fstream>
#include <iostream>

#include "ServeHttpRequestHandler.h"

using namespace std;

ServeHttpRequestHandler::ServeHttpRequestHandler(string homePath)
{
    this->homePath = homePath;
}

bool ServeHttpRequestHandler::handleRequest(string url, HttpArguments arguments, vector<char> &response)
{
    return serve(url, response);
}

/**
 * @brief Serves a static webpage
 *
 * @param url The URL
 * @param response The HTTP response
 * @return true URL valid
 * @return false URL invalid
 */
bool ServeHttpRequestHandler::serve(string url, vector<char> &response)
{
    // Blocks directory traversal
    // e.g. https://www.example.com/show_file.php?file=../../MyFile
    // * Builds absolute local path from url
    // * Checks if absolute local path is within home path
    auto homeAbsolutePath = filesystem::absolute(homePath);
    auto relativePath = homeAbsolutePath / url.substr(1);
    string path = filesystem::absolute(relativePath.make_preferred()).string();

    if (path.substr(0, homeAbsolutePath.string().size()) != homeAbsolutePath)
        return false;

    // Serves file
    ifstream file(path);
    if (file.fail())
        return false;

    file.seekg(0, ios::end);
    int fileSize = file.tellg();
    file.seekg(0, ios::beg);

    response.resize(fileSize);
    file.read(response.data(), fileSize);

    return true;
}
