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

static const string KEYS_SEPARATOR = "~~~~~~";

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
    // results.push_back("/wiki/Evolucion_biologica.html");
    // results.push_back("/wiki/elPepe.html");

    for (int i = 0; i < searchString.length(); i++)
    {
        if (searchString[i] == ' ')
            i++;

        else
        {
            int j = i + 1;

            while (searchString[j] != ' ' && j < searchString.length())
                j++;

            string word = searchString.substr(i, j);

            for (auto &c : word)
                c = tolower(c);

            // Para cada path que coincida con la palabra a buscar...
            for (auto path : searchIndex[word])
            {
                if (find(results.begin(), results.end(), path) == results.end()) // check de que no se haya pasado ese webPath ya
                    results.push_back(path);                                     // se agrega a results
            }

            i = j;
        }
    }

    // for (auto word : searchWords)
    // {
    //     map<string, bool> *restorer = &(searchIndex[word]);

    //     for (auto it = restorer->begin(); it != restorer->end(); it++)
    //         it->second = false;
    // }
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
        file << KEYS_SEPARATOR << endl;

        for (auto word : searchIndex)
        {
            file << word.first << endl;

            // for (auto it = word.second.begin(); it != word.second.end(); it++)
            //     file << '\t' << it->data() << endl;

            for (auto it = word.second.begin(); it != word.second.end(); it++)
                file << it->data() << endl;
            
            file << KEYS_SEPARATOR << endl;
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

        string tempStr, word;
        bool nextIsWord = false, nextIsPath = false;

        while(getline(file, tempStr))
        {
            if(!tempStr.compare(KEYS_SEPARATOR))
            {
                nextIsWord = true;
                nextIsPath = false;
            }
            else if(nextIsWord)
            {
                word = tempStr;
                nextIsWord = false;
                nextIsPath = true;
            }
            else if (nextIsPath)
            {
                searchIndex[word].insert(tempStr);
            }
        }

        cout << "Index leído..." << endl;
        return true;
    }

    cout << "No existe índice..." << endl;
    return false;
}