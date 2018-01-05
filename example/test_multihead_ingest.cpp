#include "gpudb/GPUdb.hpp"
#include "gpudb/GPUdbIngestor.hpp"

#include <exception>

#define NORMALIZER 100
#define USE_NULL_VALUE(null_percentage) \
    ((std::rand() % NORMALIZER) < null_percentage)

#define PERFORM_ACTION( percentage ) \
    ((std::rand() % NORMALIZER) < percentage)

#define GENERATE_INT8() \
    ( (std::rand() % 256) - 128)

#define GENERATE_INT16() \
    ( (std::rand() % 65536) - 32768)

#define GENERATE_FLOAT() \
    ( ((float)std::rand()) / std::rand() * std::pow(-1, (std::rand() % 2)) )


#define GENERATE_DOUBLE() \
    ( ((double)std::rand()) / std::rand() * std::pow(-1, (std::rand() % 2)) )

#define GENERATE_TIMESTAMP() \
    ( std::rand() - std::rand() )  // Range is actually [-30610239758979, 29379542399999]


// For generating strings and charNs
static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static const int alphabet_size = 62; // [0-9] ten, [A-Z] 26, [a-z] 26

/*
 * Generate a random string made of the characters in [A-Za-z0-9], up to
 * 256 characters in length.
 */
inline std::string generate_random_string()
{
    size_t max_len = 256;
    size_t str_len = (std::rand() % max_len);
    std::string str_value( str_len, ' ' );
    for ( size_t i = 0; i < str_len; ++i )
    {
        str_value[ i ] = alphanum[ std::rand() % alphabet_size ];
    }
    return str_value;
}  // end generate_random_string


/*
 * Generate a random string made of the characters in [A-Za-z0-9], up to
 * N characters in length.
 */
inline std::string generate_charN( size_t N )
{
    size_t str_len = (std::rand() % (N + 1));
    std::string str_value( str_len, ' ' );
    for ( size_t i = 0; i < str_len; ++i )
    {
        str_value[ i ] = alphanum[ std::rand() % alphabet_size ];
    }
    return str_value;
}  // end generate_charN


/// Generate a random IPv4 address ('x.x.x.x' where x is in [0, 255])
#define GENERATE_IPV4() \
    ( std::to_string(std::rand() % 256) + "." \
      + std::to_string(std::rand() % 256) + "." \
      + std::to_string(std::rand() % 256) + "." \
      + std::to_string(std::rand() % 256) )


/*
 * Generate a random date within the year range [1000, 2900]
 */
inline std::string generate_date()
{
    // Get random year and month
    int year  = ( (std::rand() % 1900) + 1000 ); // within [1000, 2900]
    int month = ( ( std::rand() % 12 ) + 1 );    // within [1, 12]

    // Get a random day
    //                             J,  F,  M,  A,  M,  J,  J,  A,  S,  O,  N,  D
    const int days_per_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int day = ( ( std::rand() % days_per_month[ month - 1 ] ) + 1);

    char buf[ 11 ];
    sprintf( buf, "%d-%02d-%02d", year, month, day );
    return std::string( buf );
}  // end generate_date



/*
 * Generate a random datetime with optional milliseconds
 */
