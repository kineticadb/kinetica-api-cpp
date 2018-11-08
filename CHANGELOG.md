# GPUdb C++ API Changelog

## Version 6.2.0

### Version 6.2.0.0 - 2018-03-24

-   Added new RecordRetriever class to support multi-head record lookup by
    shard key.
-   Refactored the following classes from GPUdbIngestor.*pp to
    utils/GPUdbMultiHeadIOUtils.h/cpp:
    -   WorkerList
    -   RecordKey
    -   RecordKeyBuilder
    -   WorkerQueue

### Version 6.2.0.1 - 2018-10-05

-   Added support for host manager endpoints
-   Added member dataTypePtr to the response protocol structs that return
    a dynamically generated table.  Currently, that includes:
    -   AggregateGroupByResponse
    -   AggregateUniqueResponse
    -   AggregateUnpivotResponse
    -   GetRecordsByColumnResponse


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
