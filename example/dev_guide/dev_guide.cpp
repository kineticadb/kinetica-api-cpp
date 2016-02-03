
#include <iostream>
#include <GPUdb.hpp>


const std::string COL_1 = "col1";
const std::string COL_2 = "col2";
const std::string COL_GROUP_ID = "group_id";
const std::string STR_GROUP_1_CONST = "Group 1";
const std::string STR_GROUP_2_CONST = "Group 2";
const std::string my_table("my_table_1");


// helper functions prototypes
void print_GetRecordsResponse(const gpudb::GetRecordsResponse<gpudb::GenericRecord>&);
void insertRecordsLocal(gpudb::GPUdb& h_db, avro::ValidSchema myTypeSchema, int start, int end, double col1_const, std::string col2_const, std::string group_id_const, bool display_record_ids = false);


int main(int argc, char *argv[])
{
    std::map<std::string,std::string> options;


    gpudb::GPUdb h_db("http://127.0.0.1:9191");

    // drops all existing tables from the system.
    // h_db.clearTable("", "", options);

    std::vector<gpudb::Type::Column> columns;
    columns.push_back(gpudb::Type::Column(COL_1, avro::AVRO_DOUBLE));
    columns.push_back(gpudb::Type::Column(COL_2, avro::AVRO_STRING));
    columns.push_back(gpudb::Type::Column(COL_GROUP_ID,avro::AVRO_STRING));
    gpudb::Type myType("my_type_1", columns);

    std::string type_id_1 = myType.create(h_db);
    std::cout << "GPUdb generated type id for the new type -  " << type_id_1 << std::endl;

    try
    {
        gpudb::CreateTableResponse response = h_db.createTable(my_table, type_id_1, options);
    }
    catch(const std::exception& ex)
    {
        std::cout << "Caught Exception: " << ex.what() << std::endl;
        return 10;
    }


    try
    {
        // Insert some records into the table
        insertRecordsLocal(h_db, myType.getSchema(), 1, 10, 0.1, "string ", STR_GROUP_1_CONST, true);

        // Call the helper function to Retrieve records from the table
        gpudb::GetRecordsResponse<gpudb::GenericRecord> response = h_db.getRecords<gpudb::GenericRecord>(myType.getSchema(), my_table,0,100,options);
        print_GetRecordsResponse(response);

        // Filter the records by an expression into a view
        std::string my_view("my_view_1");
        std::string my_filterExpr("col1=1.1");
        gpudb::FilterResponse filter_resp = h_db.filter(my_table, my_view, my_filterExpr, options);
        std::cout << "Number of records returned by filter expresion " << filter_resp.count << std::endl;

        // Retrieve records from the view
        response = h_db.getRecords<gpudb::GenericRecord>(myType.getSchema(), my_view,0,100,options);
        print_GetRecordsResponse(response);

        // Drop the view (same function as dropping a table)
        gpudb::ClearTableResponse clrTbl_Resp = h_db.clearTable(my_view, "", options);

        // Apply a filter condition with two columns
        my_filterExpr="col1 <= 9 and group_id=\"Group 1\"";
        filter_resp = h_db.filter(my_table, my_view, my_filterExpr, options);
        std::cout << "Number of records returned by second filter expresion " << filter_resp.count << std::endl;

        response = h_db.getRecords<gpudb::GenericRecord>(myType.getSchema(), my_view,0,100,options);
        print_GetRecordsResponse(response);

        // Filter by a list of values.  Note also query chaining - query run on another view.
        std::string my_view2("my_view_2");
        std::map<std::string, std::vector<std::string> > my_filterList;
        my_filterList["col1"].push_back("1.1");
        my_filterList["col1"].push_back("2.1");
        my_filterList["col1"].push_back("5.1");
        gpudb::FilterByListResponse filterByList_resp = h_db.filterByList(my_view, my_view2, my_filterList, options);
        std::cout << "Number of records returned by filter expresion " << filterByList_resp.count << std::endl;

        response = h_db.getRecords<gpudb::GenericRecord>(myType.getSchema(), my_view2,0,100,options);
        print_GetRecordsResponse(response);

        // Filter by a range
        std::string my_view3("my_view_3");
        gpudb::FilterByRangeResponse filterByRange_resp = h_db.filterByRange(my_view, my_view3, COL_1, 1, 5, options);
        std::cout << "Number of records returned by filter expresion " << filterByRange_resp.count << std::endl;

        response = h_db.getRecords<gpudb::GenericRecord>(myType.getSchema(), my_view3,0,100,options);
        print_GetRecordsResponse(response);

        // Insert New Records
        insertRecordsLocal(h_db, myType.getSchema(), 1, 8, 10.1, "string ", STR_GROUP_2_CONST, true);

        // Retrieve unique values for a column
        gpudb::AggregateUniqueResponse aggrUniq_resp = h_db.aggregateUnique(my_table, COL_GROUP_ID, 0, 100, options);
        std::cout << "Unique values in group_id column " << aggrUniq_resp.data.size() << std::endl;
        std::cout << "Schema " << aggrUniq_resp.responseSchemaStr << std::endl;
        for (size_t i = 0 ; i < aggrUniq_resp.data.size(); ++i )
        {
            const gpudb::DynamicTableRecord& record = aggrUniq_resp.data[i];

            // already know the returned column and its type ; just print it.
            assert (record.getFieldCount() == 1);
            std::cout << '"' << record.value<std::string>(0) << "\", ";
        }

        // "Group by" query
        std::vector<std::string> col_names;
        col_names.push_back(COL_2);
        gpudb::AggregateGroupByResponse aggrGB_resp = h_db.aggregateGroupBy(my_table, col_names, 0, 100, options);
        std::cout << "Number of unique values in col2  " << aggrGB_resp.data.size() << std::endl;
        std::cout << "Schema " << aggrGB_resp.responseSchemaStr << std::endl;
        for (size_t i = 0 ; i < aggrGB_resp.data.size(); ++i )
        {
            const gpudb::DynamicTableRecord& record = aggrGB_resp.data[i];

            assert (record.getFieldCount() == 2);
            std::cout << '"' << record.value<std::string>(0) << '"';
            std::cout << ':' << record.value<double>(1) << std::endl;
        }

        col_names.clear();
        col_names.push_back(COL_GROUP_ID);
        col_names.push_back("count(*)");
        col_names.push_back("sum(col1)");
        col_names.push_back("avg(col1)");
        aggrGB_resp = h_db.aggregateGroupBy(my_table, col_names, 0, 100, options);
        std::cout << "Number of unique values in group_id column " << aggrGB_resp.data.size() << std::endl;
        std::cout << "Schema " << aggrGB_resp.responseSchemaStr << std::endl;

        // Parse the group by response.
        const gpudb::DynamicTableRecord& rec1 = aggrGB_resp.data[0];
        for (size_t j = 0 ; j < rec1.getFieldCount() ; j++)
        {
            std::cout << rec1.getExpression(j) << '\t';
        }
        std::cout << std::endl;

        // print the values from the response
        for (size_t i = 0 ; i < aggrGB_resp.data.size(); ++i )
        {
            const gpudb::DynamicTableRecord& record = aggrGB_resp.data[i];
            assert (record.getFieldCount() == col_names.size());

            for (size_t j = 0 ; j < record.getFieldCount() ; j++)
            {
                switch (record.getFieldType(j))
                {
                case avro::AVRO_STRING:
                    std::cout << '"' << record.value<std::string>(j) << "\"\t";
                    break;

                case avro::AVRO_DOUBLE:
                    std::cout << record.value<double>(j) << '\t';
                    break;

                case avro::AVRO_FLOAT:
                    std::cout << record.value<float>(j) << '\t';
                    break;

                case avro::AVRO_INT:
                    std::cout << record.value<float>(j) << '\t';
                    break;

                case avro::AVRO_LONG:
                    std::cout << record.value<float>(j) << '\t';
                    break;

                default:
                    std::cout << "!Unhandled Type!" << '\t';
                }
            }
            std::cout << std::endl;
        }

        // expressions in group by
        col_names.clear();
        col_names.push_back(COL_GROUP_ID);
        col_names.push_back("sum(col1*10)");
        aggrGB_resp = h_db.aggregateGroupBy(my_table, col_names, 0, 100, options);
        std::cout << "Unique values in group_id column " << aggrGB_resp.data.size() << std::endl;
        std::cout << "Schema " << aggrGB_resp.responseSchemaStr << std::endl;

        const gpudb::DynamicTableRecord& rec2 = aggrGB_resp.data[0];
        for (size_t j = 0 ; j < rec2.getFieldCount() ; j++)
        {
            std::cout << rec2.getExpression(j) << '\t';
        }
        std::cout << std::endl;

        for (size_t i = 0 ; i < aggrGB_resp.data.size(); ++i )
        {
            const gpudb::DynamicTableRecord& record = aggrGB_resp.data[i];
            assert (record.getFieldCount() == col_names.size());

            for (size_t j = 0 ; j < record.getFieldCount() ; j++)
            {
                switch (record.getFieldType(j))
                {
                case avro::AVRO_STRING:
                    std::cout << '"' << record.value<std::string>(j) << "\"\t";
                    break;

                case avro::AVRO_DOUBLE:
                    std::cout << record.value<double>(j) << '\t';
                    break;

                case avro::AVRO_FLOAT:
                    std::cout << record.value<float>(j) << '\t';
                    break;

                case avro::AVRO_INT:
                    std::cout << record.value<float>(j) << '\t';
                    break;

                case avro::AVRO_LONG:
                    std::cout << record.value<float>(j) << '\t';
                    break;

                default:
                    std::cout << "!Unhandled Type!" << '\t';
                }
            }
            std::cout << std::endl;
        }

        // Insert few more records
        insertRecordsLocal(h_db, myType.getSchema(), 4, 10, 0.6, "string 2", STR_GROUP_1_CONST, true);

        // Aggregate Histogram
        gpudb::AggregateHistogramResponse aggrHist_resp = h_db.aggregateHistogram(my_table, COL_1, 0, 11, 1, options);
        std::cout << "Num Histogram bins: " << aggrHist_resp.counts.size()
                  << "  Start: " << aggrHist_resp.start
                  << "  end: " << aggrHist_resp.end << std::endl;
        std::cout << "Count per bin - ";
        std::copy(aggrHist_resp.counts.begin(), aggrHist_resp.counts.end(), std::ostream_iterator<int>(std::cout, ", "));
        std::cout << std::endl;

    }
    catch(const std::exception& ex)
    {
        std::cout << "Caught Exception: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}


// Helper functions

// Insert new records to the table
void insertRecordsLocal(gpudb::GPUdb& h_db, avro::ValidSchema myTypeSchema, int start, int end, double col1_const, std::string col2_const, std::string group_id_const, bool display_record_ids)
{
    std::map<std::string, std::string> options;
    std::vector<gpudb::GenericRecord> record_list;
    for (int i = start ; i < end ; i++)
    {
        gpudb::GenericRecord datum(myTypeSchema);
        datum.asDouble(COL_1) = i + col1_const;
        std::ostringstream ostr;
        ostr << col2_const << i;
        datum.asString(COL_2) = ostr.str();
        datum.asString(COL_GROUP_ID) = group_id_const;
        record_list.push_back(datum);
    }
    options["return_record_ids"] = "true";  // optionally request record_ids
    gpudb::InsertRecordsResponse response = h_db.insertRecords(my_table, record_list, options);
    options.clear();

    if(display_record_ids)
    {
        std::cout << "Record Ids for " << response.countInserted << " new records - [";
        std::copy(response.recordIds.begin(), response.recordIds.end()-1, std::ostream_iterator<std::string>(std::cout, "', '") );
        std::cout << "'" << *(response.recordIds.end()-1) << "']" << std::endl;
    }
    else
    {
        std::cout << response.countInserted << " new records inserted." << std::endl;
    }

}

// Print the records from the table
void print_GetRecordsResponse(const gpudb::GetRecordsResponse<gpudb::GenericRecord>& response)
{
    std::cout << "response schema - " << response.typeSchema << std::endl;
    for(unsigned int i = 0 ; i < response.data.size() ; ++i)
    {
        const gpudb::GenericRecord& record = response.data[i];
        std::cout << "\""     << COL_1        << "\":"   << record.asDouble(COL_1)
                  << ", \""   << COL_2        << "\":\"" << record.asString(COL_2) << std::flush
                  << "\", \"" << COL_GROUP_ID << "\":\"" << record.asString(COL_GROUP_ID) << "\"\n";
    }

}