inline std::string generate_datetime( int chance_percentage_has_time, int chance_percentage_has_ms )
{
    if ( ( chance_percentage_has_time < 0 ) || ( chance_percentage_has_time > 100 ) )
        throw std::invalid_argument( "generate_time(): chance_percentage_has_time should be within the range [0, 100]; given " + std::to_string( chance_percentage_has_time ) );

    if ( ( chance_percentage_has_ms < 0 ) || ( chance_percentage_has_ms > 100 ) )
        throw std::invalid_argument( "generate_time(): chance_percentage_has_ms should be within the range [0, 100]; given " + std::to_string( chance_percentage_has_ms ) );

    // Get random year and month
    int year  = ( (std::rand() % 1900) + 1000 ); // within [1000, 2900]
    int month = ( ( std::rand() % 12 ) + 1 );    // within [1, 12]

    // Get a random day
    //                             J,  F,  M,  A,  M,  J,  J,  A,  S,  O,  N,  D
    const int days_per_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int day = ( ( std::rand() % days_per_month[ month - 1 ] ) + 1);

    // If no time is to be included, return just the date
    if ( PERFORM_ACTION( chance_percentage_has_time ) )
    {
        char buf[ 11 ];
        sprintf( buf, "%d-%02d-%02d", year, month, day );
        return std::string( buf );
    }
    // else, need to generate a time component:
    // ----------------------------------------

    // Get random hour, minute, second
    int hour   = ( std::rand() % 24 );  // within [0, 23]
    int minute = ( std::rand() % 60 );  // within [0, 59]
    int second = ( std::rand() % 60 );  // within [0, 59]

    char buf[ 30 ];

    // Get a random millisecond, if any
    if ( PERFORM_ACTION( chance_percentage_has_ms ) )
    {
        int ms = 0;
        // Generate milliseconds of a length within [1, 6]
        switch ( std::rand() % 6 )
        {
            case 0:  // length 1: range is [0, 9]
                ms = ( std::rand() % 10 ); break;

            case 1:  // length 2: range is [10, 99]
                ms = ( ( std::rand() % 90 ) + 10); break;

            case 2:  // length 3: range is [100, 999]
                ms = ( ( std::rand() % 900 ) + 100); break;

            case 3:  // length 4: range is [1000, 9999]
                ms = ( ( std::rand() % 9000 ) + 1000); break;

            case 4:  // length 5: range is [10000, 99999]
                ms = ( ( std::rand() % 90000 ) + 10000); break;

            case 5:  // length 6: range is [100000, 999999]
                ms = ( ( std::rand() % 900000 ) + 100000); break;
        }


        // 50% of the time we get hours less than 10, pad with zero
        if ( (hour < 10) && ((std::rand() % 2) == 1) )
            sprintf( buf, "%d-%02d-%02d %02d:%02d:%02d.%d", year, month, day, hour, minute, second, ms );
        else  // no padding for small hour
            sprintf( buf, "%d-%02d-%02d %d:%02d:%02d.%d", year, month, day, hour, minute, second, ms );
    }
    else  // generate a time without any milliseconds
    {
        // 50% of the time we get hours less than 10, pad with zero
        if ( (hour < 10) && ((std::rand() % 2) == 1) )
            sprintf( buf, "%d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second );
        else  // no padding for small hour
            sprintf( buf, "%d-%02d-%02d %d:%02d:%02d", year, month, day, hour, minute, second );
    }

    return std::string( buf );
}  // end generate_datetime


/*
 * Generate a random time with optional milliseconds
 */
inline std::string generate_time( int chance_percentage_has_ms )
{
    if ( ( chance_percentage_has_ms < 0 ) || ( chance_percentage_has_ms > 100 ) )
        throw std::invalid_argument( "generate_time(): chance_percentage_has_ms should be within the range [0, 100]; given " + std::to_string( chance_percentage_has_ms ) );

    // Get random hour, minute, second
    int hour   = ( std::rand() % 24 );  // within [0, 23]
    int minute = ( std::rand() % 60 );  // within [0, 59]
    int second = ( std::rand() % 60 );  // within [0, 59]

    char buf[ 15 ];

    // Get a random millisecond, if any
    if ( PERFORM_ACTION( chance_percentage_has_ms ) )
    {
        int ms = 0;
        // Generate milliseconds of any of the following lengths: 1, 2, 3
        switch ( std::rand() % 3 )
        {
            case 0:  // length 1: range is [0, 9]
                ms = ( std::rand() % 10 ); break;

            case 1:  // length 2: range is [10, 99]
                ms = ( ( std::rand() % 90 ) + 10); break;

            case 2:  // length 3: range is [100, 999]
                ms = ( ( std::rand() % 900 ) + 100); break;
        }


        // 50% of the time we get hours less than 10, pad with zero
        if ( (hour < 10) && ((std::rand() % 2) == 1) )
            sprintf( buf, "%02d:%02d:%02d.%d", hour, minute, second, ms );
        else  // no padding for small hour
            sprintf( buf, "%d:%02d:%02d.%d", hour, minute, second, ms );
    }
    else  // generate a time without any milliseconds
    {
        // 50% of the time we get hours less than 10, pad with zero
        if ( (hour < 10) && ((std::rand() % 2) == 1) )
            sprintf( buf, "%02d:%02d:%02d", hour, minute, second );
        else  // no padding for small hour
            sprintf( buf, "%d:%02d:%02d", hour, minute, second );
    }

    return std::string( buf );
}  // end generate_time



/*
 * Generate a random decimal value with optional fractions and signs.
 */
inline std::string generate_decimal( int frac_only_percentage, int has_frac_percentage, int negative_percentage, int positive_percentage )
{
    if ( ( frac_only_percentage < 0 ) || ( frac_only_percentage > 100 ) )
        throw std::invalid_argument( "generate_decimal(): frac_only_percentage should be within the range [0, 100]; given " + std::to_string( frac_only_percentage ) );

    if ( ( has_frac_percentage < 0 ) || ( has_frac_percentage > 100 ) )
        throw std::invalid_argument( "generate_decimal(): has_frac_percentage should be within the range [0, 100]; given " + std::to_string( has_frac_percentage ) );

    if ( ( negative_percentage < 0 ) || ( negative_percentage > 100 ) )
        throw std::invalid_argument( "generate_decimal(): negative_percentage should be within the range [0, 100]; given " + std::to_string( negative_percentage ) );

    if ( ( positive_percentage < 0 ) || ( positive_percentage > 100 ) )
        throw std::invalid_argument( "generate_decimal(): positive_percentage should be within the range [0, 100]; given " + std::to_string( positive_percentage ) );

    // The -ve and +ve percentages better not exceed 100%!
    if ( (negative_percentage + positive_percentage) > 100 )
        throw std::invalid_argument( "generate_decimal(): The sum of positive_percentage and negative_percentage should not exceed 100; given +ve " + std::to_string( positive_percentage ) + " and -ve " + std::to_string( negative_percentage ) );

    // Get the sign (either none, or minus sign or plus sign)
    std::string sign = "";
    if ( PERFORM_ACTION( negative_percentage ) )
        sign = "-";
    else if ( PERFORM_ACTION( positive_percentage ) )
        sign = "+";

    // The range of values: [-922337203685477.5808, 922337203685477.5807]

    // Generate the scale (up to 10,000)
    std::string scale = std::to_string( std::rand() % 10000 );

    // Are we generating only a fraction or a precision as well?
    if ( PERFORM_ACTION( frac_only_percentage ) )
    {
        return (sign + "." + scale);
    }

    // Need to generate the precision (the integral part)
    static const char digits[] = "0123456789";
    static const int num_digits = 10; // [0-9] ten

    // We'll go only up to 14 digits for the precision so that we need not
    // worry about going beyond the max precision 922337203685477
    static const size_t max_len = 13;
    size_t precision_len = ((std::rand() % max_len) + 1); // [1, 14]
    std::string precision_value( precision_len, ' ' );
    for ( size_t i = 0; i < precision_len; ++i )
    {
        precision_value[ i ] = digits[ std::rand() % num_digits ];
    }

    // Does the number include the fraction/scale?
    if ( PERFORM_ACTION( frac_only_percentage ) )
    {  // no fraction; so put together the sign and the precision
        return (sign + precision_value);
    }

    // Put the sign, precision and scale together
    return (sign + precision_value + "." + scale);
}  // end generate_decimal



void test_all_columns_sharding( const gpudb::GPUdb &db )
{
    std::cout << "Test multi-head ingestion with sharding on all possible column types" << std::endl;
    std::cout << "====================================================================" << std::endl;

    try
    {
        // Create a type
        std::vector<gpudb::Type::Column> columns;
        columns.push_back( gpudb::Type::Column("a",  gpudb::Type::Column::INT) );
        columns.push_back( gpudb::Type::Column("i",  gpudb::Type::Column::INT, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE ) );
        columns.push_back( gpudb::Type::Column("i8", gpudb::Type::Column::INT, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::INT8 ) );
        columns.push_back( gpudb::Type::Column("i16", gpudb::Type::Column::INT, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::INT16 ) );
        columns.push_back( gpudb::Type::Column("d", gpudb::Type::Column::DOUBLE, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE ) );
        columns.push_back( gpudb::Type::Column("f", gpudb::Type::Column::FLOAT, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE ) );
        columns.push_back( gpudb::Type::Column("l", gpudb::Type::Column::LONG, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE ) );
        columns.push_back( gpudb::Type::Column("timestamp", gpudb::Type::Column::LONG, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::TIMESTAMP ) );
        columns.push_back( gpudb::Type::Column("s",    gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE ) );
        columns.push_back( gpudb::Type::Column("c1",   gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::CHAR1   ) );
        columns.push_back( gpudb::Type::Column("c2",   gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::CHAR2   ) );
        columns.push_back( gpudb::Type::Column("c4",   gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::CHAR4   ) );
        columns.push_back( gpudb::Type::Column("c8",   gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::CHAR8   ) );
        columns.push_back( gpudb::Type::Column("c16",  gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::CHAR16  ) );
        columns.push_back( gpudb::Type::Column("c32",  gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::CHAR32  ) );
        columns.push_back( gpudb::Type::Column("c64",  gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::CHAR64  ) );
        columns.push_back( gpudb::Type::Column("c128", gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::CHAR128 ) );
        columns.push_back( gpudb::Type::Column("c256", gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::CHAR256 ) );
        columns.push_back( gpudb::Type::Column("ipv4", gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::IPV4    ) );
        columns.push_back( gpudb::Type::Column("date", gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::DATE    ) );
        columns.push_back( gpudb::Type::Column("time", gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::TIME    ) );
        columns.push_back( gpudb::Type::Column("dt",   gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::DATETIME ) );
        columns.push_back( gpudb::Type::Column("decimal", gpudb::Type::Column::STRING, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE, gpudb::ColumnProperty::DECIMAL ) );

        gpudb::Type _type = gpudb::Type( "Test", columns );
        std::string type_id = _type.create( db );

        std::string table_name = "gpudb_ingestor_test";

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
        // No insert options or worker lists given
        gpudb::GPUdbIngestor ingestor1( db, _type, table_name, batch_size );

        // Insert options given, but no worker list given
        std::map<std::string, std::string> insert_options;
        insert_options[ gpudb::insert_records_return_record_ids ] = gpudb::insert_records_true;
        gpudb::GPUdbIngestor ingestor2( db, _type, table_name, insert_options, batch_size );

        // Worker list given, but no insert options given
        gpudb::WorkerList worker_list( db );
        std::cout << "Worker list: " << worker_list.toString() << std::endl;
        gpudb::GPUdbIngestor ingestor3( db, _type, table_name, worker_list, batch_size );

        // Both worker list and insert options given
        gpudb::GPUdbIngestor ingestor4( db, _type, table_name, worker_list, insert_options, batch_size );
        std::cout << "Count inserted: " << ingestor4.getCountInserted() << std::endl;
        std::cout << "Count updated: " << ingestor4.getCountUpdated() << std::endl;


        // Test insertion
        // ==============
        // Create records and insert them
        int32_t null_percentage      = 10;  // 10%
        int32_t time_ms_percentage   = 50;  // 50%
        int32_t time_percentage      = 70;  // 70%
        int32_t frac_only_percentage = 35;  // 35%; for decimal
        int32_t has_frac_percentage  = 70;  // 70%; for decimal
        int32_t negative_percentage  = 40;  // 40%; for decimal
        int32_t positive_percentage  = 10;  // 10%; for decimal

        size_t num_batches = 5;
        size_t num_records = 1000;

        // Run # num_batches batches
        for ( size_t i = 0; i < num_batches; ++i )
        {
            std::vector<gpudb::GenericRecord> records;

            // Generate # num_records records for each batch
            for ( size_t j = 0; j < num_records; ++j )
            {
                std::cout << "outer loop #: " << i << " inner loop #: " << j << std::endl;  // degug~~~~~~~
                gpudb::GenericRecord record( _type );
                // Regular int, no sharding
                record.intValue("a") = 1;

                // int, regular
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "i" );
                else
                    record.setAsNullableInt( "i", boost::optional<int32_t>( std::rand() ) );

                // int8
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "i8" );
                else
                    record.setAsNullableInt( "i8", boost::optional<int32_t>( GENERATE_INT8() ) );

                // int16
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "i16" );
                else
                    record.setAsNullableInt( "i16", boost::optional<int32_t>( GENERATE_INT16() ) );

                // double
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "d" );
                else
                    record.setAsNullableDouble( "d", boost::optional<double>( GENERATE_DOUBLE() ) );

                // float
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "f" );
                else
                    record.setAsNullableFloat( "f", boost::optional<float>( GENERATE_FLOAT() ) );

                // long
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "l" );
                else
                    record.setAsNullableLong( "l", boost::optional<int64_t>( (long)std::rand() ) );

                // timestamp (long)
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "timestamp" );
                else
                    record.setAsNullableLong( "timestamp", boost::optional<int64_t>( GENERATE_TIMESTAMP() ) );

                // string (regular)
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "s" );
                else
                    record.setAsNullableString( "s", boost::optional<std::string>( generate_random_string() ) );

                // char1
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "c1" );
                else
                    record.setAsNullableString( "c1", boost::optional<std::string>( generate_charN( 1 ) ) );

                // char2
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "c2" );
                else
                    record.setAsNullableString( "c2", boost::optional<std::string>( generate_charN( 2 ) ) );

                // char4
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "c4" );
                else
                    record.setAsNullableString( "c4", boost::optional<std::string>( generate_charN( 4 ) ) );

                // char8
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "c8" );
                else
                    record.setAsNullableString( "c8", boost::optional<std::string>( generate_charN( 8 ) ) );

                // char16
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "c16" );
                else
                    record.setAsNullableString( "c16", boost::optional<std::string>( generate_charN( 16 ) ) );

                // char32
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "c32" );
                else
                    record.setAsNullableString( "c32", boost::optional<std::string>( generate_charN( 32 ) ) );

                // char64
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "c64" );
                else
                {
    //                record.setAsNullableString( "c64", boost::optional<std::string>( "01234567891123456789212345678931234567894123456789512345678912345" ) ); // too long
                    record.setAsNullableString( "c64", boost::optional<std::string>( generate_charN( 64 ) ) );
                }

                // char128
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "c128" );
                else
                {
    //                record.setAsNullableString( "c128", boost::optional<std::string>( "012345678911234567892123456789312345678941234567895123456789123401234567891123456789212345678931234567894123456789512345678912345" ) ); // too long
    //                record.setAsNullableString( "c128", boost::optional<std::string>( "01234567891123456789212345678931234567894123456789512345678912340123456789112345678921234567893123456789412345678951234567891234" ) ); // 128 long
                    record.setAsNullableString( "c128", boost::optional<std::string>( generate_charN( 128 ) ) );
                }

                // char256
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "c256" );
                else
                {
    //                record.setAsNullableString( "c256", boost::optional<std::string>( "01234567891123456789212345678931234567894123456789512345678912340123456789112345678921234567893123456789412345678951234567891234012345678911234567892123456789312345678941234567895123456789123401234567891123456789212345678931234567894123456789512345678912345" ) ); // too long
    //                record.setAsNullableString( "c256", boost::optional<std::string>( "0123456789112345678921234567893123456789412345678951234567891234012345678911234567892123456789312345678941234567895123456789123401234567891123456789212345678931234567894123456789512345678912340123456789112345678921234567893123456789412345678951234567891234" ) ); // 256 long
                    record.setAsNullableString( "c256", boost::optional<std::string>( generate_charN( 256 ) ) );
                }

                // IPv4
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "ipv4" );
                else
                {
    //                record.setAsNullableString( "ipv4", boost::optional<std::string>( "333.2.3.4" ) );  // illegal
    //                record.setAsNullableString( "ipv4", boost::optional<std::string>( "-1.2.3.4" ) );  // illegal
    //                record.setAsNullableString( "ipv4", boost::optional<std::string>( "01.2.3.4" ) );  // legal
    //                record.setAsNullableString( "ipv4", boost::optional<std::string>( "1.2.3.4." ) );  // illegal
    //                record.setAsNullableString( "ipv4", boost::optional<std::string>( "1.2.3.4.5" ) );  // illegal
    //                record.setAsNullableString( "ipv4", boost::optional<std::string>( "2.k.2.3" ) );  // illegal
    //                record.setAsNullableString( "ipv4", boost::optional<std::string>( "2l3k4jl" ) );  // illegal
                    record.setAsNullableString( "ipv4", boost::optional<std::string>( GENERATE_IPV4() ) );
                }

                // Date
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "date" );
                else
                {
    //                record.setAsNullableString( "date", boost::optional<std::string>( "2343-02-29" ) );  // invalid (bad day--non-existing leap day)
    //                record.setAsNullableString( "date", boost::optional<std::string>( "5343-21-09" ) );  // invalid (bad month)
    //                record.setAsNullableString( "date", boost::optional<std::string>( "5343-12-09" ) );  // invalid (bad year)
    //                record.setAsNullableString( "date", boost::optional<std::string>( "2343-12-9" ) );  // invalid (no match)
    //                record.setAsNullableString( "date", boost::optional<std::string>( "234k-23-l9" ) );  // invalid (no match)
    //                record.setAsNullableString( "date", boost::optional<std::string>( "        " ) );  // invalid (no match)
    //                record.setAsNullableString( "date", boost::optional<std::string>( "     " + generate_date() + "   " ) );  // valid with space before and after
    //                record.setAsNullableString( "date", boost::optional<std::string>( " " + generate_date() ) );  // valid with space before
    //                record.setAsNullableString( "date", boost::optional<std::string>( generate_date() + " " ) );  // valid with space after
                    record.setAsNullableString( "date", boost::optional<std::string>( generate_date() ) );
                }

                // Time
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "time" );
                else
                {
    //                record.setAsNullableString( "time", boost::optional<std::string>( "\n 11:22:33.44\t" ) ); // valid; space before and after
    //                record.setAsNullableString( "time", boost::optional<std::string>( "11:22:33.44\t" ) ); // valid; space after
    //                record.setAsNullableString( "time", boost::optional<std::string>( " 11:22:33.44" ) ); // valid; space before
    //                record.setAsNullableString( "time", boost::optional<std::string>( "30:22:33" ) );  // invalid hour
    //                record.setAsNullableString( "time", boost::optional<std::string>( "03:66:33" ) );  // invalid minute
    //                record.setAsNullableString( "time", boost::optional<std::string>( "03:26:73" ) );  // invalid second
    //                record.setAsNullableString( "time", boost::optional<std::string>( "03:26:33.3489" ) );  // invalid millisecond
    //                record.setAsNullableString( "time", boost::optional<std::string>( "03:26:33." ) );  // invalid millisecond
    //                record.setAsNullableString( "time", boost::optional<std::string>( "lksj:23:8s" ) );  // invalid; does not match regex
                    record.setAsNullableString( "time", boost::optional<std::string>( generate_time( time_ms_percentage ) ) );
                }

                // Datetime
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "dt" );
                else
                {
    //                record.setAsNullableString( "dt", boost::optional<std::string>( "1342-03-09 11:22:33.1234567" ) ); // invalid millisecond
    //                record.setAsNullableString( "dt", boost::optional<std::string>( "1342-03-09 11:42:63.44" ) ); // invalid second
    //                record.setAsNullableString( "dt", boost::optional<std::string>( "1342-03-09 11:62:33.44" ) ); // invalid minute
    //                record.setAsNullableString( "dt", boost::optional<std::string>( "1342-03-09 41:22:33.44" ) ); // invalid hour
    //                record.setAsNullableString( "dt", boost::optional<std::string>( "1342-03-99 11:22:33.44" ) ); // invalid day
    //                record.setAsNullableString( "dt", boost::optional<std::string>( "1342-33-09 11:22:33.44" ) ); // invalid month
    //                record.setAsNullableString( "dt", boost::optional<std::string>( "3342-03-09 11:22:33.44" ) ); // invalid year
    //                record.setAsNullableString( "dt", boost::optional<std::string>( "2342:03-09 11:22:33.44" ) ); // invalid; regex mismatch
    //                record.setAsNullableString( "dt", boost::optional<std::string>( "2000-1223-09 11:22:33.44  \t" ) ); // invalid; regex mismatch
    //                record.setAsNullableString( "dt", boost::optional<std::string>( "2000-12-09 11:22:33.44  \t" ) ); // valid; space after
    //                record.setAsNullableString( "dt", boost::optional<std::string>( "        \t      2000-12-09 11:22:33.44" ) ); // valid; space before
    //                record.setAsNullableString( "dt", boost::optional<std::string>( " 1919-09-09 11:22:33.44\t" ) ); // valid; space before and after
    //                record.setAsNullableString( "dt", boost::optional<std::string>( "\n 1919-09-09 11:22:33.44\t" ) ); // valid; space before and after
                    record.setAsNullableString( "dt", boost::optional<std::string>( generate_datetime( time_percentage, time_ms_percentage ) ) );
                }

                // Decimal
                if ( USE_NULL_VALUE( null_percentage ) )
                    record.setNull( "decimal" );
                else
                {
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "-0.990100" ) ); // invalid; scale too big/small
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "0.990100" ) ); // invalid; scale too big
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "-932337203685477.234" ) ); // invalid; precision too big
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "932337203685477.234" ) ); // invalid; precision too big
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "  -1234.923 " ) ); // seemingly invalid; space before and after
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "  -1234.9999" ) ); // valid; space before
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "-1234.999  " ) ); // seemingly invalid; space after
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "-.10000" ) ); // invalid: too big a fraction
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( ".10000" ) ); // invalid: too big a fraction
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "-.9999" ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( ".9999" ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "-922337203685478" ) ); // invalid: too small int
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "-922337203685477" ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "922337203685478" ) ); // invalid: too big int
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "922337203685477" ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "-.456" ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "+.456" ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( ".456" ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( ".456" ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "-123" ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "+123" ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "123" ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "123." ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "+123." ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "-123." ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "-123.4" ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "-123.4" ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "+123.456" ) ); // valid
    //                record.setAsNullableString( "decimal", boost::optional<std::string>( "123.456" ) ); // valid
                    record.setAsNullableString( "decimal", boost::optional<std::string>( generate_decimal( frac_only_percentage, has_frac_percentage, negative_percentage, positive_percentage ) ) );
                }

                std::cout << "generated record " << record << std::endl << std::endl;  // degug~~~~~~~

                records.push_back( record );

    //                ingestor1.insert( record );
    //                std::cout << "inserted record" << std::endl;  // degug~~~~~~~
    //                ingestor1.flush(); // debug~~~~~~~~~~~~~~
    //                std::cout << "flushed record--------------" << std::endl << std::endl;  // degug~~~~~~~
            }

            // Insert this batch of records into the ingestor
            ingestor1.insert( records );
        }

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

}  // end test_all_columns_sharding




