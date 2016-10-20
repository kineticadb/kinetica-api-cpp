#include "gpudb/GPUdb.hpp"

#include <boost/lexical_cast.hpp>

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: gpudb-api-example http://172.30.70.6:9191\n";
		exit(1);
	}

	std::string host(argv[1]);
	std::cout << "Connecting to GPUdb host: '" << host << "'\n";

	gpudb::GPUdb gpudb(host, gpudb::GPUdb::Options().setThreadCount(4));
	std::map<std::string, std::string> options;

	// Get the version information
	std::cout << "GPUdb C++ Client Version: " << gpudb.getApiVersion() << std::endl;

	// Create some test data

	std::vector<gpudb::Type::Column> columns;
	std::vector<std::string> pk;
	pk.push_back("primary_key");
	std::vector<gpudb::GenericRecord> data;

	columns.push_back(gpudb::Type::Column("A", gpudb::Type::Column::INT, pk));
	gpudb::Type aType = gpudb::Type("JoinTestA", columns);
	gpudb.createTable("JoinTestA", aType.create(gpudb), options);

	for (int ii = 0; ii < 10; ii++) {
		gpudb::GenericRecord aRecord(aType);
		aRecord.asInt("A") = ii;
		data.push_back(aRecord);
	}

    gpudb.insertRecords("JoinTestA", data, options);

    columns.clear();
    columns.push_back(gpudb::Type::Column("B", gpudb::Type::Column::INT, pk));
    gpudb::Type bType = gpudb::Type("JoinTestB", columns);
    gpudb.createTable("JoinTestB", bType.create(gpudb), options);

	data.clear();

	for (int ii = 0; ii < 20; ii++) {
		gpudb::GenericRecord bRecord(bType);
		bRecord.asInt("B") = ii;
		data.push_back(bRecord);
	}
    gpudb.insertRecords("JoinTestB", data, options);

    std::vector<std::string> joinTables;
    joinTables.push_back("JoinTestA");
    joinTables.push_back("JoinTestB");
    std::vector<std::string> aliases;
    aliases.push_back("A");
    aliases.push_back("B");
    std::vector<std::string> expressions;
    gpudb.createJoinTable("JoinTestC", joinTables, aliases, "A.A = B.B", expressions, options);

    std::vector<std::string> columnNames;
    columnNames.push_back("A.A");
    columnNames.push_back("B.B");
    gpudb::GetRecordsByColumnResponse grbcResponse = gpudb.getRecordsByColumn("JoinTestC", columnNames, 0, 10, options);

    for (size_t i = 0; i < grbcResponse.data.size(); ++i)
    {
        gpudb::GenericRecord& record = grbcResponse.data[i];
        std::cout << record.asInt("A.A")
                << " " << record.asInt("B.B")
                << std::endl;
    }

    gpudb.clearTable("JoinTestA", "", options);
    gpudb.clearTable("JoinTestB", "", options);

    return 0;
}