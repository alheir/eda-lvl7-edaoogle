/**
 * @file EDAoogleHttpRequestHandler.h
 * @author Marc S. Ressl
 * @brief EDAoggle search engine
 * @version 0.1
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef EDAOOGLEHTTPREQUESTHANDLER_H
#define EDAOOGLEHTTPREQUESTHANDLER_H

#include "ServeHttpRequestHandler.h"
#include <unordered_map>
#include <unordered_set>

class EDAoogleHttpRequestHandler : public ServeHttpRequestHandler
{
public:
    EDAoogleHttpRequestHandler(std::string homePath);

    bool handleRequest(std::string url, HttpArguments arguments, std::vector<char> &response);

private:
    void matchSearch(std::string &searchString, std::vector<std::string> &results);
    void buildSearchIndex();
    void printSearchIndex();

    // std::map<std::string, std::map<std::string, bool>> searchIndex;
    std::unordered_map<std::string, std::unordered_set<std::string>> searchIndex;
};

#endif
