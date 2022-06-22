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

#include "EDAoogleHttpRequestHandler.h"

using namespace std;

static void removeHtmlFromLine(string &line);
static void splitLineInStrings(string &linel, vector<string> &output);
static bool isNotSeparator(char c);
static void decodeHtmlEntities(vector<string> &words);
static void printSearchIndex();

EDAoogleHttpRequestHandler::EDAoogleHttpRequestHandler(string homePath) : ServeHttpRequestHandler(homePath)
{
    if (!loadSearchIndex())
    {
        buildSearchIndex();
        printSearchIndex();
    }
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
    if (searchString.size())
    {
        vector<string> wordsToSearch;
        splitLineInStrings(searchString, wordsToSearch);

        vector<unordered_set<string>> partialResults;

        for (auto &word : wordsToSearch)
        {
            for (auto &c : word)
                c = tolower(c);

            // Se separan todas las coincidencias para cada palabra
            partialResults.push_back(searchIndex[word]);
        }

        // Solo se aceptan solo los paths comunes a todas las coincidencias
        for (auto path : partialResults[0])
        {
            bool pathChecker = false;

            for (int i = 1; i < partialResults.size(); i++)
            {
                if (!partialResults[i].count(path))
                {
                    pathChecker = true;
                    break;
                }
            }

            if (!pathChecker)
                results.push_back(path);
        }
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

                while (getline(fileStream, tempStr))
                {
                    removeHtmlFromLine(tempStr);

                    vector<string> words;
                    splitLineInStrings(tempStr, words);

                    decodeHtmlEntities(words);

                    for (auto word : words)
                    {
                        for (auto &c : word)
                            c = tolower(c);

                        searchIndex[word].insert(webPath);
                    }
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

void EDAoogleHttpRequestHandler::printSearchIndex()
{
    ofstream file("searchIndex.txt");

    if (file.is_open())
    {
        for (auto word : searchIndex)
        {
            file << word.first << " ";

            for (auto it = word.second.begin(); it != word.second.end(); it++)
                file << it->data() << " ";

            file << endl;
        }
    }

    file.close();
}

bool EDAoogleHttpRequestHandler::loadSearchIndex()
{
    ifstream file("searchIndex.txt");

    if (file.is_open())
    {
        cout << "Leyendo índice..." << endl;

        string tempStr;

        while (getline(file, tempStr))
        {
            size_t startIndex = 0;
            size_t endIndex = tempStr.find_first_of(' ');

            string word = tempStr.substr(startIndex, endIndex - startIndex);

            startIndex = endIndex + 1;

            while ((endIndex = tempStr.find(' ', startIndex)) != string::npos)
            {
                searchIndex[word].insert(tempStr.substr(startIndex, endIndex - startIndex));
                startIndex = endIndex + 1;
            }
        }

        cout << "Índice leído..." << endl;
        return true;
    }

    cout << "No existe índice. Creándolo..." << endl;
    return false;
}