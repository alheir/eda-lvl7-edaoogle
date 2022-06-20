/**
 * @file EDAoogleHttpRequestHandler.h
 * @author Marc S. Ressl
 * @brief EDAoggle search engine
 * @version 0.1
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>
#include <regex>

#include "EDAoogleHttpRequestHandler.h"

using namespace std;

static void removeHtmlFromLine(string &line);
static void splitLineInStrings(string &linel, vector<string> &output);
static bool isNotSeparator(char c);
static void decodeHtmlEntities(vector<string> &words);

EDAoogleHttpRequestHandler::EDAoogleHttpRequestHandler(string homePath) : ServeHttpRequestHandler(homePath)
{
    buildSearchIndex();
}

bool EDAoogleHttpRequestHandler::handleRequest(string url,
                                               HttpArguments arguments,
                                               vector<char> &response)
{
    string searchPage = "/search";
    if (url.substr(0, searchPage.size()) == searchPage)
    {
        string searchString;
        if (arguments.find("q") != arguments.end())
            searchString = arguments["q"];

        // Header
        string responseString = string("<!DOCTYPE html>\
<html>\
\
<head>\
    <meta charset=\"utf-8\" />\
    <title>EDAoogle</title>\
    <link rel=\"preload\" href=\"https://fonts.googleapis.com\" />\
    <link rel=\"preload\" href=\"https://fonts.gstatic.com\" crossorigin />\
    <link href=\"https://fonts.googleapis.com/css2?family=Inter:wght@400;800&display=swap\" rel=\"stylesheet\" />\
    <link rel=\"preload\" href=\"../css/style.css\" />\
    <link rel=\"stylesheet\" href=\"../css/style.css\" />\
</head>\
\
<body>\
    <article class=\"edaoogle\">\
        <div class=\"title\"><a href=\"/\">EDAoogle</a></div>\
        <div class=\"search\">\
            <form action=\"/search\" method=\"get\">\
                <input type=\"text\" name=\"q\" value=\"" +
                                       searchString + "\" autofocus>\
            </form>\
        </div>\
        ");

        float searchTime = 0.1F;
        vector<string> results;
        matchSearch(searchString, results);

        // Print search results
        responseString += "<div class=\"results\">" + to_string(results.size()) +
                          " results (" + to_string(searchTime) + " seconds):</div>";
        for (auto &result : results)
            responseString += "<div class=\"result\"><a href=\"" +
                              result + "\">" + result + "</a></div>";

        // Trailer
        responseString += "    </article>\
</body>\
</html>";

        response.assign(responseString.begin(), responseString.end());

        return true;
    }
    else
        return serve(url, response);

    return false;
}

void EDAoogleHttpRequestHandler::matchSearch(string &searchString, vector<string> &results)
{
    // results.push_back("/wiki/Evolucion_biologica.html");
    // results.push_back("/wiki/elPepe.html");

    vector<string> searchWords;

    for (int i = 0; i < searchString.length(); i++)
    {
        if (searchString[i] == ' ')
            i++;

        else
        {
            int j = i + 1;

            while (searchString[j] != ' ' && j < searchString.length())
                j++;

            searchWords.push_back(searchString.substr(i, j - i));

            map<string, bool> *matches = &(searchIndex[searchWords.back()]);

            for (auto it = matches->begin(); it != matches->end(); it++)
            {
                if (!it->second)
                {
                    results.push_back(it->first);
                    it->second = true;
                }
            }

            i = j;
        }
    }

    for (auto word : searchWords)
    {
        map<string, bool> *restorer = &(searchIndex[word]);

        for (auto it = restorer->begin(); it != restorer->end(); it++)
            it->second = false;
    }
}

void EDAoogleHttpRequestHandler::buildSearchIndex()
{
    const auto path = filesystem::absolute("www/wiki");

    // Paths de todos los .html
    vector fileList(filesystem::directory_iterator(path), {});

    for (auto &file : fileList)
    {
        if (filesystem::is_regular_file(file.path()))
        {
            ifstream fileStream(file.path().string());

            if (fileStream)
            {
                string webPath = file.path().string();
                webPath = webPath.substr(webPath.find("/wiki"));

                string tempStr;

                // while (getline(fileStream, tempStr))
                // {
                //     if (tempStr.substr(0, 3) == "<p>")
                //         break;
                // }

                while (getline(fileStream, tempStr))
                {
                    removeHtmlFromLine(tempStr);
                    // RemoveHTMLTags(tempStr);

                    vector<string> words;
                    splitLineInStrings(tempStr, words);

                    decodeHtmlEntities(words);

                    for (auto word : words)
                        searchIndex[word][webPath] = false;
                    ;
                }
            }
        }
    }
}

static void removeHtmlFromLine(string &line)
{
    while ((line.find('<')) != string::npos)
    {
        int starter = line.find('<');
        int ender = line.find('>') + 1;

        if (ender != string::npos)
        {
            line.erase(starter, ender - starter);
        }
    }
}

static void splitLineInStrings(string &line, vector<string> &output)
{

    output.clear();
    for (int i = 0; i < line.length(); i++)
    {
        if (isNotSeparator(line[i]))
        {
            int j = i + 1;

            while (isNotSeparator(line[j]) && j < line.length())
                j++;

            output.push_back(line.substr(i, j - i));

            i = j;
        }
    }
}

static bool isNotSeparator(char c)
{
    return !(c == ' ' || c == ',' || c == '.' || c == '\r' || c == '-' || c == '\t');
}

static void decodeHtmlEntities(vector<string> &words)
{
    for (auto &word : words)
    {
        int startIndex = word.find("&#");

        if (startIndex != string::npos)
        {
            int endIndex = startIndex + 4;

            if (word[endIndex] != ';')
                endIndex++;

            int code = stoi(word.substr(startIndex + 2, endIndex - 1));

            string realChar;

            switch (code)
            {
            case 225:
                realChar = "á";
                break;
            case 233:
                realChar = "é";
                break;
            case 237:
                realChar = "í";
                break;
            case 243:
                realChar = "ó";
                break;
            case 250:
                realChar = "ú";
                break;
            case 63:
                realChar = "?";
                break;
            case 191:
                realChar = "¿";
                break;
            case 33:
                realChar = "!";
                break;
            case 161:
                realChar = "¡";
                break;
            default:
                break;
            }

            word.replace(startIndex, (code <= 99) ? 5 : 6, realChar);
        }
    }
}