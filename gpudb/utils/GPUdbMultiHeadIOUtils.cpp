#define INCL_EXTRA_HTON_FUNCTIONS
#define NOMINMAX // On Windows, prevent defining min/max macros

#include "gpudb/utils/GPUdbMultiHeadIOUtils.h"

#include "gpudb/utils/MurmurHash3.h"

#include <cstdlib>  // srand, rand
#include <cstring>
#ifdef _MSC_VER
    // See: https://gist.github.com/panzi/6856583
    #include <WinSock2.h>
    #define htobe16(x)  htons(x)
    #define htobe32(x)  htonl(x)
    #define htobe64(x)  htonll(x)
#else
    #include <endian.h>
#endif
#include <map>
#include <sstream>
#include <time.h>   // time for seeding the rng


#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>



namespace gpudb
{

// --------------------- Implementation of Class WorkerList ----------------------

/* Creates a <see cref="WorkerList"/> object and automatically populates it with the
 * worker URLs from GPUdb to support multi-head ingest. ( If the
 * specified GPUdb instance has multi-head ingest disabled, the worker
 * list will be empty and multi-head ingest will not be used.) Note that
 * in some cases, workers may be configured to use more than one IP
 * address, not all of which may be accessible to the client; this
 * constructor uses the first IP returned by the server for each worker.
 * </summary>
 * 
 * <param name="db">The <see cref="GPUdb"/> instance from which to
 * obtain the worker URLs.</param>
 */
WorkerList::WorkerList( const GPUdb &gpudb )
{
    // Get the system properties information from the server
    std::map<std::string, std::string> options;
    gpudb::ShowSystemPropertiesResponse sys_properties_rsp;
    sys_properties_rsp = gpudb.showSystemProperties( options );
    std::map<std::string, std::string> &sys_property_map = sys_properties_rsp.propertyMap;

    // Check if multihead ingestion is enabled
    std::map<std::string, std::string>::iterator it;
    it = sys_property_map.find( gpudb::show_system_properties_conf_enable_worker_http_servers );
    if ( it == sys_property_map.end() )
    { // keyword not found
        throw GPUdbException( "Missing value for " + gpudb::show_system_properties_conf_enable_worker_http_servers );
    }

    // so, is multi-head ingestion enabled?
    bool multi_head_ingest_enabled = ( it->second == gpudb::show_system_properties_TRUE );
    if ( !multi_head_ingest_enabled )
    {   // nope, it is NOT enabled
        return; // nothing more to do
    }


    // Multi-head ingestion IS enabled; find the worker URLs; then
    // add them to the worker url vector.
    // -----------------------------------------------------------
    std::string url_keyword = "conf.worker_http_server_urls";
    it = sys_property_map.find( url_keyword );
    if ( it != sys_property_map.end() )
    { // keyword was found
        // Parse the URLs
        char delim = ';';
        std::vector<std::string> worker_url_lists;
        split_string( it->second, delim, worker_url_lists );

        // For each worker rank--except the very first (rank0)--add the URLs
        std::vector<std::string> worker_urls;  // will be cleared in split-string calls
        char url_delim = ',';
        for ( size_t i = 1; i < worker_url_lists.size(); ++i )
        {
            // Each of the URL entries is actually a list delimited by a comma
            split_string( worker_url_lists[ i ], url_delim, worker_urls );

            // Go through all URLs and see if any will work
            std::vector<std::string>::const_iterator it2;
            for( it2 = worker_urls.begin(); it2 != worker_urls.end(); ++it2 )
            {
                gpudb::HttpUrl worker_url( *it2 );
                m_worker_urls.push_back( worker_url );
                break; // no need to look at the remaining URLs in this group
            }  // end inner loop
        } // end outer loop
    }
    else  // URLs not found; try to construct from IP addresses and ports
    {
        // Get the IP addresses of the worker ranks
        std::string ip_keyword = gpudb::show_system_properties_conf_worker_http_server_ips;
        it = sys_property_map.find( ip_keyword );
        if ( it == sys_property_map.end() )
        { // keyword not found
            throw GPUdbException( "Missing value for " + ip_keyword );
        }

        // Parse the IP addresses
        char delim = ';';
        std::vector<std::string> worker_ip_lists;
        split_string( it->second, delim, worker_ip_lists );

        // Check that the there's at least one IP address given
        if ( worker_ip_lists.empty() )
            throw GPUdbException( "Empty value for " + ip_keyword );

        // Get the port numbers of the worker ranks
        std::string port_keyword = gpudb::show_system_properties_conf_worker_http_server_ports;
        it = sys_property_map.find( port_keyword );
        if ( it == sys_property_map.end() )
        { // keyword not found
            throw GPUdbException( "Missing value for " + port_keyword );
        }

        // Parse the ports
        std::vector<std::string> worker_ports;
        split_string( it->second, delim, worker_ports );

        // Check that the same number of IP addresses and ports are provided
        if ( worker_ip_lists.size() != worker_ports.size() )
            throw GPUdbException( "Inconsistent number of values for " + ip_keyword + " and " + port_keyword );

        // Get the protocol for the URLs to be created
        const std::string& protocol = gpudb.getUrl().getProtocol();
        
        // For each worker rank--except the very first (rank0)--create a URL from
        // the IP address and port number
        std::vector<std::string> worker_ips;  // will be cleared in split-string calls
        char ip_delim = ',';
        for ( size_t i = 1; i < worker_ip_lists.size(); ++i )
        {
            // Each of the IP entries is actually a list delimited by a comma
            split_string( worker_ip_lists[ i ], ip_delim, worker_ips );

            // Go through all IPs and see if any will work
            std::vector<std::string>::const_iterator it;
            for( it = worker_ips.begin(); it != worker_ips.end(); ++it )
            {
                std::string worker_url_str = (protocol + "://" + *it + ":" + worker_ports[ i ]);
                gpudb::HttpUrl worker_url( worker_url_str );
                m_worker_urls.push_back( worker_url );
                break; // no need to look at the remaining IPs in this group
            }  // end inner loop
        } // end outer loop
    }  // end if-else
    

    if ( m_worker_urls.empty() ) // O_o No worker found!
        throw GPUdbException( "No worker HTTP servers found." );

    return;
}  // end WorkerList constructor



/* Creates a <see cref="WorkerList"/> object and automatically populates it with the
 * worker URLs from GPUdb to support multi-head ingest. ( If the
 * specified GPUdb instance has multi-head ingest disabled, the worker
 * list will be empty and multi-head ingest will not be used.) Note that
 * in some cases, workers may be configured to use more than one IP
 * address, not all of which may be accessible to the client; this
 * constructor uses the provided regular expression to match the workers in each
 * group, and only uses matching workers, if any.
 * </summary>
 * 
 * <param name="db">The <see cref="GPUdb"/> instance from which to
 * obtain the worker URLs.</param>
 * <param name="ip_regex_str">A regular expression pattern for the IPs to match.</param>
 */
WorkerList::WorkerList(const GPUdb &gpudb, const std::string &ip_regex_str )
{
    // Generate the regular expression from the given pattern
    std::regex ip_regex;
    try
    {
        ip_regex = std::regex( ip_regex_str );
    } catch (const std::exception&)
    {
        throw GPUdbException( "Unable to create a regular expression from given pattern '"
                              + ip_regex_str + "'" );
    }

    // Get the system properties information from the server
    std::map<std::string, std::string> options;
    gpudb::ShowSystemPropertiesResponse sys_properties_rsp;
    sys_properties_rsp = gpudb.showSystemProperties( options );
    std::map<std::string, std::string> &sys_property_map = sys_properties_rsp.propertyMap;

    // Check if multihead ingestion is enabled
    std::map<std::string, std::string>::iterator it;
    it = sys_property_map.find( gpudb::show_system_properties_conf_enable_worker_http_servers );
    if ( it == sys_property_map.end() )
    { // keyword not found
        throw GPUdbException( "Missing value for " + gpudb::show_system_properties_conf_enable_worker_http_servers );
    }

    // so, is multi-head ingestion enabled?
    bool multi_head_ingest_enabled = ( it->second == gpudb::show_system_properties_TRUE );
    if ( !multi_head_ingest_enabled )
    {   // nope, it is NOT enabled
        return; // nothing more to do
    }

    // Multi-head ingestion IS enabled; find the worker URLs;
    // then add them to the worker url vector.
    // ------------------------------------------------------
    std::string url_keyword = "conf.worker_http_server_urls";
    it = sys_property_map.find( url_keyword );
    if ( it != sys_property_map.end() )
    { // keyword was found
        // Parse the URLs
        char delim = ';';
        std::vector<std::string> worker_url_lists;
        split_string( it->second, delim, worker_url_lists );

        // For each worker rank--except the very first (rank0)--look for matching
        // URLs
        std::vector<std::string> worker_urls;  // will be cleared in split-string calls
        char url_delim = ',';
        for ( size_t i = 1; i < worker_url_lists.size(); ++i )
        {
            // Each of the URL entries is actually a list delimited by a comma
            split_string( worker_url_lists[ i ], url_delim, worker_urls );

            bool matching_url_found = false;
            
            // Go through all URLs and see if any will work
            std::vector<std::string>::const_iterator it2;
            for( it2 = worker_urls.begin(); it2 != worker_urls.end(); ++it2 )
            {
                // See if this IP is a match for the given REGEX
                matching_url_found = std::regex_match( *it2, ip_regex );

                if ( matching_url_found ) // add the matching URL
                {
                    gpudb::HttpUrl worker_url( *it2 );
                    m_worker_urls.push_back( worker_url );
                    break; // no need to look at the remaining URLs in this group
                }
            }  // end inner loop

            if ( !matching_url_found ) // none found; that's a problem
            {
                std::ostringstream ss;
                ss << "No matching URL found for worker #" << i << " for regex '" << ip_regex_str << "'";
                throw GPUdbException( ss.str() );
            }
        } // end outer loop
    }
    else  // URLs not found; try to construct from IP addresses and ports
    {

        // Get the IP addresses of the worker ranks
        std::string ip_keyword = gpudb::show_system_properties_conf_worker_http_server_ips;
        it = sys_property_map.find( ip_keyword );
        if ( it == sys_property_map.end() )
        { // keyword not found
            throw GPUdbException( "Missing value for " + ip_keyword );
        }

        // Parse the IP addresses
        char delim = ';';
        std::vector<std::string> worker_ip_lists;
        split_string( it->second, delim, worker_ip_lists );

        // Check that the there's at least one IP address given
        if ( worker_ip_lists.empty() )
            throw GPUdbException( "Empty value for " + ip_keyword );

        // Get the port numbers of the worker ranks
        std::string port_keyword = gpudb::show_system_properties_conf_worker_http_server_ports;
        it = sys_property_map.find( port_keyword );
        if ( it == sys_property_map.end() )
        { // keyword not found
            throw GPUdbException( "Missing value for " + port_keyword );
        }

        // Parse the ports
        std::vector<std::string> worker_ports;
        split_string( it->second, delim, worker_ports );

        // Check that the same number of IP addresses and ports are provided
        if ( worker_ip_lists.size() != worker_ports.size() )
            throw GPUdbException( "Inconsistent number of values for " + ip_keyword + " and " + port_keyword );

        // Get the protocol for the URLs to be created
        const std::string& protocol = gpudb.getUrl().getProtocol();
        
        // For each worker rank--except the very first (rank0)--create a URL from
        // the IP address and port number
        std::vector<std::string> worker_ips;  // will be cleared in split-string calls
        char ip_delim = ',';
        for ( size_t i = 1; i < worker_ip_lists.size(); ++i )
        {
            // Each of the IP entries is actually a list delimited by a comma
            split_string( worker_ip_lists[ i ], ip_delim, worker_ips );

            bool matching_ip_found = false;

            // Go through all IPs and see if any will work
            std::vector<std::string>::const_iterator it;
            for( it = worker_ips.begin(); it != worker_ips.end(); ++it )
            {
                // See if this IP is a match for the given REGEX
                matching_ip_found = std::regex_match( *it, ip_regex );

                // Create a URI for the matching IP address
                if ( matching_ip_found )
                {
                    std::string worker_url_str = (protocol + "://" + *it + ":" + worker_ports[ i ]);
                    gpudb::HttpUrl worker_url( worker_url_str );
                    m_worker_urls.push_back( worker_url );
                    break; // no need to look at the remaining IPs in this group
                }
            }  // end inner loop

            if ( !matching_ip_found ) // none found; that's a problem
            {
                std::ostringstream ss;
                ss << "No matching IP found for worker #" << i << " for regex '" << ip_regex_str << "'";
                throw GPUdbException( ss.str() );
            }
        } // end outer loop
    }  // end if-else

    if ( m_worker_urls.empty() ) // O_o No (matching) worker found!
        throw GPUdbException( "No worker HTTP servers found." );

    return;
}  // end WorkerList constructor


///*
// * Desctructor
// */
//WorkerList~WorkerList()
//{}


/*
 *  Returns a string representation of the workers contained within
 */
std::string WorkerList::toString() const
{
    std::ostringstream ss;
    size_t i = 0;
    for ( const_iterator it = m_worker_urls.begin();
          it != m_worker_urls.end(); ++it )
    {
        ss << *it << "; ";
        if (i > 0) // Use a separator as needed
            ss << "; ";
        ++i;
    }
    return ss.str();
}

/*
 * static convenience function
 * Any pre-existing entries in elements will be cleared.
 */
void WorkerList::split_string( const std::string &in_string,
                               char delim,
                               std::vector<std::string> &elements )
{
    elements.clear();

    std::stringstream ss( in_string );
    std::string item;
    while ( std::getline( ss, item, delim ) )
    {
        elements.push_back( item );
    }
    return;
}  // end split_string


// --------------------- END implementation of Class WorkerList ----------------------






// --------------------- Begin Various Convenient Functions for RecordKey ----------------

namespace timestamp
{
#define TIMESTAMP_BASE_YEAR 1900

#define YEARS_PER_QUAD_YEAR 4
#define DAYS_PER_YEAR       365   // not leap year

#define DAYS_PER_QUAD_YEAR  (YEARS_PER_QUAD_YEAR * DAYS_PER_YEAR+1)
#define DAYS_PER_WEEK       7
#define HOURS_PER_DAY       24
#define MINUTES_PER_HOUR    60
#define SECS_PER_MINUTE     60
#define MSECS_PER_SEC       1000L
#define MSECS_PER_MINUTE    (MSECS_PER_SEC * SECS_PER_MINUTE)
#define MSECS_PER_HOUR      (MSECS_PER_MINUTE * MINUTES_PER_HOUR)
#define MSECS_PER_DAY       (MSECS_PER_HOUR * HOURS_PER_DAY)
#define MSECS_PER_YEAR      (DAYS_PER_YEAR * MSECS_PER_DAY)
#define MSECS_PER_QUAD_YEAR (MSECS_PER_DAY * DAYS_PER_QUAD_YEAR)

#define JAN_1_0001_DAY_OF_WEEK 1  // 0 based day of week - is a friday (as if gregorian calandar started in year 1)

// Need the MS_OFFSET  to avoid division rounding towards 0.  Only values
// after 1901 are valid.  1901 was chosen because it is the year after
// a leap year.  this makes last yeqr of quad year from 1901 a leap
// year which simplifies calculations (see (*) comments below)..  Not
// going before 1900 avoids the complication that 1900 isn't a leap
// year.
#define MS_OFFSET ((68/YEARS_PER_QUAD_YEAR)*MSECS_PER_QUAD_YEAR + MSECS_PER_YEAR)

const int YEARS_PER_CENTURY = 100;
const int CENTURIES_PER_QUAD_CENTURY = 4;
const int EPOCH_YEAR = 1970;

const int LEAP_DAYS_PER_QUAD_YEAR = 1;
const int LEAP_DAYS_PER_CENTURY = ((YEARS_PER_CENTURY/YEARS_PER_QUAD_YEAR) - 1);
const int LEAP_DAYS_PER_QUAD_CENTURY = CENTURIES_PER_QUAD_CENTURY*((YEARS_PER_CENTURY/YEARS_PER_QUAD_YEAR) - 1) + 1;
const int DAYS_PER_CENTURY = (YEARS_PER_CENTURY*DAYS_PER_YEAR + LEAP_DAYS_PER_CENTURY);
const int DAYS_PER_QUAD_CENTURY = (CENTURIES_PER_QUAD_CENTURY*DAYS_PER_CENTURY + 1);
const int64_t MSECS_PER_CENTURY = (DAYS_PER_CENTURY*MSECS_PER_DAY);
const int64_t MSECS_PER_QUAD_CENTURY = (DAYS_PER_QUAD_CENTURY*MSECS_PER_DAY);

const int YEARS_TO_EPOCH = (EPOCH_YEAR-1); // from year 1
const int YEARS_PER_QUAD_CENTURY  = (YEARS_PER_CENTURY*CENTURIES_PER_QUAD_CENTURY);

const int QUAD_CENTURIES_OFFSET          = (YEARS_TO_EPOCH/YEARS_PER_QUAD_CENTURY);
const int YEAR_IN_QUAD_CENTURY_OFFSET    = (YEARS_TO_EPOCH%YEARS_PER_QUAD_CENTURY);
const int CENTURY_OF_QUAD_CENTURY_OFFSET = (YEAR_IN_QUAD_CENTURY_OFFSET/YEARS_PER_CENTURY);
const int YEAR_IN_CENTURY_OFFSET         = (YEAR_IN_QUAD_CENTURY_OFFSET%YEARS_PER_CENTURY);
const int QUAD_YEAR_OF_CENTURY_OFFSET    = (YEAR_IN_CENTURY_OFFSET/YEARS_PER_QUAD_YEAR);
const int YEAR_IN_QUAD_YEAR_OFFSET       = (YEAR_IN_CENTURY_OFFSET%YEARS_PER_QUAD_YEAR);

const int64_t MS_EPOCH_OFFSET = (QUAD_CENTURIES_OFFSET*MSECS_PER_QUAD_CENTURY + CENTURY_OF_QUAD_CENTURY_OFFSET*MSECS_PER_CENTURY + \
                                 QUAD_YEAR_OF_CENTURY_OFFSET*MSECS_PER_QUAD_YEAR + YEAR_IN_QUAD_YEAR_OFFSET*MSECS_PER_YEAR);

inline bool isa_leap_year(int year)
{
    return (
        (year % YEARS_PER_QUAD_CENTURY == 0) ? true :
        (year % YEARS_PER_CENTURY == 0)      ? false :
        (year % YEARS_PER_QUAD_YEAR == 0)    ? true :
                                               false );
}  // end isa_leap_year


inline int get_month_and_day_from_day_of_year(int &dy, bool ly)
{
    if (dy <= 31)
        return 1;
    dy -= 31;
    if (dy <= 28 + ly)
        return 2;
    dy -= 28 + ly;
    if (dy <= 31)
        return 3;
    dy -= 31;
    if (dy <= 30)
        return 4;
    dy -= 30;
    if (dy <= 31)
        return 5;
    dy -= 31;
    if (dy <= 30)
        return 6;
    dy -= 30;
    if (dy <= 31)
        return 7;
    dy -= 31;
    if (dy <= 31)
        return 8;
    dy -= 31;
    if (dy <= 30)
        return 9;
    dy -= 30;
    if (dy <= 31)
        return 10;
    dy -= 31;
    if (dy <= 30)
        return 11;
    dy -= 30;
    return 12;      // december
}  // end get_month_and_day_from_day_of_year


inline int hour_from_ms(int64_t ms)
{
    return (int)(((ms + MS_EPOCH_OFFSET)/MSECS_PER_HOUR) % HOURS_PER_DAY);
}

inline int minute_from_ms(int64_t ms)
{
    return (int)(((ms + MS_EPOCH_OFFSET)/MSECS_PER_MINUTE) % MINUTES_PER_HOUR);
}

inline int sec_from_ms(int64_t ms)
{
    return (int)(((ms + MS_EPOCH_OFFSET)/MSECS_PER_SEC) % SECS_PER_MINUTE);
}

inline int msec_from_ms(int64_t ms)
{
    return (int)((ms + MS_EPOCH_OFFSET) % MSECS_PER_SEC);
}

inline int days_since_0001_from_ms(int64_t ms)
{
    return (ms + MS_EPOCH_OFFSET)/ MSECS_PER_DAY;
}

inline int day_of_week_from_days_since_0001(int days_since_1)
{
    return ((days_since_1 + JAN_1_0001_DAY_OF_WEEK) % DAYS_PER_WEEK) + 1;
}

inline int day_of_week_from_ms(int64_t ms)
{
    return day_of_week_from_days_since_0001(days_since_0001_from_ms(ms));
}



inline int days_in_month(int year, int month)
{
    bool ly = (year % 400 == 0) ? true : (year % 100 == 0) ? false : (year % 4 == 0) ? true : false;
    //                             J,  F,  M,  A,  M,  J,  J,  A,  S,  O,  N,  D
    const int days_per_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int leap_day = (ly && (month == 2)) ? 1 : 0;
    return ( days_per_month[month-1] + leap_day );
}

#define BITS_PER_YEAR 11     // 1900 based (field == 0 is year 1900 )
#define BITS_PER_MONTH 4    // 1 based
#define BITS_PER_DAY 5      // 1 based
#define BITS_PER_HOUR 5     // 0 based
#define BITS_PER_MINUTE 6   // 0 based
#define BITS_PER_SEC 6      // 0 based
#define BITS_PER_MSEC 10    // 0 based
#define BITS_PER_YDAY 9     // 1 based
#define BITS_PER_WDAY 3     // 1 based

#define SHIFT_YEAR   (64-BITS_PER_YEAR)               // 53
#define SHIFT_MONTH  (SHIFT_YEAR   - BITS_PER_MONTH)  // 49
#define SHIFT_DAY    (SHIFT_MONTH  - BITS_PER_DAY)    // 44
#define SHIFT_HOUR   (SHIFT_DAY    - BITS_PER_HOUR)   // 39
#define SHIFT_MINUTE (SHIFT_HOUR   - BITS_PER_MINUTE) // 33
#define SHIFT_SEC    (SHIFT_MINUTE - BITS_PER_SEC)    // 27
#define SHIFT_MSEC   (SHIFT_SEC    - BITS_PER_MSEC)   // 17
#define SHIFT_YDAY   (SHIFT_MSEC   - BITS_PER_YDAY)   // 8
#define SHIFT_WDAY   (SHIFT_YDAY   - BITS_PER_WDAY)   // 5

inline int64_t create_timestamp_from_fields( int year_field,
                                             int month_of_year_field,
                                             int day_of_month_field,
                                             int hour_field,
                                             int minute_field,
                                             int sec_field,
                                             int msec_field,
                                             int day_of_year_field,
                                             int day_of_week_field )
{
    return
        (((int64_t)year_field)          << SHIFT_YEAR) +
        (((int64_t)month_of_year_field) << SHIFT_MONTH) +
        (((int64_t)day_of_month_field)  << SHIFT_DAY) +
        (((int64_t)hour_field)          << SHIFT_HOUR) +
        (((int64_t)minute_field)        << SHIFT_MINUTE) +
        (((int64_t)sec_field)           << SHIFT_SEC) +
        (((int64_t)msec_field)          << SHIFT_MSEC) +
        (((int64_t)day_of_year_field)   << SHIFT_YDAY) +
        (((int64_t)day_of_week_field)   << SHIFT_WDAY);
}  // end create_timestamp_from_fields


inline int64_t create_timestamp_from_epoch_ms(int64_t ms)
{
    int days_since_1 = (ms+MS_EPOCH_OFFSET)/MSECS_PER_DAY;
    int quad_century = days_since_1/DAYS_PER_QUAD_CENTURY;
    int day_of_quad_century = days_since_1 - quad_century*DAYS_PER_QUAD_CENTURY;
    int century_of_quad_century = day_of_quad_century/DAYS_PER_CENTURY;
    if (century_of_quad_century == 4) century_of_quad_century = 3;
    int day_of_century = day_of_quad_century - century_of_quad_century*DAYS_PER_CENTURY;
    int quad_year_of_century = day_of_century/DAYS_PER_QUAD_YEAR;
    int day_of_quad_year = day_of_century - quad_year_of_century * DAYS_PER_QUAD_YEAR;
    int year_of_quad_year = day_of_quad_year/DAYS_PER_YEAR;
    if (year_of_quad_year == 4) year_of_quad_year = 3;
    int day_of_year_field = day_of_quad_year - year_of_quad_year*DAYS_PER_YEAR + 1;

    int year = YEARS_PER_QUAD_CENTURY*quad_century + YEARS_PER_CENTURY*century_of_quad_century + YEARS_PER_QUAD_YEAR*quad_year_of_century + year_of_quad_year + 1;
    int year_field = year - TIMESTAMP_BASE_YEAR;

    bool ly = isa_leap_year(year);

    int day_of_month_field = day_of_year_field; // gets modified by next function call
    int month_of_year_field = get_month_and_day_from_day_of_year(day_of_month_field,ly);

    int hour_field   = hour_from_ms(ms);
    int minute_field = minute_from_ms(ms);
    int sec_field    = sec_from_ms(ms);
    int msec_field   = msec_from_ms(ms);
    int day_of_week_field = day_of_week_from_ms(ms);

    return create_timestamp_from_fields(year_field,
                                        month_of_year_field,
                                        day_of_month_field,
                                        hour_field,
                                        minute_field,
                                        sec_field,
                                        msec_field,
                                        day_of_year_field,
                                        day_of_week_field);
}  // end create_timestamp_from_epoch_ms


inline int get_day_of_year_from_month_and_day(int month, int day, int leap_year)
{
    const int days_before_month[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    return ( days_before_month[month-1]
             + day
             + ( (month > 2) ? leap_year : 0) );  // # of days after the given day
}

inline int day_of_year_from_date(int year, int month, int day)
{
    bool ly = (year % 400 == 0) ? true : (year % 100 == 0) ? false : (year % 4 == 0) ? true : false;
    return get_day_of_year_from_month_and_day(month, day, ly);
}


inline int days_since_epoch_from_year_and_day_of_year(int year, int day_of_year)
{
    // get days between 1 and y
    int y   = year-1;  // years since base year of 1
    int qc  = y / YEARS_PER_QUAD_CENTURY;
    int yqc = y - qc * YEARS_PER_QUAD_CENTURY;
    int c   = yqc/ YEARS_PER_CENTURY;
    int yc  = yqc - c * YEARS_PER_CENTURY;
    int q   = yc / YEARS_PER_QUAD_YEAR;
    int yq  = yc - q * YEARS_PER_QUAD_YEAR;
    int d   = day_of_year-1;
    int days = (DAYS_PER_QUAD_CENTURY * qc) + (DAYS_PER_CENTURY * c) + (DAYS_PER_QUAD_YEAR * q) + DAYS_PER_YEAR*yq + d;

    // get days between 1 and 1970
    const int y1 = 1970-1;
    const int qc1  = y1 / YEARS_PER_QUAD_CENTURY;
    const int yqc1 = y1 - qc1 * YEARS_PER_QUAD_CENTURY;
    const int c1   = yqc1/ YEARS_PER_CENTURY;
    const int yc1  = yqc1 - c1 * YEARS_PER_CENTURY;
    const int q1   = yc1 / YEARS_PER_QUAD_YEAR;
    const int yq1  = yc1 - q1 * YEARS_PER_QUAD_YEAR;
    const int days1 = (DAYS_PER_QUAD_CENTURY * qc1) + (DAYS_PER_CENTURY * c1) + (DAYS_PER_QUAD_YEAR * q1) + DAYS_PER_YEAR*yq1;

    // return the difference
    return days - days1;
}


inline void get_day_of_year_and_week( int year, int month, int day_of_month, int& day_of_year, int& day_of_week )
{
    day_of_year = day_of_year_from_date( year, month, day_of_month );

    int day_of_epoch = days_since_epoch_from_year_and_day_of_year(year, day_of_year ); // 0 based

    // 1 based. add large multiple of 7 to days_of_epoch to make it positive
    // and add 4 since Jan 1 1970 was a Thursday.
    day_of_week = ( ( (day_of_epoch + 52*1000*7 + 4) % 7 ) + 1 );
}


#define DATE_YEAR_BASE 1900

#define DATE_BITS_PER_YEAR  11     // 1900 based (field == 0 is year 1000), range: 876-2923
#define DATE_BITS_PER_MONTH  4     // 1 based
#define DATE_BITS_PER_DAY    5     // 1 based
#define DATE_BITS_PER_YDAY   9     // 1 based
#define DATE_BITS_PER_WDAY   3     // 1 based

#define DATE_SHIFT_YEAR   ( 32                - DATE_BITS_PER_YEAR  )  // 21
#define DATE_SHIFT_MONTH  ( DATE_SHIFT_YEAR   - DATE_BITS_PER_MONTH )  // 17
#define DATE_SHIFT_DAY    ( DATE_SHIFT_MONTH  - DATE_BITS_PER_DAY   )  // 12
#define DATE_SHIFT_YDAY   ( DATE_SHIFT_DAY    - DATE_BITS_PER_YDAY  )  // 3
#define DATE_SHIFT_WDAY   ( DATE_SHIFT_YDAY   - DATE_BITS_PER_WDAY  )  // 0

inline int32_t create_date_from_fields( int year, int month, int day_of_month,
                                        int day_of_year, int day_of_week )
{
    return ( ( year          << DATE_SHIFT_YEAR  ) +
             ( month         << DATE_SHIFT_MONTH ) +
             ( day_of_month  << DATE_SHIFT_DAY   ) +
             ( day_of_year   << DATE_SHIFT_YDAY  ) +
             ( day_of_week   << DATE_SHIFT_WDAY  ) );
}


inline bool process_date(int year, int month, int day_of_month, int32_t &date )
{
    // Check that it is within our range
    if ( (year < 1000) || (year > 2900) )
        return false;

    // Check that the month is valid
    if ( (month < 1) || (month > 12) )
        return false;

    // Check that the day is valid for the given month
    int max_day = days_in_month(year, month);
    if ( (day_of_month < 1) || (day_of_month > max_day) )
        return false;

    // Get the day of year and the day of week
    int day_of_year;
    int day_of_week;
    get_day_of_year_and_week( year, month, day_of_month, day_of_year, day_of_week );

    // Finally, encode the various fields into one integer
    date = create_date_from_fields( year - DATE_YEAR_BASE, month, day_of_month,
                                     day_of_year, day_of_week);
    return true;
}  // end process_date



inline bool process_datetime( int year, int month, int day_of_month,
                              int hour, int minute, int second, int msec,
                              int64_t &datetime )
{
    // Check that it is within our range
    if ( (year < 1000) || (year > 2900) )
        return false;

    // Check that the month is valid
    if ( (month < 1) || (month > 12) )
        return false;

    // Check that the day is valid for the given month
    int max_day = days_in_month(year, month);
    if ( (day_of_month < 1) || (day_of_month > max_day) )
        return false;

    // Check that the hour is valid
    if ( (hour < 0) || (hour > 23) )
        return false;

    // Check that the minute is valid
    if ( (minute < 0) || (minute > 59) )
        return false;

    // Check that the second is valid
    if ( (second < 0) || (second > 59) )
        return false;

    // Check that the millisecond is valid
    if ( (msec < 0) || (msec > 999) )
        return false;

    // Get the day of year and the day of week
    int day_of_year;
    int day_of_week;
    get_day_of_year_and_week( year, month, day_of_month, day_of_year, day_of_week );

    // Finally, encode the various fields into one integer
    datetime = create_timestamp_from_fields( year - DATE_YEAR_BASE, month, day_of_month,
                                             hour, minute, second, msec,
                                             day_of_year, day_of_week );
    return true;
}  // end process_datetime



#define TIME_SHIFT_HOUR   26
#define TIME_SHIFT_MINUTE 20
#define TIME_SHIFT_SEC    14
#define TIME_SHIFT_MSEC    4

inline bool process_time(int hour, int minute, int second, int millisecond, int32_t &time )
{
    // Check that the hour is valid
    if ( (hour < 0) || (hour > 23) )
        return false;

    // Check that the minute is valid
    if ( (minute < 0) || (minute > 59) )
        return false;

    // Check that the second is valid
    if ( (second < 0) || (second > 59) )
        return false;

    // Check that the millisecond is valid
    if ( (millisecond < 0) || (millisecond > 999) )
        return false;

    // Finally, encode the various fields into one integer
    time = ( ( ((uint32_t) hour)        << TIME_SHIFT_HOUR   ) +
             ( ((uint32_t) minute)      << TIME_SHIFT_MINUTE ) +
             ( ((uint32_t) second)      << TIME_SHIFT_SEC    ) +
             ( ((uint32_t) millisecond) << TIME_SHIFT_MSEC   ) );

    return true;
}  // end process_time


} // end namespace timestamp



namespace charN
{
// --------------------------------------------------------------------------
/// A big-endian 0-32 byte char* string stored as a 4 64-bit ints.
/// All inlined since these may be used in loops.
// --------------------------------------------------------------------------
template<int N>     // N is the number of bytes (32,64,128,256)
struct fixed_array_buf_t
{
    /// Convert from a little-endian string to a big-endian intXXX
    inline fixed_array_buf_t(const std::string& s, bool reverse=false)
    {
        init(s.data(), s.size(), reverse);
    }

