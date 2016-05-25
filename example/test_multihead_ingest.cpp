#include "gpudb/GPUdb.hpp"
#include "gpudb/GPUdbIngestor.hpp"


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: gpudb-api-example http://gpudb_host:9191\n";
        exit(1);
    }

    std::string host(argv[1]);
    std::cout << "Connecting to GPUdb host: '" << host << "'\n";

    gpudb::GPUdb gpudb(host, gpudb::GPUdb::Options().setThreadCount(4));
    std::map<std::string, std::string> options;

    gpudb::GPUdbIngestor<int> gpudb_ingestor();

}
