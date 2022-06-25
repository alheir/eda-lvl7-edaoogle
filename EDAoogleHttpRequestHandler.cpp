/**
 * @file EDAoogleHttpRequestHandler.h
 * @authors Marc S. Ressl, Heir Alejandro, Hertter José, Vieira Valentín
 * @brief EDAoggle search engine
 * @version 0.1
 * @copyright Copyright (c) 2022
 *
 */

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

// Referencia para la implementación de medición de tiempos:
// https://stackoverflow.com/questions/22387586/measuring-execution-time-of-a-function-in-c
#include <chrono>

#include "EDAoogleHttpRequestHandler.h"

using namespace std;

static void removeHtmlFromLine(string &line);
static void splitLineInStrings(string &linel, vector<string> &output);
static bool isNotSeparator(char c);
static void decodeHtmlEntities(vector<string> &words);
static void encodeHtmlEntities(vector<string> &words);
static void printSearchIndex();

/**
 * @brief Construct a new EDAoogleHttpRequestHandler::EDAoogleHttpRequestHandler object
 *
 * @param homePath
 */
EDAoogleHttpRequestHandler::EDAoogleHttpRequestHandler(string homePath) : ServeHttpRequestHandler(homePath)
{
    auto t1 = chrono::high_resolution_clock::now();
    if (!loadSearchIndex())
    {
        auto t1 = chrono::high_resolution_clock::now();

        buildSearchIndex();

        auto t2 = chrono::high_resolution_clock::now();
        chrono::duration<double, std::milli> buildSearchIndexTime = t2 - t1;

        printSearchIndex();
        t1 = chrono::high_resolution_clock::now();

        chrono::duration<double, std::milli> printSearchIndexTime = t1 - t2;

        cout << "Tiempo de armado de índice: " << buildSearchIndexTime.count() << "ms" << endl;
        cout << "Tiempo de escritura de índice : " << printSearchIndexTime.count() << "ms" << endl;
    }
    else
    {
        auto t2 = chrono::high_resolution_clock::now();
        chrono::duration<double, std::milli> loadSearchIndexTime = t2 - t1;
        cout << "Tiempo de lectura de índice : " << loadSearchIndexTime.count() << "ms" << endl;
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

        vector<string> results;

        auto t1 = chrono::high_resolution_clock::now();

        matchSearch(searchString, results);

        auto t2 = chrono::high_resolution_clock::now();
        chrono::duration<double, std::milli> matchSearchTime = t2 - t1;
        cout << "Tiempo de búsqueda: " << matchSearchTime.count() << "ms" << endl;

        float searchTime = matchSearchTime.count() / 1000.0F;

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
/**
 * @brief Find the pages that contain all the words given in the string "searchString" and
 *        completes the "results" vector.
 *
 * @param searchString
 * @param results
 */
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
/**
 * @brief makes a search index that contains all the words that appear
 *        on the available pages
 *
 */
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

                    // Intentamos encodear las palabras ingresadas por el usuario, pero se hacía mal
                    // el reemplazo por htmlEntities (agregaba caracteres extra)

                    // Sería más eficiente, ya que no se decodifica cada palabra de la wiki
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
/**
 * @brief removes all HTML content from a given line
 *
 * @param line
 */
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
/**
 * @brief divides a line into separated strings
 *
 * @param line      Line to split into separated strings
 * @param output    vector that contains the separated strings
 */
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
/**
 * @brief Determines if a character is a separator of words
 *
 * @param c character to evaluate
 * @return true if c is not a separator
 * @return false if c is a separator
 */
static bool isNotSeparator(char c)
{
    return !(c == ' ' || c == ',' || c == '.' || c == '\r' || c == '-' || c == '\t' || c == '"' ||
             c == '\'' || c == ';' || c == '(' || c == ')' || c == '[' || c == ']' || c == ':');
}
/**
 * @brief Convert an HTML entity to a UNICODE character
 *
 * @param words     words that may contain HTML entities to convert
 */
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

/**
 * @brief convert an Unicode character to a HTML entity
 *
 * @param words     Words which may contain the characters to be converted
 */
static void encodeHtmlEntities(vector<string> &words)
{
    for (auto &word : words)
    {
        int entitieIndex;

        if ((entitieIndex = word.find("á")) != string::npos)
            word.replace(entitieIndex, 1, "&#225;");
        if ((entitieIndex = word.find("é")) != string::npos)
            word.replace(entitieIndex, 1, "&#233;");
        if ((entitieIndex = word.find("í")) != string::npos)
            word.replace(entitieIndex, 0, "&#237;");
        if ((entitieIndex = word.find("ó")) != string::npos)
            word.replace(entitieIndex, 1, "&#243;");
        if ((entitieIndex = word.find("ú")) != string::npos)
            word.replace(entitieIndex, 1, "&#250;");

        if ((entitieIndex = word.find("?")) != string::npos)
            word.replace(entitieIndex, 1, "&#63;");
        if ((entitieIndex = word.find("¿")) != string::npos)
            word.replace(entitieIndex, 1, "&#191;");

        if ((entitieIndex = word.find("!")) != string::npos)
            word.replace(entitieIndex, 1, "&#33;");
        if ((entitieIndex = word.find("¡")) != string::npos)
            word.replace(entitieIndex, 1, "&#161;");
    }
}

/**
 * @brief Saves in disk the search index
 *
 */

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

/**
 * @brief Checks if the index exists and in that case, it reads it
 *
 * @return true if the search was read
 * @return false if the index does not exist
 */
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