    inline void init(const char* s, size_t len, bool reverse=false)
    {
        uint64_t v[N] = {};

        // Initialize to zero
        for (int i = 0; i < N; ++i )
            v[ i ] = 0;

        const int num_longs = (N/8);
        size_t offset = 0;
        for (int i = 0; i < num_longs; ++i )
        {
            if (len > offset)
            {
                memcpy(&v[i], (s + offset), std::min( size_t(8),
                                                      (len - offset) ) );
            }
            offset += 8;
        }

        if (reverse)
        {
            for (int i = 0; i < num_longs; ++i )
                val[i] = htobe64( v[i] );
        }
        else
        {
            for (int i = 0; i < num_longs; ++i)
                val[i] = htobe64( v[ num_longs-1-i ] );
        }
    }

    union
    {
        uint64_t val[ N/8 ];
        char     buf[ N ];
    };
};  // end struct fixed_array_buf_t


// Templated on the number of bytes
typedef fixed_array_buf_t<32>  Int256_buf;
typedef fixed_array_buf_t<64>  Int512_buf;
typedef fixed_array_buf_t<128> Int1024_buf;
typedef fixed_array_buf_t<256> Int2048_buf;


// Rename for convenience
typedef Int256_buf   char32_buf_t;
typedef Int512_buf   char64_buf_t;
typedef Int1024_buf  char128_buf_t;
typedef Int2048_buf  char256_buf_t;


}  // end namespace charN

// --------------------- END Various Convenient Functions for RecordKey ----------------------





// --------------------- Begin Class RecordKey ----------------------

enum ColumnTypeSize_T
{
    BOOLEAN   =   1,
    CHAR1     =   1,
    CHAR2     =   2,
    CHAR4     =   4,
    CHAR8     =   8,
    CHAR16    =  16,
    CHAR32    =  32,
    CHAR64    =  64,
    CHAR128   = 128,
    CHAR256   = 256,
    DATE      =   4,
    DATETIME  =   8,
    DECIMAL   =   8,
    DOUBLE    =   8,
    FLOAT     =   4,
    INT       =   4,
    INT8      =   1,
    INT16     =   2,
    IPV4      =   4,
    LONG      =   8,
    STRING    =   8,
    TIME      =   4,
    TIMESTAMP =   8,
    ULONG     =   8,
    UUID      =  16,
};

#define GPUDB_HASH_SEED 10


// Default constructor
RecordKey::RecordKey()
{
    m_buffer_size  = 0;
    m_current_size = 0;
    m_hash_code    = 0;
    m_routing_hash = 0;

    m_is_valid        = false;
    m_key_is_complete = false;
}


// Constructor
RecordKey::RecordKey( size_t buffer_size )
{
    if ( buffer_size < 1 )
    {
        std::ostringstream ss;
        ss << "Buffer size must be greater than or equal to 1; given: '"
           << buffer_size << "'";
        throw GPUdbException( ss.str() );
    }

    // Allocate the buffer and initialize
    m_buffer.reserve( buffer_size );

    m_buffer_size  = buffer_size;
    m_current_size = 0;
    m_hash_code    = 0;
    m_routing_hash = 0;

    m_is_valid        = true;
    m_key_is_complete = false;
}  // end RecordKey constructor


// Copy constructor
RecordKey::RecordKey( const RecordKey &other )
{
    this->copy( other );
}  // end RecordKey copy constructor


// Assignment operator
RecordKey& RecordKey::operator=(const RecordKey& other)
{
    this->copy( other );
    return *this;
}  // end assignment operator


// Copy contents of another key into this one
void RecordKey::copy(const RecordKey& other)
{
    // Protect against invalid self-assignment
    if ( this != &other )
    {
        // Set this key with the other key's buffer size
        this->reset( other.m_buffer_size );

        // Copy the content of the buffer (already allocated by
        // reset()
        m_buffer.insert( m_buffer.begin(),
                         other.m_buffer.begin(), other.m_buffer.end() );

        // Copy over the other variables
        m_current_size    = other.m_current_size;
        m_hash_code       = other.m_hash_code;
        m_routing_hash    = other.m_routing_hash;
        m_is_valid        = other.m_is_valid;
        m_key_is_complete = other.m_key_is_complete;
    }
}  // end copy()


// Destructor
RecordKey::~RecordKey() {}


/*
 * Converts the internal byte buffer to string for printing purposes.
 */
std::string RecordKey::toString( const std::string& separator ) const
{
    std::ostringstream ss;
    for (size_t n = 0; n < m_buffer_size; ++n)
    {
        if (n > 0) ss << separator;
        ss << m_buffer[ n ];
    }

    return ss.str();
}  // end toString


/*
 *  Resets the key to be an empty one with the new buffer size
 *
 * <param name="buffer_size">The new size for the key buffer</param>
 */
void RecordKey::reset( size_t buffer_size )
{
    if ( buffer_size < 1 )
    {
        std::ostringstream ss;
        ss << "Buffer size must be greater than or equal to 1; given: '"
           << buffer_size << "'";
        throw GPUdbException( ss.str() );
    }

    m_buffer_size  = buffer_size;
    m_current_size = 0;
    m_hash_code    = 0;
    m_routing_hash = 0;

    m_is_valid        = true;
    m_key_is_complete = false;

    // Delete old buffer content, and allocate enough for the new key
    m_buffer.clear();
    m_buffer.resize( m_buffer_size );
}  // end reset



bool RecordKey::is_buffer_full( bool throw_if_full ) const
{
    if ( m_current_size >= m_buffer_size ) // should never be greater, but staying on the safe side
    {
        if ( throw_if_full )
            throw GPUdbException( "The buffer is already full!" );
        return true; // yes, the buffer is full, and we haven't thrown
    }

    return false; // buffer is NOT full
}  // end is_buffer_full


bool RecordKey::will_buffer_overflow( int n, bool throw_if_overflow ) const
{
    if ( (m_current_size + n) > m_buffer_size )
    {
        if ( throw_if_overflow )
        {
            std::ostringstream ss;
            ss << "The buffer (of size " << m_buffer_size << ") does not "
               << "have sufficient room in it to put " << n << " more "
               << "byte(s) (current size is " << m_current_size << ")";
            throw GPUdbException( ss.str() );
        }
        return true; // yes, the buffer WILL overflow, and we haven't thrown
    }
    return false; // buffer will NOT overflow
} // end will_buffer_overflow


/*
 * Adds a byte to the byte buffer and increments the current size
 * by one.  Use ONLY this method to add to the buffer; do not add
 * to buffer in other methods directly.  Note that this function does
 * not check if the buffer will overflow or is already full.  The caller
 * must take care of that.
 *
 * <param name="b">The byte to be added to the buffer.</param>
 */
void RecordKey::add( uint8_t b )
{
    // Let's not forget to increment the size!
    m_buffer[ m_current_size++ ] = b;
}


/*
 * Adds a boolean to the buffer.
 *
 * <param name="value">The boolean to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_boolean( bool value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::BOOLEAN );

    // Handle nulls
    if ( is_null )
    {   // Add one zero for the null value
        this->add( 0 ); // only 0
        return;
    }

    // Copy the bytes of the bool to the buffer
    memset( &m_buffer[0] + m_current_size, value ? 1 : 0, ColumnTypeSize_T::BOOLEAN );
    m_current_size += ColumnTypeSize_T::BOOLEAN;
    return;
} // end RecordKey::add_bool


/*
 * Adds a char1 to the buffer.
 *
 * <param name="value">The char1 to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_char1( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::CHAR1 );

    // Handle nulls
    if ( is_null )
    {   // Add one zero for the null value
        this->add( 0 ); // the only 0
        return;
    }

    // Process the data
    uint8_t char1_val = 0;
    memcpy(&char1_val, &value[0], std::min( (size_t)ColumnTypeSize_T::CHAR1, value.size() ) );

    // Copy the bytes of the char1 to the buffer
    memcpy( &m_buffer[0] + m_current_size, &char1_val, ColumnTypeSize_T::CHAR1 );
    m_current_size += ColumnTypeSize_T::CHAR1;
} // end RecordKey::add_char1



/*
 * Adds a char2 to the buffer.
 *
 * <param name="value">The char2 to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_char2( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::CHAR2 );

    // Handle nulls
    if ( is_null )
    {   // Add two zeroes for the null value
        this->add( 0 ); // 1st 0
        this->add( 0 ); // 2nd 0
        return;
    }

    // Process the data
    uint16_t char2_val_tmp = 0;
    uint16_t char2_val     = 0;
    memcpy(&char2_val_tmp, &value[0], std::min( (size_t)ColumnTypeSize_T::CHAR2, value.size() ) );
    char2_val = htobe16( char2_val_tmp );

    // Copy the bytes of the char2 to the buffer
    memcpy( &m_buffer[0] + m_current_size, &char2_val, ColumnTypeSize_T::CHAR2 );
    m_current_size += ColumnTypeSize_T::CHAR2;

} // end RecordKey::add_char2


/*
 * Adds a char4 to the buffer.
 *
 * <param name="value">The char4 to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_char4( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::CHAR4 );

    // Handle nulls
    if ( is_null )
    {   // Add four zeroes for the null value
        this->add( 0 ); // 1st 0
        this->add( 0 ); // 2nd 0
        this->add( 0 ); // 3rd 0
        this->add( 0 ); // 4th 0
        return;
    }

    // Process the data
    uint32_t char4_val_tmp = 0;
    uint32_t char4_val     = 0;
    memcpy(&char4_val_tmp, &value[0], std::min( (size_t)ColumnTypeSize_T::CHAR4, value.size() ) );
    char4_val = htobe32( char4_val_tmp );

    // Copy the bytes of the char4 to the buffer
    memcpy( &m_buffer[0] + m_current_size, &char4_val, ColumnTypeSize_T::CHAR4 );
    m_current_size += ColumnTypeSize_T::CHAR4;

} // end RecordKey::add_char4




/*
 * Adds a char8 to the buffer.
 *
 * <param name="value">The char8 to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_char8( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::CHAR8 );

    // Handle nulls
    if ( is_null )
    {   // Add eight zeroes for the null value
        this->add( 0 ); // 1st 0
        this->add( 0 ); // 2nd 0
        this->add( 0 ); // 3rd 0
        this->add( 0 ); // 4th 0
        this->add( 0 ); // 5th 0
        this->add( 0 ); // 6th 0
        this->add( 0 ); // 7th 0
        this->add( 0 ); // 8th 0
        return;
    }

    // Process the data
    uint64_t char8_val_tmp = 0;
    uint64_t char8_val     = 0;
    memcpy(&char8_val_tmp, &value[0], std::min( (size_t)ColumnTypeSize_T::CHAR8, value.size() ) );
    char8_val = htobe64( char8_val_tmp );

    // Copy the bytes of the char8 to the buffer
    memcpy( &m_buffer[0] + m_current_size, &char8_val, ColumnTypeSize_T::CHAR8 );
    m_current_size += ColumnTypeSize_T::CHAR8;

} // end RecordKey::add_char8



/*
 * Adds a char16 to the buffer.
 *
 * <param name="value">The char16 to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_char16( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::CHAR16 );

    // Handle nulls
    if ( is_null )
    {   // Add 16 zeroes for the null value
        for ( size_t i = 0; i < ColumnTypeSize_T::CHAR16; ++i )
        {
            this->add( 0 );
        }
        return;
    }

    // Process the data
    size_t half_size = (size_t)ColumnTypeSize_T::CHAR8;
    uint64_t char16_val_tmp[2] = {0, 0};  // low and high values
    uint64_t char16_val[2] = {0, 0};
    // First the low value
    memcpy(&char16_val_tmp[0], &value[0], std::min( half_size, value.size() ) );
    // Then the high value, if needed
    if ( value.size() > half_size )
        memcpy(&char16_val_tmp[1], &value[half_size], std::min( half_size,
                                                        ( value.size() - half_size ) ) );
    // Flip the endianness
    char16_val[0] = htobe64( char16_val_tmp[1] );  // high -> low
    char16_val[1] = htobe64( char16_val_tmp[0] );  // low  -> high

    // Copy the bytes of the char16 to the buffer
    memcpy( &m_buffer[0] + m_current_size, &char16_val, ColumnTypeSize_T::CHAR16 );
    m_current_size += ColumnTypeSize_T::CHAR16;

} // end RecordKey::add_char16



/*
 * Adds a char32 to the buffer.
 *
 * <param name="value">The char32 to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_char32( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::CHAR32 );

    // Handle nulls
    if ( is_null )
    {   // Add 32 zeroes for the null value
        for ( size_t i = 0; i < ColumnTypeSize_T::CHAR32; ++i )
        {
            this->add( 0 );
        }
        return;
    }

    // Process the data
    charN::char32_buf_t char32_val( value );

    // Copy the bytes of the char32 to the buffer (four bytes at a time)
    memcpy( &m_buffer[0] + m_current_size, &char32_val.buf, ColumnTypeSize_T::CHAR32 );
    m_current_size += ColumnTypeSize_T::CHAR32;

} // end RecordKey::add_char32



/*
 * Adds a char64 to the buffer.
 *
 * <param name="value">The char64 to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_char64( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::CHAR64 );

    // Handle nulls
    if ( is_null )
    {   // Add 64 zeroes for the null value
        for ( size_t i = 0; i < ColumnTypeSize_T::CHAR64; ++i )
        {
            this->add( 0 );
        }
        return;
    }

    // Process the data
    charN::char64_buf_t char64_val( value );

    // Copy the bytes of the char64 to the buffer (four bytes at a time)
    memcpy( &m_buffer[0] + m_current_size, &char64_val.buf, ColumnTypeSize_T::CHAR64 );
    m_current_size += ColumnTypeSize_T::CHAR64;

} // end RecordKey::add_char64



/*
 * Adds a char128 to the buffer.
 *
 * <param name="value">The char128 to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_char128( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::CHAR128 );

    // Handle nulls
    if ( is_null )
    {   // Add 128 zeroes for the null value
        for ( size_t i = 0; i < ColumnTypeSize_T::CHAR128; ++i )
        {
            this->add( 0 );
        }
        return;
    }

    // Process the data
    charN::char128_buf_t char128_val( value );

    // Copy the bytes of the char128 to the buffer (four bytes at a time)
    memcpy( &m_buffer[0] + m_current_size, &char128_val.buf, ColumnTypeSize_T::CHAR128 );
    m_current_size += ColumnTypeSize_T::CHAR128;

} // end RecordKey::add_char128



/*
 * Adds a char256 to the buffer.
 *
 * <param name="value">The char256 to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_char256( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::CHAR256 );

    // Handle nulls
    if ( is_null )
    {   // Add 256 zeroes for the null value
        for ( size_t i = 0; i < ColumnTypeSize_T::CHAR256; ++i )
        {
            this->add( 0 );
        }
        return;
    }

    // Process the data
    charN::char256_buf_t char256_val( value );

    // Copy the bytes of the char256 to the buffer (four bytes at a time)
    memcpy( &m_buffer[0] + m_current_size, &char256_val.buf, ColumnTypeSize_T::CHAR256 );
    m_current_size += ColumnTypeSize_T::CHAR256;

} // end RecordKey::add_char256



// Needed for parsing date
static boost::regex date_regex( "\\A\\s*(\\d{4})-(\\d{2})-(\\d{2})\\s*$" );

/*
 * Adds a string of the format "YYYY-MM-DD" to the buffer.  Internally, the
 * date is stored as an integer.
 *
 * <param name="value">The date to be added.  Must be of the format
 * "YYYY-MM-DD".</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_date( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::DATE );

    // Handle nulls
    if ( is_null )
    {   // Add a null int
        this->add_int( 0, is_null );
        return;
    }

    // Does the given value match our format of YYYY-MM-DD?
    boost::cmatch matches;
    if ( !boost::regex_match( value.c_str(), matches, date_regex ) )
    {  // No match, so the key is invalid
        this->add_int( 0, is_null );
        m_is_valid = false;
        return;
    }

    // Get the year, month and day out
    int year  = boost::lexical_cast<int>( std::string( matches[1].first, matches[1].second ) );
    int month = boost::lexical_cast<int>( std::string( matches[2].first, matches[2].second ) );
    int day   = boost::lexical_cast<int>( std::string( matches[3].first, matches[3].second ) );

    // Process the date
    int32_t encoded_date_integer = 0;
    bool encoding_success = timestamp::process_date( year, month, day, encoded_date_integer );
    if ( !encoding_success )
    {  // something went wrong in the encoding process
        this->add_int( 0, is_null );
        m_is_valid = false;
        return;
    }

    this->add_int( encoded_date_integer, is_null );

} // end RecordKey::add_date



// Needed for parsing datetime
// YYYY-MM-DD[ HH:MM:SS[.ffffff]]
// Note - allow up to 6 decimal digits (but we ignore the last 3)
static boost::regex datetime_regex( "\\A\\s*(\\d{4})-(\\d{2})-(\\d{2})(?:\\s+(\\d{1,2}):(\\d{2}):(\\d{2})(?:\\.(\\d{1,6}))?)?\\s*$" );

/*
 * Adds a string of the format "YYYY-MM-DD[ HH:MM:SS[.mmm]]" to the buffer.  Internally, the
 * date is stored as an long.  For the optional milliseconds, allow up to six
 * decimal digits, but we discard the last three.
 *
 * <param name="value">The datetime to be added.  Must be of the format
 * "YYYY-MM-DD[ HH:MM:SS[.mmm]]".</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_datetime( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::DATETIME );

    // Handle nulls
    if ( is_null )
    {   // Add a null int
        this->add_long( 0, is_null );
        return;
    }

    // Does the given value match our format of "YYYY-MM-DD[ HH:MM:SS[.mmm]]"?
    boost::cmatch matches;
    if ( !boost::regex_match( value.c_str(), matches, datetime_regex ) )
    {  // No match, so the key is invalid
        this->add_long( 0, is_null );
        m_is_valid = false;
        return;
    }

    // The regex groups for specific parts
    boost::csub_match year_group  = matches[ 1 ];
    boost::csub_match month_group = matches[ 2 ];
    boost::csub_match day_group   = matches[ 3 ];
    boost::csub_match hour_group  = matches[ 4 ];
    boost::csub_match min_group   = matches[ 5 ];
    boost::csub_match sec_group   = matches[ 6 ];
    boost::csub_match msec_group  = matches[ 7 ];

    // Get the year, month, day, hour, minute, and second
    int year, month, day;
    int hour, min, sec;
    hour = 0;
    min  = 0;
    sec  = 0;
    year  = boost::lexical_cast<int>( std::string( year_group.first,  year_group.second  ) );
    month = boost::lexical_cast<int>( std::string( month_group.first, month_group.second ) );
    day   = boost::lexical_cast<int>( std::string( day_group.first,   day_group.second   ) );

    if ( hour_group.matched )
    {
        hour  = boost::lexical_cast<int>( std::string( hour_group.first, hour_group.second ) );
        min   = boost::lexical_cast<int>( std::string( min_group.first,  min_group.second  ) );
        sec   = boost::lexical_cast<int>( std::string( sec_group.first,  sec_group.second  ) );
    }

    int msec = 0;

    // The millisecond component is optional
    if ( msec_group.matched )
    {
        msec = boost::lexical_cast<int>( std::string( msec_group.first, msec_group.second ) );

        // Handle single and double digits for milliseconds
        switch ( msec_group.length() )
        {
            case 1:
                msec *= 100; break;
            case 2:
                msec *= 10; break;
            // Skipping 3--nothing to do in that case
            case 4:
                msec /= 10; break;
            case 5:
                msec /= 100; break;
            case 6:
                msec /= 1000; break;
        }  // end switch
    }  // end if msec is given

    // Process the date
    int64_t encoded_datetime = 0;
    bool encoding_success = timestamp::process_datetime( year, month, day,
                                                         hour, min, sec, msec,
                                                         encoded_datetime );
    if ( !encoding_success )
    {  // something went wrong in the encoding process
        this->add_long( 0, is_null );
        m_is_valid = false;
        return;
    }

    this->add_long( encoded_datetime, is_null );
    return;
} // end RecordKey::add_datetime



// Needed for parsing decimal values
static boost::regex decimal_regex( "\\A\\s*(?<sign>[+-]?)((?<int>\\d+)(\\.(?<intfrac>\\d{0,4}))?|\\.(?<onlyfrac>\\d{1,4}))\\s*\\z" );

/*
 * Adds a decimal string to the buffer.  Internally, the
 * date is stored as a long.
 *
 * <param name="value">The decimal value to be added.  Must have
 * upto 19 digits of precision and four digits of scale format.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_decimal( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::DECIMAL );

    // Handle nulls
    if ( is_null )
    {   // Add a null long
        this->add_long( 0L, is_null );
        return;
    }

    // Does the given value match decimal format?
    boost::cmatch matches;
    if ( !boost::regex_match( value.c_str(), matches, decimal_regex ) )
    {  // No match, so the key is invalid
        this->add_long( 0L, is_null );
        m_is_valid = false;
        return;
    }

    // Get the regex match groups
    boost::csub_match sign_group               = matches[ 1 ];
    boost::csub_match integral_group           = matches[ 3 ];
    boost::csub_match frac_with_integral_group = matches[ 5 ];
    boost::csub_match frac_only_group          = matches[ 6 ];

    int64_t encoded_decimal_val = 0;

    // Parse the string; first check to see if there's an integral part
    if ( integral_group.matched )
    {
        // Get the integral part
        encoded_decimal_val = boost::lexical_cast<int64_t>( std::string( integral_group.first, integral_group.second ) );

        // Do we also have a fractional part?
        if ( frac_with_integral_group.matched )
        {   // yes!
            // The fraction could be zero in length (i.e. the string ends with the decimal point)
            if ( frac_with_integral_group.length() > 0 )
            {
                int64_t fraction = 0;
                fraction = boost::lexical_cast<int64_t>( std::string( frac_with_integral_group.first,
                                                                      frac_with_integral_group.second ) );

                // We need to shift the integral part to the left appropriately
                // before adding the fraction
                int64_t integral_part = encoded_decimal_val * (int64_t) std::pow( 10, frac_with_integral_group.length() );
                encoded_decimal_val   = (integral_part + fraction);
            }
        }  // end fraction given with integer

        // Shift the integer to the left if the fraction is less than 10,000
        switch ( frac_with_integral_group.length() )
        {
            case 0:
                encoded_decimal_val *= 10000; break;
            case 1:
                encoded_decimal_val *= 1000; break;
            case 2:
                encoded_decimal_val *= 100; break;
            case 3:
                encoded_decimal_val *= 10; break;
        }  // end switch
    }
    else if ( frac_only_group.matched ) // no integral part given; only fraction
    {
        encoded_decimal_val = boost::lexical_cast<int64_t>( std::string( frac_only_group.first,
                                                                         frac_only_group.second ) );

        // Adjust the value so that it is always four digits long
        switch ( frac_only_group.length() )
        {
            case 1:
                encoded_decimal_val *= 1000; break;
            case 2:
                encoded_decimal_val *= 100; break;
            case 3:
                encoded_decimal_val *= 10; break;
        }
    }
    else // no match!
    {  // something went wrong in the encoding process
        this->add_long( 0L, is_null );
        m_is_valid = false;
        return;
    }

    // Now handle the sign, if any
    if ( sign_group.matched )
    {
        // Needs action only if negative
        if ( sign_group.compare( "-" ) == 0 )
        {
            encoded_decimal_val = (-1) * encoded_decimal_val;
        }
    }

    this->add_long( encoded_decimal_val, is_null );
    return;
} // end RecordKey::add_decimal



/*
 * Adds a double to the buffer.
 *
 * <param name="value">The double to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_double( double value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::DOUBLE );

    // Handle nulls
    if ( is_null )
    {   // Add eight zeroes for the null value
        this->add( 0 ); // 1st 0
        this->add( 0 ); // 2nd 0
        this->add( 0 ); // 3rd 0
        this->add( 0 ); // 4th 0
        this->add( 0 ); // 5th 0
        this->add( 0 ); // 6th 0
        this->add( 0 ); // 7th 0
        this->add( 0 ); // 8th 0
        return;
    }

    // Copy the bytes of the double to the buffer
    memcpy( &m_buffer[0] + m_current_size, &value, ColumnTypeSize_T::DOUBLE );
    m_current_size += ColumnTypeSize_T::DOUBLE;
    return;
} // end RecordKey::add_double


/*
 * Adds a float to the buffer.
 *
 * <param name="value">The float to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_float( float value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::FLOAT );

    // Handle nulls
    if ( is_null )
    {   // Add four zeroes for the null value
        this->add( 0 ); // 1st 0
        this->add( 0 ); // 2nd 0
        this->add( 0 ); // 3rd 0
        this->add( 0 ); // 4th 0
        return;
    }

    // Copy the bytes of the float to the buffer
    memcpy( &m_buffer[0] + m_current_size, &value, ColumnTypeSize_T::FLOAT );
    m_current_size += ColumnTypeSize_T::FLOAT;
    return;
} // end RecordKey::add_float



/*
 * Adds an integer to the buffer.
 *
 * <param name="value">The integer to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_int( int value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::INT );

    // Handle nulls
    if ( is_null )
    {   // Add four zeroes for the null value
        this->add( 0 ); // 1st 0
        this->add( 0 ); // 2nd 0
        this->add( 0 ); // 3rd 0
        this->add( 0 ); // 4th 0
        return;
    }

    // Copy the bytes of the integer to the buffer
    memcpy( &m_buffer[0] + m_current_size, &value, ColumnTypeSize_T::INT );
    m_current_size += ColumnTypeSize_T::INT;
} // end RecordKey::add_int


/*
 * Adds an int8 to the buffer.
 *
 * <param name="value">The integer/byte to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_int8( int8_t value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::INT8 );

    // Handle nulls
    if ( is_null )
    {   // Add one zero for the null value
        this->add( 0 ); // only 0
        return;
    }

    // Copy the bytes of the int8 to the buffer
    memcpy( &m_buffer[0] + m_current_size, &value, ColumnTypeSize_T::INT8 );
    m_current_size += ColumnTypeSize_T::INT8;
    return;
} // end RecordKey::add_int8


/*
 * Adds an int16 to the buffer.
 *
 * <param name="value">The integer/short to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_int16( int16_t value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::INT16 );

    // Handle nulls
    if ( is_null )
    {   // Add two zeroes for the null value
        this->add( 0 ); // 1st 0
        this->add( 0 ); // 2nd 0
        return;
    }

    // Copy the bytes of the int16 to the buffer
    memcpy( &m_buffer[0] + m_current_size, &value, ColumnTypeSize_T::INT16 );
    m_current_size += ColumnTypeSize_T::INT16;
    return;
} // end RecordKey::add_int16



/*
 * Adds a IPv4 address to the buffer.  Internally, the address is stored
 * as an integer.
 *
 * <param name="value">The IPv4 address to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_ipv4( const std::string& value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::IPV4 );

    // Handle nulls
    if ( is_null )
    {   // Add four zeroes for the null value
        this->add_int( 0, is_null );
        return;
    }

    // Parse the string of the format "x.x.x.x" where each x is in [0, 255]
    unsigned int c0, c1, c2, c3;
    int sscanf_response = sscanf( value.c_str(), "%u.%u.%u.%u", &c0, &c1, &c2, &c3 );

    // Could not parse it as an IPv4 address
    if (sscanf_response != 4)
    {
        this->add_int( 0, is_null );
        m_is_valid = false;
        return;
    }

    // Check that each x is within [0, 255]
    if ( (c0 > 255) || (c1 > 255) || (c2 > 255) || (c3 > 255) )
    {
        this->add_int( 0, is_null );
        m_is_valid = false;
        return;
    }

    // Deduce the integer representing the IPv4 address
    int32_t ipv4_integer = ( (c0 << 24) | (c1 << 16) | (c2 << 8) | c3);
    this->add_int( ipv4_integer, is_null );
    return;
} // end RecordKey::add_ipv4



/*
 * Adds a long to the buffer.
 *
 * <param name="value">The long to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_long( int64_t value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::LONG );

    // Handle nulls
    if ( is_null )
    {   // Add eight zeroes for the null value
        this->add( 0 ); // 1st 0
        this->add( 0 ); // 2nd 0
        this->add( 0 ); // 3rd 0
        this->add( 0 ); // 4th 0
        this->add( 0 ); // 5th 0
        this->add( 0 ); // 6th 0
        this->add( 0 ); // 7th 0
        this->add( 0 ); // 8th 0
        return;
    }

    // Copy the bytes of the long to the buffer
    memcpy( &m_buffer[0] + m_current_size, &value, ColumnTypeSize_T::LONG );
    m_current_size += ColumnTypeSize_T::LONG;
    return;
} // end RecordKey::add_long



/*
 * Adds a string to the buffer.
 *
 * <param name="value">The string to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_string( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::STRING );

    // Handle nulls
    if ( is_null )
    {   // Add eight zeroes for the null value
        this->add( 0 ); // 1st 0
        this->add( 0 ); // 2nd 0
        this->add( 0 ); // 3rd 0
        this->add( 0 ); // 4th 0
        this->add( 0 ); // 5th 0
        this->add( 0 ); // 6th 0
        this->add( 0 ); // 7th 0
        this->add( 0 ); // 8th 0
        return;
    }

    // Get the murmur hash of the string
    uint64_t murmur[2] = {0, 0};
    MurmurHash3_x64_128( value.c_str(), value.size(), GPUDB_HASH_SEED, murmur);

    // Copy the bytes of the string to the buffer
    memcpy( &m_buffer[0] + m_current_size, &murmur, ColumnTypeSize_T::STRING );
    m_current_size += ColumnTypeSize_T::STRING;
    return;
} // end RecordKey::add_string



/// A static utility function for verifying if a given string is a valid
// ulong value
bool RecordKey::verify_ulong_value( const std::string& value )
{
    static const size_t max_len = std::to_string( ULONG_MAX ).size();

    // Need to have the correct number of characters
    size_t str_len = value.size();
    if ((str_len == 0) || (str_len > max_len))
    {
        return false;
    }

    // Each character must be a digit
    for (size_t i = 0; i < str_len; ++i )
    {
        if ( !std::isdigit( static_cast<unsigned char>(value[i]) ) )
        {
            return false;
        }
    }
    return true;
}



/*
 * Adds an unsigned long to the buffer.
 *
 * <param name="value">The unsigned long to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_ulong( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::ULONG );

    // Handle nulls
    if ( is_null )
    {   // Add eight zeroes for the null value
        this->add( 0 ); // 1st 0
        this->add( 0 ); // 2nd 0
        this->add( 0 ); // 3rd 0
        this->add( 0 ); // 4th 0
        this->add( 0 ); // 5th 0
        this->add( 0 ); // 6th 0
        this->add( 0 ); // 7th 0
        this->add( 0 ); // 8th 0
        return;
    }

    if ( !verify_ulong_value( value ) )
    {
        throw GPUdbException( "Unable to parse string value '" + value
                              + "' as an unsigned long" );
    }

    // Process the data
    uint64_t unsigned_long_value;
    try
    {
        unsigned_long_value = std::stoul( value );
    } catch (const std::exception& e)
    {
        throw GPUdbException( "Unable to parse string value '" + value
                              + "' as an unsigned long" );
    }

    // Copy the bytes of the unsigned long to the buffer
    memcpy( &m_buffer[0] + m_current_size,
            &unsigned_long_value,
            ColumnTypeSize_T::ULONG );
        
    m_current_size += ColumnTypeSize_T::ULONG;

} // end RecordKey::add_ulong


/*
 * Adds a uuid to the buffer.
 *
 * <param name="value">The uuid to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_uuid( const std::string& value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::UUID );

    // Handle nulls
    if ( is_null )
    {   // Add 16 bytes of zeroes for the null value
        this->add_long( 0, is_null );
        this->add_long( 0, is_null );
        return;
    }

    // Final group of 12 split into two sections.  E.g., 123e4567-e89b-12d3-a456-426614174000
    long parts[6];
    int sscanf_response = sscanf( value.c_str(), "%8lx-%4lx-%4lx-%4lx-%4lx%8lx", &parts[0], &parts[1], &parts[2], &parts[3], &parts[4], &parts[5] );

    // Could not parse it as a uuid with hyphens
    if (sscanf_response != 6)
    {
        // Try it without hyphens
        sscanf_response = sscanf( value.c_str(), "%8lx%4lx%4lx%4lx%4lx%8lx", &parts[0], &parts[1], &parts[2], &parts[3], &parts[4], &parts[5] );

        if (sscanf_response != 6)
        {
            this->add_long( 0, is_null );
            this->add_long( 0, is_null );
            m_is_valid = false;
            return;
        }
    }

    // // See: uuid_string_to_int128_t()
    this->add_int((int)parts[5], is_null); // Last 8 hex digits of last group
    this->add_int16((int16_t)parts[4], is_null); // First 4 hex digits of last group
    this->add_int16((int16_t)parts[3], is_null); // Fourth group, 4 hex digits
    this->add_int16((int16_t)parts[2], is_null); // Third group, 4 hex digits
    this->add_int16((int16_t)parts[1], is_null); // Second group, 4 hex digits
    this->add_int((int)parts[0], is_null); // First 8 hex digits

    return;
} // end RecordKey::add_uuid



// Needed for parsing time
// Handle with and without milliseconds, with and without leading '0'
// Possible formats:
// '02:10:10.123' length: 12
// '2:10:10.123'  length: 11
// '2:10:10.1'    length:  9
// '02:10:10'     length:  8
// '2:10:10'      length:  7
static boost::regex time_regex( "\\A\\s*(\\d{1,2}):(\\d{2}):(\\d{2})(\\.(\\d{1,3}))?\\s*$" );

/*
 * Adds a string of the format "HH:MM:SS[.mmm]" (where the milliseconds are
 * optional) to the buffer.  Internally, the time is stored as an integer.
 *
 * <param name="value">The time to be added.  Must be of the format
 * "HH:MM:SS[.mmm]".</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_time( const std::string &value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::TIME );

    // Handle nulls
    if ( is_null )
    {   // Add a null int
        this->add_int( 0, is_null );
        return;
    }

    // Does the given value match our format of YYYY-MM-DD?
    boost::cmatch matches;
    if ( !boost::regex_match( value.c_str(), matches, time_regex ) )
    {  // No match, so the key is invalid
        this->add_int( 0, is_null );
        m_is_valid = false;
        return;
    }

    // Regex groups for specific parts
    boost::csub_match hr_group  = matches[ 1 ];
    boost::csub_match min_group = matches[ 2 ];
    boost::csub_match sec_group = matches[ 3 ];
    boost::csub_match ms_group  = matches[ 5 ];

    // Get the year, month and time out
    int hour   = boost::lexical_cast<int>( std::string( hr_group.first,  hr_group.second  ) );
    int minute = boost::lexical_cast<int>( std::string( min_group.first, min_group.second ) );
    int second = boost::lexical_cast<int>( std::string( sec_group.first, sec_group.second ) );
    int millisecond = 0;

    // The millisecond component is optional
    if ( ms_group.matched )
        millisecond = boost::lexical_cast<int>( std::string( ms_group.first, ms_group.second ) );

    // Handle single and double digits for milliseconds
    switch ( ms_group.length() )
    {
        case 1:
            millisecond *= 100; break;
        case 2:
            millisecond *= 10; break;
    }

    // Process the time
    int32_t encoded_time_integer = 0;
    bool encoding_success = timestamp::process_time( hour, minute, second, millisecond, encoded_time_integer );
    if ( !encoding_success )
    {  // something went wrong in the encoding process
        this->add_int( 0, is_null );
        m_is_valid = false;
        return;
    }

    this->add_int( encoded_time_integer, is_null );
    return;
} // end RecordKey::add_time



/*
 * Adds a timestamp (long) to the buffer.
 *
 * <param name="value">The timestamp to be added.</param>
 * <param name="is_null">Indicates that a null value is to be added.</param>
 */
void RecordKey::add_timestamp( int64_t value, bool is_null )
{
    // Check if the given number of characters will fit in the buffer
    this->will_buffer_overflow( ColumnTypeSize_T::TIMESTAMP );

    // Handle nulls
    if ( is_null )
    {   // Add eight zeroes for the null value
        this->add( 0 ); // 1st 0
        this->add( 0 ); // 2nd 0
        this->add( 0 ); // 3rd 0
        this->add( 0 ); // 4th 0
        this->add( 0 ); // 5th 0
        this->add( 0 ); // 6th 0
        this->add( 0 ); // 7th 0
        this->add( 0 ); // 8th 0
        return;
    }

    // Encode the timestamp properly
    int64_t timestamp = timestamp::create_timestamp_from_epoch_ms( value );

    // Copy the bytes of the timestamp to the buffer
    memcpy( &m_buffer[0] + m_current_size, &timestamp, ColumnTypeSize_T::TIMESTAMP );
    m_current_size += ColumnTypeSize_T::TIMESTAMP;
    return;
} // end RecordKey::add_timestamp


/*
 * Compute the hash of the key in the buffer.  Use the Murmurhash3
 * algorithm to compute the hash.  If not all of the values have been
 * added to the key (i.e. if the buffer is not full), then throw an
 * exception.
 */
void RecordKey::compute_hash()
{
    // Check all the values for the key have been added
//    if ( m_buffer.size() != m_buffer_size )
    if ( m_current_size != m_buffer_size )
    {
        std::ostringstream ss;
        ss << "The RecordKey buffer is not full; check that all the relevant values have been added (buffer size "
           << m_buffer_size << ", current buffer size " << m_current_size;
        throw GPUdbException( ss.str() );
    }

    // Check that it's not a zero-sized key
    if ( m_buffer_size == 0 )
        throw GPUdbException( "The RecordKey has zero buffer size; cannot compute any hash.  Reset size and add values." );

    // Hash the value
    int64_t murmur[2] = {0, 0};

    // run the hash function
    MurmurHash3_x64_128( &m_buffer[0], m_buffer_size, GPUDB_HASH_SEED, murmur);

    // unpack
    m_routing_hash = murmur[0];

    // Calculate the 32-bit hash code
    m_hash_code = (int32_t) ( m_routing_hash ^ ((m_routing_hash >> 32) & 0x0000ffffL) );

    // Flag indicating that now the key is complete and ready to be used
    m_key_is_complete = true;
}  // end compute_hash



/*
 * Given a routing table consisting of worker rank indices, choose a
 * worker rank based on the hash of the record key.
 *
 * <param name="routing_table">A list of integers which represent worker ranks.</param>
 *
 * <returns>The appropriate entry from <paramref name="routingTable"/>.</returns>
 */
size_t RecordKey::route( const std::vector<int32_t>& routing_table ) const
{
    if ( !m_key_is_complete )
        throw GPUdbException( "Can't route using the RecordKey since it is not ready to be used; check that all the relevant values have been added." );

    // Return 1 less than the value of the nth element of routingTable where
    //    n == (record key hash) % (number of elements in routingTable)
    // (because the 1st worker rank is the 0th element in the worker list)
    int32_t destination_rank = routing_table[ std::abs( m_routing_hash % (int)routing_table.size() ) ];
    return (size_t)(destination_rank - 1);
}  // end route


// Returns true if the other RecordKey is equivalent to this key
// (if the routing hashes are the same)
bool RecordKey::operator==(const RecordKey& rhs) const
{
    return ( m_routing_hash == rhs.m_routing_hash );
}  // end ==


// Returns true if this RecordKey is less than the other key
// (if the routing hash of this one is less than that of the other)
bool RecordKey::operator<(const RecordKey& rhs) const
{
    return ( m_routing_hash < rhs.m_routing_hash );
}  // end <



// --------------------- END Class RecordKey ----------------------






// --------------------- Begin Class RecordKeyBuilder ----------------------

/*
 * Constructor.
 * <param name="is_primary_key">Indicates whether the RecordKey to be built
 * is a primary key or not.</param>
 * <param name="record_type">The type of the record that this builder is for.</param>
 */
RecordKeyBuilder::RecordKeyBuilder( bool is_primary_key, const gpudb::Type& record_type ) :
    m_record_type( record_type )
{
    m_key_buffer_size = 0;

    // We need to deduce if it's a "track" type:
    // does it have x, y, timestamp, and track ID.
    // If it is, the track ID column will be a routing column
    bool has_x = false;
    bool has_y = false;
    bool has_timestamp = false;
    int track_id_column_idx = -1; // not found yet

    const std::vector<gpudb::Type::Column> columns = record_type.getColumns();
    std::string sk_property = gpudb::ColumnProperty::SHARD_KEY;
    if (!is_primary_key)
    {
        // If shard_key == primary_key, then columns will only have the PK property
        // so we'll need to use that when checking for shard_key

        // Go through the list and see if any have the shard_key property
        bool found_sk = false;
        for ( size_t i = 0; i < columns.size(); ++i )
            if (columns[ i ].hasProperty( gpudb::ColumnProperty::SHARD_KEY ))
            {
                found_sk = true;
                break;
            }
        
        if (!found_sk)
            sk_property = gpudb::ColumnProperty::PRIMARY_KEY;
    }

    for ( size_t i = 0; i < columns.size(); ++i )
    {
        // Get the column
        gpudb::Type::Column column = columns[ i ];

        // Check if it is one of: x, y, timestamp, track ID
        std::string column_name = column.getName();

        if ( column_name.compare( "x" ) == 0 )
            has_x = true;

        if ( column_name.compare( "y" ) == 0 )
            has_y = true;

        if ( column_name.compare( "TIMESTAMP" ) == 0 )
            has_timestamp = true;

        if ( column_name.compare( "TRACKID" ) == 0 )
            track_id_column_idx = i;

        // Check if this column has been declared as a primary/shard key
        // And if so, and if appropriate, add it to the routing key column list
        if ( is_primary_key && column.hasProperty( gpudb::ColumnProperty::PRIMARY_KEY ) )
        {
            m_pk_shard_key_indices.push_back( i );
        }
        else if ( !is_primary_key && column.hasProperty( sk_property ) )
        {
            m_pk_shard_key_indices.push_back( i );
        }
    }  // end loop

    // Check if this is a track-type table; if so, add the track ID column's index to the list
    if ( !is_primary_key && has_timestamp && has_x && has_y
         && (track_id_column_idx != -1) )
    {
        if ( m_pk_shard_key_indices.size() == 0)
        {
            m_pk_shard_key_indices.push_back( track_id_column_idx );
        }
        else if ( (m_pk_shard_key_indices.size() != 1)
                  || (m_pk_shard_key_indices[0] != track_id_column_idx) )
            throw GPUdbException( "Cannot have a shard key other than 'TRACKID' for track tables." );
    } // end processing track types

    // For each index of routing columns, save the column type, and increase
    // the buffer size appropriately
    for (size_t i = 0; i < m_pk_shard_key_indices.size(); ++i)
    {
        // Get the column info
        gpudb::Type::Column column = columns[ m_pk_shard_key_indices[ i ] ];

        switch( column.getType() )
        {
            case gpudb::Type::Column::ColumnType::DOUBLE:
            {
                m_column_types.push_back( ColumnType_T::DOUBLE );
                m_key_buffer_size += ColumnTypeSize_T::DOUBLE;
                break;
            }

            case gpudb::Type::Column::ColumnType::FLOAT:
            {
                m_column_types.push_back( ColumnType_T::FLOAT );
                m_key_buffer_size += ColumnTypeSize_T::FLOAT;
                break;
            }

            case gpudb::Type::Column::ColumnType::INT:
            {   // Int can be a regular integer or boolean or int8 or int16
                if ( column.hasProperty( gpudb::ColumnProperty::BOOLEAN ) )
                {
                    m_column_types.push_back( ColumnType_T::BOOLEAN );
                    m_key_buffer_size += ColumnTypeSize_T::BOOLEAN;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::INT8 ) )
                {
                    m_column_types.push_back( ColumnType_T::INT8 );
                    m_key_buffer_size += ColumnTypeSize_T::INT8;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::INT16 ) )
                {
                    m_column_types.push_back( ColumnType_T::INT16 );
                    m_key_buffer_size += ColumnTypeSize_T::INT16;
                }
                else // a regular integer
                {
                    m_column_types.push_back( ColumnType_T::INT );
                    m_key_buffer_size += ColumnTypeSize_T::INT;
                }
                break;
            }  // end case int

            case gpudb::Type::Column::ColumnType::LONG:
            {   // Long can be either a regular long or a timestamp
                if ( column.hasProperty( gpudb::ColumnProperty::TIMESTAMP ) )
                {
                    m_column_types.push_back( ColumnType_T::TIMESTAMP );
                    m_key_buffer_size += ColumnTypeSize_T::TIMESTAMP;
                }
                else  // regular long
                {
                    m_column_types.push_back( ColumnType_T::LONG );
                    m_key_buffer_size += ColumnTypeSize_T::LONG;
                }
                break;
            }  // end case long

            case gpudb::Type::Column::ColumnType::STRING:
            {   // Strings can have a plethora of (mutually exclusive) properties
                if ( column.hasProperty( gpudb::ColumnProperty::CHAR1 ) )
                {
                    m_column_types.push_back( ColumnType_T::CHAR1 );
                    m_key_buffer_size += ColumnTypeSize_T::CHAR1;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::CHAR2 ) )
                {
                    m_column_types.push_back( ColumnType_T::CHAR2 );
                    m_key_buffer_size += ColumnTypeSize_T::CHAR2;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::CHAR4 ) )
                {
                    m_column_types.push_back( ColumnType_T::CHAR4 );
                    m_key_buffer_size += ColumnTypeSize_T::CHAR4;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::CHAR8 ) )
                {
                    m_column_types.push_back( ColumnType_T::CHAR8 );
                    m_key_buffer_size += ColumnTypeSize_T::CHAR8;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::CHAR16 ) )
                {
                    m_column_types.push_back( ColumnType_T::CHAR16 );
                    m_key_buffer_size += ColumnTypeSize_T::CHAR16;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::CHAR32 ) )
                {
                    m_column_types.push_back( ColumnType_T::CHAR32 );
                    m_key_buffer_size += ColumnTypeSize_T::CHAR32;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::CHAR64 ) )
                {
                    m_column_types.push_back( ColumnType_T::CHAR64 );
                    m_key_buffer_size += ColumnTypeSize_T::CHAR64;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::CHAR128 ) )
                {
                    m_column_types.push_back( ColumnType_T::CHAR128 );
                    m_key_buffer_size += ColumnTypeSize_T::CHAR128;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::CHAR256 ) )
                {
                    m_column_types.push_back( ColumnType_T::CHAR256 );
                    m_key_buffer_size += ColumnTypeSize_T::CHAR256;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::DATE ) )
                {
                    m_column_types.push_back( ColumnType_T::DATE );
                    m_key_buffer_size += ColumnTypeSize_T::DATE;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::DATETIME ) )
                {
                    m_column_types.push_back( ColumnType_T::DATETIME );
                    m_key_buffer_size += ColumnTypeSize_T::DATETIME;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::DECIMAL ) )
                {
                    m_column_types.push_back( ColumnType_T::DECIMAL );
                    m_key_buffer_size += ColumnTypeSize_T::DECIMAL;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::IPV4) )
                {
                    m_column_types.push_back( ColumnType_T::IPV4 );
                    m_key_buffer_size += ColumnTypeSize_T::IPV4;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::TIME ) )
                {
                    m_column_types.push_back( ColumnType_T::TIME );
                    m_key_buffer_size += ColumnTypeSize_T::TIME;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::ULONG ) )
                {
                    m_column_types.push_back( ColumnType_T::ULONG );
                    m_key_buffer_size += ColumnTypeSize_T::ULONG;
                }
                else if ( column.hasProperty( gpudb::ColumnProperty::UUID) )
                {
                    m_column_types.push_back( ColumnType_T::UUID );
                    m_key_buffer_size += ColumnTypeSize_T::UUID;
                }
                else // a regular string
                {
                    m_column_types.push_back( ColumnType_T::STRING );
                    m_key_buffer_size += ColumnTypeSize_T::STRING;
                }
                break;
            }  // end case string

            // Other types are not allowed for routing columns
            case gpudb::Type::Column::BYTES:
            default:
                throw GPUdbException( "Cannot use column '" + column.getName()
                                      + "' as a sharding column." );
        }  // end switch
    }  // end loop

}  // end constructor



