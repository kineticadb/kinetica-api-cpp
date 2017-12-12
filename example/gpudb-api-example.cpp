#include "gpudb/GPUdb.hpp"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional_io.hpp>

void run(gpudb::GPUdb &gpudb)
{
    std::map<std::string, std::string> options;

    // Get the version information
    std::cout << "GPUdb C++ Client Version: " << gpudb.getApiVersion() << std::endl;

    // Create some test data

    std::vector<gpudb::Type::Column> columns;
    columns.push_back(gpudb::Type::Column("TRACKID", gpudb::Type::Column::STRING));
    columns.push_back(gpudb::Type::Column("TIMESTAMP", gpudb::Type::Column::LONG));
    columns.push_back(gpudb::Type::Column("x", gpudb::Type::Column::DOUBLE));
    columns.push_back(gpudb::Type::Column("y", gpudb::Type::Column::DOUBLE));
    gpudb::Type newType = gpudb::Type("Test", columns);
    std::string typeId = newType.create(gpudb);
    std::string table_name = "Test";

    try
    {
        gpudb.createTable(table_name, typeId, options);

        std::vector<gpudb::GenericRecord> data;

        long t = 0;

        for (double x = -180.0; x <= 180.0; x += 1.0)
        {
            for (double y = -90.0; y <= 90.0; y += 1.0)
            {
                gpudb::GenericRecord record(newType);
                record.stringValue("TRACKID") = boost::lexical_cast<std::string>(x);
                record.longValue("TIMESTAMP") = t++;
                record.doubleValue("x") = x;
                record.doubleValue("y") = y;
                data.push_back(record);
            }
        }

        gpudb.insertRecords(table_name, data, options);

        std::cout << "done inserting records" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << " caught exception: " << e.what() << " continuing" << std::endl;
    }

    // Group by value

    std::vector<std::string> gbvColumns;
    gbvColumns.push_back("x");
    std::map<std::string, std::string> gbvParams;
    gbvParams["limit"] = "10";


    std::cout << "calling groupby (new)" << std::endl;

    std::vector<std::string> gbColumns;
    gbColumns.push_back("x");
    gbColumns.push_back("count(*)");
    gbColumns.push_back("sum(y)");
    gbColumns.push_back("min(y)");
    gbColumns.push_back("max(y)");
    std::map<std::string, std::string> gbParams;
//    gbParams[ gpudb::AggregateGroupByRequest::OPTIONS.SORT_ORDER ] = gpudb::AggregateGroupByRequest::OPTIONS.ASCENDING;
//    // gbParams[ gpudb::AggregateGroupByRequest::Options::SORT_ORDER ] = gpudb::AggregateGroupByRequest::Options::ASCENDING;

    gpudb::AggregateGroupByResponse gbResponse = gpudb.aggregateGroupBy(table_name, gbColumns, 0, 20, gbParams);

    std::cout << "got groupby response, data size: " << gbResponse.data.size() << std::endl;

    for (size_t i = 0; i < gbResponse.data.size(); ++i)
    {
        gpudb::GenericRecord& record = gbResponse.data[i];
        const gpudb::Type& type = record.getType();
        if (i == 0)
        {
            std::cout << "Number of fields : " << type.getColumnCount() << std::endl;
            for (size_t j = 0; j < type.getColumnCount(); ++j)
            {
                std::cout << type.getColumn(j).getName() << "\t";
            }
            std::cout << std::endl;
        }

        for (size_t j=0;j<type.getColumnCount();++j)
        {
            std::cout << record.toString(j) << "\t";
        }
        std::cout << std::endl;
    }

    std::cout << " done groupby" << std::endl;

    // Filter / GetRecordsFromCollection

    std::string table_view_name = "TestResult";
    gpudb.filter(table_name, table_view_name, "x > 89", options);

    gpudb::GetRecordsResponse<gpudb::GenericRecord> gsoResponse = gpudb.getRecords<gpudb::GenericRecord>(newType, table_view_name, 0, 10, options);

    for (size_t i = 0; i < gsoResponse.data.size(); ++i)
    {
        const gpudb::GenericRecord& record = gsoResponse.data[i];
        std::cout << record.toString("TRACKID")
                << " " << record.toString("TIMESTAMP")
                << " " << record.toString("x")
                << " " << record.toString("y")
                << std::endl;
    }

    // Alternate GetSetObjects (if you don't already have the schema)

    gpudb::GetRecordsResponse<boost::any> gsoResponse2 = gpudb.getRecords<boost::any>(table_view_name, 0, 10, options);

    for (size_t i = 0; i < gsoResponse2.data.size(); ++i)
    {
        gpudb::GenericRecord record = boost::any_cast<gpudb::GenericRecord>(gsoResponse2.data[i]);
        std::cout << record.toString("TRACKID")
                << " " << record.toString("TIMESTAMP")
                << " " << record.toString("x")
                << " " << record.toString("y")
                << std::endl;
    }

    // GetRecordsByColumn

    std::cout << "doing getrecordsbycolumn" << std::endl;
    std::vector<std::string> columnNames;
    columnNames.push_back("TRACKID");
    columnNames.push_back("TIMESTAMP");
    columnNames.push_back("x");
    columnNames.push_back("y");
    gpudb::GetRecordsByColumnResponse grbcResponse = gpudb.getRecordsByColumn(table_view_name, columnNames, 0, 10, options);

    for (size_t i = 0; i < grbcResponse.data.size(); ++i)
    {
        gpudb::GenericRecord& record = grbcResponse.data[i];
        std::cout << record.toString("TRACKID")
                << " " << record.toString("TIMESTAMP")
                << " " << record.toString("x")
                << " " << record.toString("y")
                << std::endl;
    }

    std::cout << "Done getrecordsbycolumn" << std::endl;

    // Clean up

    gpudb.clearTable(table_name, "", options);

}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: gpudb-api-example http://gpudb_host:9191" << std::endl;
        std::cout << "\tgpudb-api-example http://gpudb_host:9191,http://gpudb_host:9192" << std::endl;

		int i;
		std::cout << "Press any key...";
		std::cin >> i;

        exit(1);
    }

    std::vector<std::string> hosts;
    boost::split(hosts, argv[1], boost::is_any_of(","));

    #ifndef GPUDB_NO_HTTPS
    // Use SSL context that does no certificate validation (for example purposes only)
    boost::asio::ssl::context sslContext(boost::asio::ssl::context::sslv23);
    gpudb::GPUdb::Options opts = gpudb::GPUdb::Options().setTimeout(1000).setSslContext(&sslContext);
    #else
    gpudb::GPUdb::Options opts = gpudb::GPUdb::Options().setTimeout(1000);
    #endif

    if (hosts.size() == 1)
    {
        std::string host(hosts[0]);
        std::cout << "Connecting to GPUdb host: '" << host << "'" << std::endl;
        gpudb::GPUdb gpudb(host, opts);
        run(gpudb);
    }
    else if (hosts.size() > 1)
    {
        gpudb::GPUdb gpudb(hosts, opts);
        std::cout << "Connecting to GPUdb host: '" << gpudb.getUrl() << "'" << std::endl;
        run(gpudb);
    }
    else
    {
        std::cout << "No host provided" << std::endl;
    }

	int i;
	std::cout << "Press any key...";
	std::cin >> i;

    return 0;
}
