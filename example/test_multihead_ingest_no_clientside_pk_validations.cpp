#include "gpudb/GPUdb.hpp"
#include "gpudb/GPUdbIngestor.hpp"
#include "gpudb/RecordRetriever.hpp"

#include <exception>


void test_worker_list( std::string host )
{
    std::cout << "Test WorkerList:" << std::endl;
    std::cout << "================" << std::endl;
    gpudb::GPUdb gpudb(host, gpudb::GPUdb::Options().setThreadCount(4));

    // Test WorkerList
    // ===============
    // Case: Without a regex
    std::cout << "Case: Without a regex" << std::endl;
    gpudb::WorkerList workers( gpudb );
    std::cout << "# workers: " << workers.size() << std::endl;
    std::cout << workers.toString() << std::endl;
    std::cout << std::endl;

    // Case: Invalid regex
    std::cout << "Case: Invalid regex" << std::endl;
    try
    {
        std::string ip_regex_str(".*30\\!"); // Invalid regex
        gpudb::WorkerList workers( gpudb, ip_regex_str );
        std::cout << "Failure: Should not have gotten anything due to invalid regex";
        std::cout << "# workers: " << workers.size() << std::endl;
        std::cout << workers.toString() << std::endl;
        std::cout << std::endl;
    } catch (gpudb::GPUdbException& e)
    {
        std::cout << "Success: GPUdbException caught: " << e.what() << std::endl;
        std::cout << std::endl;
    }


    // Case: Matching regex
    std::cout << "Case: Matching regex" << std::endl;
    try
    {
        std::string ip_regex_str(".*32\\.21.*");  // Valid regex; should match
        gpudb::WorkerList workers( gpudb, ip_regex_str );
        if (workers.size() > 0)
        {
            std::cout << "Success: Got matching addresses: ";
            std::cout << "# workers: " << workers.size() << std::endl;
            std::cout << workers.toString() << std::endl;
        }
        else
            std::cout << "Failure: Should get matching addresses: but did not get any!";
            
        std::cout << std::endl;
    } catch (gpudb::GPUdbException& e)
    {
        std::cout << "Failure: GPUdbException caught: " << e.what() << std::endl;
        std::cout << std::endl;
    }

    // Case: Non-matching regex
    std::cout << "Case: Non-matching regex" << std::endl;
    try
    {
        std::string ip_regex_str(".*30\\.37.*");  // Valid regex, but no matching
        gpudb::WorkerList workers( gpudb, ip_regex_str );
        if (workers.size() > 0)
        {
            std::cout << "Failure: Should not get any matching address: but got: ";
            std::cout << "# workers: " << workers.size() << std::endl;
            std::cout << workers.toString() << std::endl;
        }
        else
            std::cout << "Success: Should not get any matching address; and did not get any! ";
        std::cout << std::endl;
    } catch (gpudb::GPUdbException& e)
    {
        std::cout << "Success: Should not get any matching address; and did not get any! " << std::endl;
        std::cout << std::endl;
    }
} // end test_worker_list