/*
 * Build a RecordKey object based on a record.
 *
 * <param name="record">The record off which the key needs to be built.</param>
 *
 * <param name="record_key">The record key to be built; any pre-existing info in the
 * key will be obliterated (i.e. the key will be reset).</param>
 *
 * <returns>True if the record key could be built; false otherwise.</returns>
 */
bool RecordKeyBuilder::build( const gpudb::GenericRecord& record, RecordKey& record_key ) const
{
    // Can't build a key if the buffer size is zero!
    if ( m_key_buffer_size == 0 )
        return false;

    // Reset the passed in key so that it matches this key builder's type
    record_key.reset( m_key_buffer_size );

    // Add each routing column's value to the key
    size_t num_key_columns = m_pk_shard_key_indices.size();
    for ( size_t i = 0; i < num_key_columns; ++i )
    {
        // Get the index of the column inside the record
        size_t column_index = m_pk_shard_key_indices[ i ];

        // Get the column information (useful for finding out nullability)
        gpudb::Type::Column column = m_record_type.getColumn( column_index );

        // Figure out if it's a nullable type and also if it's a null value
        // (which tells us whether we can get a value out of the column)
        bool is_null = record.isNull( column_index );

        // Get the relevant column's value out from the record
        // (the type may be different per column) and add to the record key
        switch ( m_column_types[ i ] )  // ith column
        {
            case ColumnType_T::CHAR1:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the char1 value (or null) to the record key
                record_key.add_char1( value, is_null );
                break;
            }  // end case char1

            case ColumnType_T::CHAR2:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the char2 value (or null) to the record key
                record_key.add_char2( value, is_null );
                break;
            }  // end case char2

            case ColumnType_T::CHAR4:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the char4 value (or null) to the record key
                record_key.add_char4( value, is_null );
                break;
            }  // end case char4

            case ColumnType_T::CHAR8:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the char8 value (or null) to the record key
                record_key.add_char8( value, is_null );
                break;
            }  // end case char8

            case ColumnType_T::CHAR16:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the char16 value (or null) to the record key
                record_key.add_char16( value, is_null );
                break;
            }  // end case char16

            case ColumnType_T::CHAR32:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the char32 value (or null) to the record key
                record_key.add_char32( value, is_null );
                break;
            }  // end case char32

            case ColumnType_T::CHAR64:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the char64 value (or null) to the record key
                record_key.add_char64( value, is_null );
                break;
            }  // end case char64

            case ColumnType_T::CHAR128:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the char128 value (or null) to the record key
                record_key.add_char128( value, is_null );
                break;
            }  // end case char128

            case ColumnType_T::CHAR256:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the char256 value (or null) to the record key
                record_key.add_char256( value, is_null );
                break;
            }  // end case char256

            case ColumnType_T::DATE:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the date value (or null) to the record key
                record_key.add_date( value, is_null );
                break;
            }  // end case date

            case ColumnType_T::DATETIME:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the datetime value (or null) to the record key
                record_key.add_datetime( value, is_null );
                break;
            }  // end case datetime

            case ColumnType_T::DECIMAL:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the decimal value (or null) to the record key
                record_key.add_decimal( value, is_null );
                break;
            }  // end case decimal

            case ColumnType_T::DOUBLE:
            {
                double value = 0;
                if ( !is_null )
                    value = record.getAsDouble( column_index );

                // Now we add the double value (or null) to the record key
                record_key.add_double( value, is_null );
                break;
            }  // end case double

            case ColumnType_T::FLOAT:
            {
                float value = 0;
                if ( !is_null )
                    value = record.getAsFloat( column_index );

                // Now we add the float value (or null) to the record key
                record_key.add_float( value, is_null );
                break;
            }  // end case float

            case ColumnType_T::INT:
            {
                int32_t value = 0;
                if ( !is_null )
                    value = record.getAsInt( column_index );

                // Now we add the int value (or null) to the record key
                record_key.add_int( value, is_null );
                break;
            }  // end case int

            case ColumnType_T::BOOLEAN:
            case ColumnType_T::INT8:
            {
                int32_t value = 0;
                if ( !is_null )
                    value = record.getAsInt( column_index );

                // Now we add the int8 value (or null) to the record key
                record_key.add_int8( (int8_t)value, is_null );
                break;
            }  // end case int8

            case ColumnType_T::INT16:
            {
                int32_t value = 0;
                if ( !is_null )
                    value = record.getAsInt( column_index );

                // Now we add the int16 value (or null) to the record key
                record_key.add_int16( (int16_t)value, is_null );
                break;
            }  // end case int16

            case ColumnType_T::IPV4:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the IPv4 value (or null) to the record key
                record_key.add_ipv4( value, is_null );
                break;
            }  // end case ipv4

            case ColumnType_T::LONG:
            {
                int64_t value = 0;
                if ( !is_null )
                    value = record.getAsLong( column_index );

                // Now we add the long value (or null) to the record key
                record_key.add_long( value, is_null );
                break;
            }  // end case long

            case ColumnType_T::TIME:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the time value (or null) to the record key
                record_key.add_time( value, is_null );
                break;
            }  // end case time

            case ColumnType_T::TIMESTAMP:
            {
                int64_t value = 0;
                if ( !is_null )
                    value = record.getAsLong( column_index );

                // Now we add the long value (or null) to the record key
                record_key.add_timestamp( value, is_null );
                break;
            }  // end case timestamp

            case ColumnType_T::ULONG:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the long value (or null) to the record key
                record_key.add_ulong( value, is_null );
                break;
            }  // end case timestamp

            case ColumnType_T::UUID:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the UUID value (or null) to the record key
                record_key.add_uuid( value, is_null );
                break;
            }  // end case uuid

            case ColumnType_T::STRING:
            {
                std::string value;
                if ( !is_null )
                    value = record.getAsString( column_index );

                // Now we add the string value (or null) to the record key
                record_key.add_string( value, is_null );
                break;
            }  // end case string

            default:
                throw GPUdbException( "Unhandled type for column '" + column.getName()
                                      + "'."  );
        }  // end switch on column type
    }  // end loop

    // Compute the hash for the key before sending it
    record_key.compute_hash();

    return true;  // we succeeded in building a key
}  // end build



