#include "gpudb/GPUdb.hpp"

#include <boost/lexical_cast.hpp>

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: gpudb-api-example http://172.30.70.6:9191" << std::endl;
		exit(1);
	}

	std::string host(argv[1]);
	std::cout << "Connecting to GPUdb host: '" << host << "'\n";

    #ifndef GPUDB_NO_HTTPS
    // Use SSL context that does no certificate validation (for example purposes only)
    boost::asio::ssl::context sslContext(boost::asio::ssl::context::sslv23);
    gpudb::GPUdb::Options opts = gpudb::GPUdb::Options().setSslContext(&sslContext);
    #else
    gpudb::GPUdb::Options opts;
    #endif

	gpudb::GPUdb gpudb(host, opts);
	std::map<std::string, std::string> options;

	// Get the version information
	std::cout << "GPUdb C++ Client Version: " << gpudb.getApiVersion() << std::endl;

	// Create some test data

	std::vector<gpudb::Type::Column> columns;
	std::vector<gpudb::GenericRecord> data;

	columns.push_back(gpudb::Type::Column("A", gpudb::Type::Column::INT, "primary_key"));
	gpudb::Type aType = gpudb::Type("JoinTestA", columns);
	gpudb.createTable("JoinTestA", aType.create(gpudb), options);

	for (int ii = 0; ii < 10; ii++) {
		gpudb::GenericRecord aRecord(aType);
		aRecord.intValue("A") = ii;
		data.push_back(aRecord);
	}

    gpudb.insertRecords("JoinTestA", data, options);

    columns.clear();
    columns.push_back(gpudb::Type::Column("B", gpudb::Type::Column::INT, "primary_key"));
    gpudb::Type bType = gpudb::Type("JoinTestB", columns);
    gpudb.createTable("JoinTestB", bType.create(gpudb), options);

	data.clear();

	for (int ii = 0; ii < 20; ii++) {
		gpudb::GenericRecord bRecord(bType);
		bRecord.intValue("B") = ii;
		data.push_back(bRecord);
	}
    gpudb.insertRecords("JoinTestB", data, options);

    std::vector<std::string> joinTables;
    joinTables.push_back("JoinTestA as A");
    joinTables.push_back("JoinTestB as B");
    std::vector<std::string> columnNames;
    columnNames.push_back("A.A as A");
    columnNames.push_back("B.B as B");
    std::vector<std::string> expressions;
    expressions.push_back("A = B");
    gpudb.createJoinTable("JoinTestC", joinTables, columnNames, expressions, options);

    columnNames.clear();
    columnNames.push_back("A");
    columnNames.push_back("B");
    gpudb::GetRecordsByColumnResponse grbcResponse = gpudb.getRecordsByColumn("JoinTestC", columnNames, 0, 10, options);

    for (size_t i = 0; i < grbcResponse.data.size(); ++i)
    {
        gpudb::GenericRecord& record = grbcResponse.data[i];
        std::cout << record.toString("A")
                << " " << record.toString("B")
                << std::endl;
    }

    gpudb.clearTable("JoinTestA", "", options);
    gpudb.clearTable("JoinTestB", "", options);

    return 0;
}