void test_with_pk_and_non_nullable_column( const gpudb::GPUdb &db )
{
    std::cout << "Test multi-head ingestion with a primary key column" << std::endl;
    std::cout << "===================================================" << std::endl;

    try
    {
        // Create a type
        std::vector<gpudb::Type::Column> columns;
        columns.push_back( gpudb::Type::Column("c2",    gpudb::Type::Column::INT, gpudb::ColumnProperty::PRIMARY_KEY, gpudb::ColumnProperty::SHARD_KEY ) );
        columns.push_back( gpudb::Type::Column("c1", gpudb::Type::Column::INT, gpudb::ColumnProperty::NULLABLE ) );

        gpudb::Type _type = gpudb::Type( "Test3", columns );
        std::string type_id = _type.create( db );

        std::string table_name = "gpudb_ingestor_test_for_no_pk_checks_on_client";

        // Clear any existing table first
        std::cout << "Clearing the table..." << std::endl;
        std::map<std::string, std::string> clear_table_options;
        clear_table_options[ gpudb::clear_table_no_error_if_not_exists ] = gpudb::clear_table_true;
        db.clearTable( table_name, "", clear_table_options );

        // Now create the table
        std::cout << "Creating the table..." << std::endl;
        std::map<std::string, std::string> options;
        db.createTable( table_name, type_id, options );

        // Batch size; using this odd number so that there are some queues
        // remaining at the very end (which will have to be manually flushed)
        size_t batch_size = 33;

        // Test Constructors
        // =================
        // Will NOT update any existing primary key
        gpudb::GPUdbIngestor ingestor1( db, _type, table_name, batch_size );

        // // Insert into the same table, but update on existing primary keys
        // std::map<std::string, std::string> insert_options;
        // insert_options[ gpudb::insert_records_update_on_existing_pk ] = gpudb::insert_records_true;
        // gpudb::GPUdbIngestor ingestor2( db, _type, table_name, insert_options, batch_size );

        // Test insertion
        // ==============
        // Create records and insert them

        std::vector<gpudb::GenericRecord> records;

        gpudb::GenericRecord record1(_type);
        record1.setAsInt( "c2", 1);
        record1.setAsInt( "c1", 2);
        records.push_back( record1 );

        gpudb::GenericRecord record2(_type);
        record2.setAsInt( "c2", 1);
        record2.setAsInt( "c1", 3);
        records.push_back( record2 );
        
        gpudb::GenericRecord record3(_type);
        record3.setAsInt( "c2", 2);
        record3.setAsNullableInt( "c1", boost::optional<int32_t>(NULL));
        records.push_back( record3 );

        gpudb::GenericRecord record4(_type);
        record4.setAsInt( "c2", 2);
        record4.setAsInt( "c1", 5);
        records.push_back( record4 );

        ingestor1.insert( records );

    // Print out the table size
        std::map<std::string, std::string> show_table_options;
        show_table_options[ gpudb::show_table_get_sizes ] = gpudb::show_table_true;
        size_t table_size = db.showTable( table_name, show_table_options ).totalSize;
        std::cout << "Table size (some are probably still in the queue): " << table_size << std::endl;

        // Flush the remaining records
        std::printf("\ntrying to flush the remaining records...\n"); // debug~~~~~~
        ingestor1.flush();
        std::printf("done flushing...\n\n"); // debug~~~~~~

        // Print out the table size
        table_size = db.showTable( table_name, show_table_options ).totalSize;
        std::cout << "Final table size (after manual flushing): " << table_size << std::endl;


    } catch (gpudb::GPUdbInsertionException& e)
    {
        std::cout << "GPUdbInsertionException caught: " << e.what() << std::endl;
    } catch (gpudb::GPUdbException& e)
    {
        std::cout << "GPUdbException caught: " << e.what() << std::endl;
    } catch (std::exception& e)
    {
        std::cout << "Exception caught: " << e.what() << std::endl;
    }

    std::cout << "Done testing; qutting test_sharding_pk..." << std::endl; // debug~~~~~
}  // end test_sharding_pk




int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: gpudb-api-example http://gpudb_host:9191\n";
        exit(1);
    }

    std::string host(argv[1]);
    std::cout << "Connecting to GPUdb host: '" << host << "'\n";

    std::srand( std::time( NULL ) ); // seed the RNG

    try
    {
        gpudb::GPUdb db(host, gpudb::GPUdb::Options().setThreadCount(4));

        // Test the WorkerList class
        test_worker_list( host );
        
        std::cout << "Test GPUdbIngestor:" << std::endl;
        std::cout << "===================" << std::endl;
        // Create a worker list
        gpudb::WorkerList workers( db );
        std::cout << "# workers: " << workers.size() << std::endl;
        std::cout << workers.toString() << std::endl;
        std::cout << std::endl;

        test_with_pk_and_non_nullable_column( db );

    } catch (gpudb::GPUdbException& e)
    {
        std::cout << "GPUdbException caught: " << e.what() << std::endl;
    } catch (std::exception& e)
    {
        std::cout << "Exception caught: " << e.what() << std::endl;
    }

    // std::map<std::string, std::string> options;

    // gpudb::GPUdbIngestor<int> gpudb_ingestor();

    std::cout << "Done testing; quitting main..." << std::endl; // debug~~~~~
}  // end main







