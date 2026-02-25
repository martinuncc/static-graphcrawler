#include <iostream>
#include <string>
#include <queue>
#include <unordered_set>
#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <stdexcept>
#include <thread>
#include <mutex>
#include "rapidjson/error/error.h"

struct ParseException : std::runtime_error, rapidjson::ParseResult {
    ParseException(rapidjson::ParseErrorCode code, const char* msg, size_t offset) : 
        std::runtime_error(msg), 
        rapidjson::ParseResult(code, offset) {}
};

#define RAPIDJSON_PARSE_ERROR_NORETURN(code, offset) \
    throw ParseException(code, #code, offset)

#include "rapidjson/reader.h"
#include <rapidjson/document.h>
#include <chrono>

bool debug = false;

// Updated service URL
const std::string SERVICE_URL = "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/";

// Function to HTTP ecnode parts of URLs. for instance, replace spaces with '%20' for URLs
std::string url_encode(CURL *curl, std::string input)
{
    char *out = curl_easy_escape(curl, input.c_str(), input.size());
    std::string s = out;
    curl_free(out);
    return s;
}

// Callback function for writing response data
size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *output)
{
    size_t totalSize = size * nmemb;
    output->append((char *)contents, totalSize);
    return totalSize;
}

// Function to fetch neighbors using libcurl with debugging
std::string fetch_neighbors(CURL *curl, const std::string &node)
{

    std::string url = SERVICE_URL + url_encode(curl, node);
    std::string response;

    if (debug)
        std::cout << "Sending request to: " << url << std::endl;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // Verbose Logging

    // Set a User-Agent header to avoid potential blocking by the server
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: C++-Client/1.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    }
    else
    {
        if (debug)
            std::cout << "CURL request successful!" << std::endl;
    }

    // Cleanup
    curl_slist_free_all(headers);

    if (debug)
        std::cout << "Response received: " << response << std::endl; // Debug log

    return (res == CURLE_OK) ? response : "{}";
}

// Function to parse JSON and extract neighbors
std::vector<std::string> get_neighbors(const std::string &json_str)
{
    std::vector<std::string> neighbors;
    try
    {
        rapidjson::Document doc;
        doc.Parse(json_str.c_str());

        if (doc.HasMember("neighbors") && doc["neighbors"].IsArray())
        {
            for (const auto &neighbor : doc["neighbors"].GetArray())
                neighbors.push_back(neighbor.GetString());
        }
    }
    catch (const ParseException &e)
    {
        std::cerr << "Error while parsing JSON: " << json_str << std::endl;
        throw e;
    }
    return neighbors;
}

// BFS Traversal Function
std::vector<std::vector<std::string>> bfs(const std::string &start, int depth)
{
    std::vector<std::vector<std::string>> levels;
    std::unordered_set<std::string> visited;

    levels.push_back({start});
    visited.insert(start);
    std::mutex mtx;

    for (int d = 0; d < depth; d++)
    {
        if (debug)
            std::cout << "starting level: " << d << "\n";
        levels.push_back({});
        // Create threads here and join before depth change
        int max_threads = std::thread::hardware_concurrency();
        int num_nodes = levels[d].size();
        int num_threads = std::min(max_threads, num_nodes);
        int nodes_per_thread = num_nodes / num_threads;

        std::vector<std::thread> threads;
        for (int t = 0; t < num_threads; t++)
        {
            int start = t * nodes_per_thread;
            int end = (t == num_threads - 1) ? num_nodes : start + nodes_per_thread;

            threads.push_back(std::thread([&, start, end]()
                                          {
        CURL *tcurl = curl_easy_init();
        if (!tcurl)
        {
          std::cerr << "Failed to initialize CURL\n";
          return;
        }
        for (int i = start; i < end; i++)
        {
          std::string s = levels[d][i];
          try
          {
            for (const auto &neighbor : get_neighbors(fetch_neighbors(tcurl, s)))
            {
              std::lock_guard<std::mutex> lock(mtx);
              if (!visited.count(neighbor))
              {
                visited.insert(neighbor);
                levels[d + 1].push_back(neighbor);
              }
            }
          }
          catch (const ParseException &e)
          {
            std::cerr << "Error while fetching neighbors of: " << s << std::endl;
            throw e;
          }
        }
        curl_easy_cleanup(tcurl); }));
        }
        for (std::thread &thread : threads)
        {
            thread.join();
        }
    }
    return levels;
}

int main(int argc, char *argv[])
{
    curl_global_init(CURL_GLOBAL_ALL);
    int numthreads = std::thread::hardware_concurrency();
    std::cout << "Number of threads available: " << numthreads << "\n";
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <node_name> <depth>\n";
        return 1;
    }

    std::string start_node = argv[1]; // example "Tom%20Hanks"
    int depth;
    try
    {
        depth = std::stoi(argv[2]);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: Depth must be an integer.\n";
        return 1;
    }

    const auto start{std::chrono::steady_clock::now()};

    for (const auto &n : bfs(start_node, depth))
    {
        for (const auto &node : n)
            std::cout << "- " << node << "\n";
        std::cout << n.size() << "\n";
    }

    const auto finish{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{finish - start};
    std::cout << "Time to crawl: " << elapsed_seconds.count() << "s\n";

    curl_global_cleanup();
    return 0;
}
