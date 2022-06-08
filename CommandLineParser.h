/**
 * @file CommandLineParser.h
 * @author Marc S. Ressl
 * @brief Quick and dirty command line parser
 * @version 0.1
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <string>
#include <vector>

class CommandLineParser
{
public:
    CommandLineParser(int argc, const char *argv[]);

    bool hasOption(const std::string &option);
    std::string getOption(const std::string &option);

private:
    std::vector<std::string> arguments;
};

#endif
