# static-graphcrawler
Author: Martin Anuonye

This repository contains two programs which both use BFS to traverse movie and actor wikidata. parlevelclient.cpp utilizes threads to do this. Both programs take command line arguments, first should be the starting node as one string, and the second argument should be the depth. The program will output a log file that contains all nodes visited and the time it took. Users should download curl and rapidjson, and a makefile is also included which should be used to point to wherever rapidjson is located for the user.

Sequentially: Crawled Tom Hanks to depth 1 in 0.160989 seconds.
Sequentially: Crawled Tom Hanks to depth 2 in 3.29435 seconds.
Sequentially: Crawled Tom Hanks to depth 3 in 72.2112 seconds.

In Parallel: Crawled Tom Hanks to depth 1 in 0.161243 seconds.
In Parallel: Crawled Tom Hanks to depth 2 in 0.613942 seconds.
In Parallel: Crawled Tom Hanks to depth 3 in 4.21738 seconds.

