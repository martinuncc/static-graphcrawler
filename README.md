# static-graphcrawler
Author: Martin Anuonye

This repository contains two programs which both use BFS to traverse movie and actor wikidata. parlevelclient.cpp utilizes threads to do this. Both programs take command line arguments, first should be the starting node as one string, and the second argument should be the depth. The program will output a log file that contains all nodes visited and the time it took. Users should download curl and rapidjson, and a makefile is also included which should be used to point to wherever rapidjson is located for the user.
