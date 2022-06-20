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

class EDAoogleHttpRequestHandler : public ServeHttpRequestHandler
{
public:
    EDAoogleHttpRequestHandler(std::string homePath);

    bool handleRequest(std::string url, HttpArguments arguments, std::vector<char> &response);

private:
    void matchSearch(std::string &searchString, std::vector<std::string> &results);
    void buildSearchIndex();

    std::map<std::string, std::map<std::string, uint32_t>> searchIndex;
};

#endif