void test_sharding_pk( const gpudb::GPUdb &db )
{
    std::cout << "Test multi-head ingestion with a primary key column" << std::endl;
    std::cout << "===================================================" << std::endl;

    try
    {
        // Create a type
        std::vector<gpudb::Type::Column> columns;
        columns.push_back( gpudb::Type::Column("c2",    gpudb::Type::Column::STRING, gpudb::ColumnProperty::PRIMARY_KEY, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::CHAR2 ) );
        columns.push_back( gpudb::Type::Column("c1", gpudb::Type::Column::STRING, gpudb::ColumnProperty::PRIMARY_KEY, gpudb::ColumnProperty::CHAR1 ) );

        gpudb::Type _type = gpudb::Type( "Test2", columns );
        std::string type_id = _type.create( db );

        std::string table_name = "gpudb_ingestor_test2";

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

        // Insert into the same table, but update on existing primary keys
        std::map<std::string, std::string> insert_options;
        insert_options[ gpudb::insert_records_update_on_existing_pk ] = gpudb::insert_records_true;
        gpudb::GPUdbIngestor ingestor2( db, _type, table_name, insert_options, batch_size );

        // Test insertion
        // ==============
        // Create records and insert them
//        size_t num_batches = 1;
//        size_t num_records = 10;
        size_t num_batches = 10;
        size_t num_records = 1000;

        // Run # num_batches batches
        for ( size_t i = 0; i < num_batches; ++i )
        {
            std::vector<gpudb::GenericRecord> records;

            // Generate # num_records records for each batch
            for ( size_t j = 0; j < num_records; ++j )
            {
                std::cout << "outer loop #: " << i << " inner loop #: " << j << std::endl;  // degug~~~~~~~
                gpudb::GenericRecord record( _type );

                // char2
                record.setAsNullableString( "c2", boost::optional<std::string>( generate_charN( 2 ) ) );

                // char1
                record.setAsNullableString( "c1", boost::optional<std::string>( generate_charN( 1 ) ) );

                std::cout << "generated record " << record << std::endl << std::endl;  // degug~~~~~~~

                records.push_back( record );

    //                // Test single record insertion
    //                ingestor1.insert( record );
    //                std::cout << "inserted record" << std::endl;  // degug~~~~~~~
    //                ingestor1.flush(); // debug~~~~~~~~~~~~~~
    //                std::cout << "flushed record--------------" << std::endl << std::endl;  // degug~~~~~~~
            }

            std::cout << "test_sharding_pk() finished generating the first batch of records; before insert" << std::endl; // debug~~~~~
            // Insert this batch of records into the ingestor
            ingestor1.insert( records );
            ingestor2.insert( records );
            std::cout << "test_sharding_pk() finished generating the first batch of records; after insert" << std::endl; // debug~~~~~
        }
        std::cout << "test_sharding_pk() finished generating and inserting records" << std::endl; // debug~~~~~

        // Print out the table size
        std::map<std::string, std::string> show_table_options;
        show_table_options[ gpudb::show_table_get_sizes ] = gpudb::show_table_true;
        size_t table_size = db.showTable( table_name, show_table_options ).totalSize;
        std::cout << "Table size (some are probably still in the queue): " << table_size << std::endl;

        // Flush the remaining records
        std::printf("\ntrying to flush the remaining records...\n"); // debug~~~~~~
        ingestor1.flush();
        ingestor2.flush();
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




void test_type_compatibility( const gpudb::GPUdb &db )
{
    std::cout << "Test type compatibilty" << std::endl;
    std::cout << "======================" << std::endl;

    // Create a type
    std::vector<gpudb::Type::Column> columns;
    columns.push_back( gpudb::Type::Column("a",  gpudb::Type::Column::INT, gpudb::ColumnProperty::STORE_ONLY) );
    columns.push_back( gpudb::Type::Column("i",  gpudb::Type::Column::INT, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE ) );

    gpudb::Type type1 = gpudb::Type( "Test", columns );
    type1.create( db );

    // Create another type
    std::vector<gpudb::Type::Column> columns2;
    columns2.push_back( gpudb::Type::Column("a",  gpudb::Type::Column::INT) );
    columns2.push_back( gpudb::Type::Column("i",  gpudb::Type::Column::INT, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE ) );

    gpudb::Type type2 = gpudb::Type( "Test", columns2 );
    type2.create( db );

    // Create a third type
    std::vector<gpudb::Type::Column> columns3;
    columns3.push_back( gpudb::Type::Column("b",  gpudb::Type::Column::INT) );
    columns3.push_back( gpudb::Type::Column("i",  gpudb::Type::Column::INT, gpudb::ColumnProperty::SHARD_KEY, gpudb::ColumnProperty::NULLABLE ) );

    gpudb::Type type3 = gpudb::Type( "Test", columns3 );
    type3.create( db );

    std::cout << "Types 1 and 2, disregarding query compatibility (expect true): " << type1.isTypeCompatible( type2 ) << std::endl;
    std::cout << "Types 1 and 2, including query compatibility (expect false): " << type1.isTypeCompatible( type2, true ) << std::endl;
    std::cout << "Types 1 and 3, disregarding query compatibility (expect false): " << type1.isTypeCompatible( type3 ) << std::endl;
    std::cout << "Types 1 and 2, including query compatibility (expect false): " << type1.isTypeCompatible( type3, true ) << std::endl;
    std::cout << std::endl;

} // end test_type_compatibility



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

        // Simple type compatibility test
        test_type_compatibility( db );

        std::cout << "Test GPUdbIngestor:" << std::endl;
        std::cout << "===================" << std::endl;
        // Create a worker list
        gpudb::WorkerList workers( db );
        std::cout << "# workers: " << workers.size() << std::endl;
        std::cout << workers.toString() << std::endl;
        std::cout << std::endl;

        test_all_columns_sharding( db );
        test_sharding_pk( db );

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