// Build a key-lookup expression based on a generic record
bool RecordKeyBuilder::buildExpression( const gpudb::GenericRecord& record,
                                        std::string& result ) const
{
    if ( m_key_buffer_size == 0 )
        return false;

    // Now build the string expression based on all shardin
    // column (e.g. 'x = 2 and s = "blah"')
    std::stringstream ss;
    size_t num_key_columns = m_pk_shard_key_indices.size();
    for (size_t i = 0; i < num_key_columns; ++i)
    {
        if ( i > 0 )  // Need a conjunction
            ss << " and ";

        // Get the index of the column inside the record
        size_t column_index = m_pk_shard_key_indices[ i ];

        // Get the column information (useful for finding out nullability)
        gpudb::Type::Column column = m_record_type.getColumn( column_index );

        // Figure out if it's a nullable type and also if it's a null value
        // (which tells us whether we can get a value out of the column)
        if ( record.isNull( column_index ) )
        {
            ss << "is_null(" << column.getName() << ")";
            continue; // skip the rest of the loop for this column
        }

        // Not a null; so just add "(name = value)"
        ss << "(" << column.getName() << " = ";

        // Add the value
        // Get the relevant column's value out from the record
        // (the type may be different per column) and add to the record key
        switch ( m_column_types[ i ] )  // ith column
        {
            case ColumnType_T::CHAR1:
            case ColumnType_T::CHAR2:
            case ColumnType_T::CHAR4:
            case ColumnType_T::CHAR8:
            case ColumnType_T::CHAR16:
            case ColumnType_T::CHAR32:
            case ColumnType_T::CHAR64:
            case ColumnType_T::CHAR128:
            case ColumnType_T::CHAR256:
            case ColumnType_T::DATE:
            case ColumnType_T::DATETIME:
            case ColumnType_T::DECIMAL:
            case ColumnType_T::IPV4:
            case ColumnType_T::TIME:
            case ColumnType_T::STRING:
            case ColumnType_T::UUID:
            {
                ss << "\""
                   << record.getAsString( column_index )
                   << "\"";
                break;
            }  // end case string

            case ColumnType_T::ULONG:
            {
                // Need to verify if the string is an actual unsigned long value
                std::string value = record.getAsString( column_index );
                if ( !RecordKey::verify_ulong_value( value ) )
                {
                    throw GPUdbException( "Unable to parse string value '" + value
                                          + "' as an unsigned long while building expression" );
                }

                // Unsigned longs, for query purposes, are just numbers, not strings                    
                ss << value;
                break;
            }  // end case string


            case ColumnType_T::DOUBLE:
            {
                ss << record.getAsDouble( column_index );  break;
            }

            case ColumnType_T::FLOAT:
            {
                ss << record.getAsFloat( column_index );  break;
            }

            case ColumnType_T::BOOLEAN:
            case ColumnType_T::INT8:
            case ColumnType_T::INT16:
            case ColumnType_T::INT:
            {
                ss << record.getAsInt( column_index );  break;
            }

            case ColumnType_T::LONG:
            case ColumnType_T::TIMESTAMP:
            {
                ss << record.getAsLong( column_index );  break;
            }

            default:
            {
                throw GPUdbException( "Unhandled type for column '" + column.getName()
                                      + "'."  );
            }
        }  // end switch on column type

        // Add the closing parenthesis
        ss << ")";
    }  // end loop

    result = ss.str();
    return true;
}  // end buildExpression



