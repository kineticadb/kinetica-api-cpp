# GPUdb C++ API Changelog

## Version 7.1

### Version 7.1.2.0 - 2021-01-25

#### Note
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.1.1.0 - 2020-10-28

#### Note
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.



### Version 7.1.0.0 - 2020-08-18

#### Changed
-   Updated the high availability synchronicity modes:
    -   DEAFULT
    -   NONE (no replication done across HA clusters)
    -   SYNCHRONOUS (the server replicates all endpoints synchronously)
    -   ASYNCHRONOUS (the server replicates all endpoints asynchronously)


#### Note
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.



## Version 7.0

### Version 7.0.16.0 - 2020-05-28

#### Note
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.0.15.0 - 2020-04-27

#### Note
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.0.14.0 - 2020-03-25

#### Note
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.



### Version 7.0.13.0 - 2020-03-10

#### Note
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.



### Version 7.0.12.0 - 2020-01-17

#### Note
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.0.11.0 - 2019-11-23

#### Added
-   Support for overriding the high availability synchronicity mode for
    endpoints; set the mode (enum HASynchronicityMode) with the setter
    method setHASyncMode():
    - DEFAULT
    - SYNCHRONOUS
    - ASYNCRHONOUS


### Version 7.0.9.0 - 2019-10-28

#### Added
-   Support for high-availability failover when the database is in the
    offline mode.

#### Changed
-   GPUdb constructor behavior--if a single URL is used and no primary URL
    is specified via the options, the given single URL will be treated as
    the primary URL.



### Version 7.0.7.0 - 2019-08-21

#### Added
-   Support for adding and removing custom headers to the GPUdb object.  See
    methods:
    -   GPUdb.addHttpHeader( const std::string&, const std::string& )
    -   GPUdb.removeHttpHeader( const std::string& )
-   Support for new column property 'ulong' to multi-head I/O.  ***Compatible
    with Kinetica Server version 7.0.7.0 and later only.***

#### Server Version Compatibilty
-   Kinetica 7.0.7.0 and later


### Version 7.0.6.1 - 2019-08-13

#### Changed
-   Added support for high availability failover when the system is limited
    (in addition to connection problems).  ***Compatible with Kinetica Server
    version 7.0.6.2 and later only.***

#### Server Version Compatibilty
-   Kinetica 7.0.6.2 and later


### Version 7.0.6.0 - 2019-07-23

#### Added
-   Support for passing /get/records options to RecordRetriever; can be set
    via the constructors and also by the setter method.

#### Fixed
-   GenericRecord now correctly parses 'datetime' and 'wkt'/'geometry'
    properties when parsing dynamically generated responses (by endpoints
    like /get/records/bycolumn, for example).

### Version 7.0.5.0 - 2019-06-26

#### Added
-   Minor documentation and some options for some endpoints

#### Changed
-   Parameters for /visualize/isoschrone


#### Version 7.0.4.0 -- 2019-05-29

#### Added
-   A getter method for the insertion options to the GPUdbIngestor class

##### Fixed
-   Kinetica exit or other connection errors are propagated to the user
    correctly


#### Version 7.0.3.0 - 2019-05-13
##### Changed
-   /query/graph parameters: removed `restrictions` and added
    `edgeToNode`, `edgeOrNodeIntIds`, `edgeOrNodeStringIds`, and
    `edgeOrNodeWktIds`.

#### Version 7.0.2.0 - 2019-04-24
##### Added
-   Support for selecting a primary host for the GPUdb class


### Version 7.0.0.1 - 2019-02-22
##### Added
-   Support for high availability (HA) failover logic to the
    GPUdb class


### Version 7.0.0.0 - 2019-01-31

-   Version release


## Version 6.2

### Version 6.2.0.1 - 2018-10-05

-   Added support for host manager endpoints
-   Added member dataTypePtr to the response protocol structs that return
    a dynamically generated table.  Currently, that includes:
    -   AggregateGroupByResponse
    -   AggregateUniqueResponse
    -   AggregateUnpivotResponse
    -   GetRecordsByColumnResponse

### Version 6.2.0.0 - 2018-03-24

-   Added new RecordRetriever class to support multi-head record lookup by
    shard key.
-   Refactored the following classes from GPUdbIngestor.*pp to
    utils/GPUdbMultiHeadIOUtils.h/cpp:
    -   WorkerList
    -   RecordKey
    -   RecordKeyBuilder
    -   WorkerQueue


## Version 6.1.0 - 2017-12-11

-   Added support for the 'datetime' property
-   Added multi-head ingestion support
-   Added == overloaded operator for Type and Type::Column
-   Added << (ostream) overloaded operator for Type::Column


## Version 6.0.0 - 2017-01-24

-   Added string constants for column properties


## Version 5.2.0 - 2016-10-12

-   Changed source code directory structure
-   Added nullable column support


## Version 5.1.0 - 2016-05-06

-   Updated documentation generation


## Version 4.2.0 - 2016-04-11

-   Refactor generation of the APIs
