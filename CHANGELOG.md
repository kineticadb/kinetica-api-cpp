# GPUdb C++ API Changelog

## Version 7.0

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