// Returns if the other RecordKeyBuilder is equivalent to this builder
bool RecordKeyBuilder::operator==(const RecordKeyBuilder& rhs) const
{
    return ( m_column_types == rhs.m_column_types );
}  // end ==


// --------------------- END Class RecordKeyBuilder ----------------------





// --------------------- Begin Class WorkerQueue ----------------------
WorkerQueue::WorkerQueue( const std::string& url ) :
      m_url( url ),
      m_capacity( 1 )
{
    // Reserve enough capacity for the queue
    m_queue = recordVector_T();
    m_queue.reserve( m_capacity );
    return;
}  // end WorkerQueue constructor



WorkerQueue::WorkerQueue( const std::string& url, size_t capacity ) :
      m_url( url ),
      m_capacity( capacity )
{
    // Reserve enough capacity for the queue
    m_queue = recordVector_T();
    m_queue.reserve( capacity );

    return;
}  // end WorkerQueue constructor


WorkerQueue::~WorkerQueue()
{
    m_queue.clear();
}


/*
 *  Clear queue without sending
 */
void WorkerQueue::clear()
{
    m_queue.clear();
}


/*
 *  Returns the current queue and creates a new internal queue.
 */
void WorkerQueue::flush( recordVector_T& flushed_records )
{
    // Put the records in the outbound (passed in) vector
    flushed_records.clear();
    flushed_records.swap( m_queue );

    // Reserve enough capacity in the "new" queue
    m_queue.reserve( m_capacity );

    return;
}  // end WorkerQueue flush()



/*
 *  Insert a record into the queue.  Returns true if the record was inserted,
 *  false otherwise.
 */
bool WorkerQueue::insert( const gpudb::GenericRecord& record,
                          const RecordKey& /* key */,
                          recordVector_T& flushed_records )
{
    m_queue.push_back( record );
    // If the queue is full to the capacity, flush it and return those records
    if ( m_queue.size() == m_capacity )
    {
        this->flush( flushed_records );
        return true;
    }
    else // queue not full yet
    {
        return false;
    }
}  // end WorkerQueue insert



// --------------------- END Class WorkerQueue ----------------------





} // end namespace gpudb
















