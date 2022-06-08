/*
 * CommandLineArguments
 *
 * Quick and dirty command line parser
 *
 * Copyright (C) 2022 Marc S. Ressl
 */

#include "CommandLineParser.h"

using namespace std;

CommandLineParser::CommandLineParser(int argc, const char *argv[])
{
    arguments.assign(argv + 1, argv + argc);
}

bool CommandLineParser::hasOption(const string &name)
{
    for (auto &argument : arguments)
    {
        if (argument == name)
            return true;
    }

    return false;
}

string CommandLineParser::getOption(const string &name)
{
    for (auto i = arguments.begin(); i != arguments.end(); i++)
    {
        if (*i == name)
        {
            if ((i + 1) != arguments.end())
                return *(i + 1);
        }
    }

    return "";
}
