# GPUdb C++ API Changelog

## Version 7.2

### Version 7.2.2.4 - 2025-03-27

#### Fixed
-   Potential for protected headers to be added twice


### Version 7.2.2.3 - 2025-03-10

#### Fixed
-   Issue with duplicated HA headers

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.2.2.2 - 2025-02-24

#### Added
-   Support for parallel HA modes

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.2.2.1 - 2025-01-30

#### Added
-   Boost 1.87+ support

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.2.2.0 - 2024-10-15

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.2.1.0 - 2024-09-07

#### Added
-   OAuth2 authentication support


### Version 7.2.0.2 - 2024-03-20

#### Added
-   Support for disabling auto-discovery, failover, and certificate checking

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.2.0.1 - 2024-02-22

#### Added
-   Additional support for Array, JSON, & Vector types


### Version 7.2.0.0 - 2024-02-11

#### Added
-   Support for Array & Vector types

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.



## Version 7.1

### Version 7.1.10.0 - 2024-05-16

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.1.9.2 - 2024-01-22

#### Added
-   Support for disabling auto-discovery, failover, and certificate checking

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.1.9.1 - 2023-06-23

#### Added
-   Support for ULONG column type
-   Support for UUID column type & primary key

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.1.9.0 - 2023-03-19

#### Added
-   Support for `BOOLEAN` type

#### Changed
-   Improved `GPUdbIngestor` error handling and primary/shard key determination

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.1.8.0 - 2022-10-22

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.1.7.0 - 2022-07-18

#### Changed
-   Removed client-side primary key check, to improve performance and make
    returned errors more consistently delivered

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.1.6.0 - 2022-01-27

#### Added
-   Support for retrieving errant records from GPUdbIngestor ingest

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.1.5.0 - 2021-10-13

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.1.4.0 - 2021-07-29

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.1.3.0 - 2021-03-05

#### Notes
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


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

### Version 7.0.20.0 - 2020-11-25

#### Note
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.0.19.0 - 2020-08-24

#### Note
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.0.18.0 - 2020-07-30

#### Note
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


### Version 7.0.17.0 - 2020-07-06

#### Note
-   Check CHANGELOG-FUNCTIONS.md for endpoint related changes.


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

### Version 6.2.0.3 - 2019-07-18

#### Fixed
-   GenericRecord now correctly parses 'datetime' and 'wkt'/'geometry'
    properties when parsing dynamically generated responses (by endpoints
    like /get/records/bycolumn, for example).


### Version 6.2.0.2 - 2019-05-29

#### Added
-   A getter method for the insertion options to the GPUdbIngestor class


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
