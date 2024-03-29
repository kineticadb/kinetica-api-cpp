/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __SHOW_DATASOURCE_H__
#define __SHOW_DATASOURCE_H__

namespace gpudb
{
    /**
     * A set of parameters for @ref
     * GPUdb::showDatasource(const ShowDatasourceRequest&) const
     * "GPUdb::showDatasource".
     *
     * Shows information about a specified <a
     * href="../../../concepts/data_sources/" target="_top">data source</a> or
     * all data sources.
     */
    struct ShowDatasourceRequest
    {
        /**
         * Constructs a ShowDatasourceRequest object with default parameters.
         */
        ShowDatasourceRequest() :
            name(std::string()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs a ShowDatasourceRequest object with the specified
         * parameters.
         *
         * @param[in] name_  Name of the data source for which to retrieve
         *                   information. The name must refer to a currently
         *                   existing data source. If '*' is specified,
         *                   information about all data sources will be
         *                   returned.
         * @param[in] options_  Optional parameters. The default value is an
         *                      empty map.
         */
        ShowDatasourceRequest(const std::string& name_, const std::map<std::string, std::string>& options_):
            name( name_ ),
            options( options_ )
        {
        }

        /**
         * Name of the data source for which to retrieve information. The name
         * must refer to a currently existing data source. If '*' is specified,
         * information about all data sources will be returned.
         */
        std::string name;

        /**
         * Optional parameters. The default value is an empty map.
         */
        std::map<std::string, std::string> options;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::ShowDatasourceRequest>
    {
        static void encode(Encoder& e, const gpudb::ShowDatasourceRequest& v)
        {
            ::avro::encode(e, v.name);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::ShowDatasourceRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.name);
                            break;

                        case 1:
                            ::avro::decode(d, v.options);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.name);
                ::avro::decode(d, v.options);
            }
        }
    };
} // end namespace avro

namespace gpudb
{
    /**
     * A set of results returned by @ref
     * GPUdb::showDatasource(const ShowDatasourceRequest&) const
     * "GPUdb::showDatasource".
     */
    struct ShowDatasourceResponse
    {
        /**
         * Constructs a ShowDatasourceResponse object with default parameters.
         */
        ShowDatasourceResponse() :
            datasourceNames(std::vector<std::string>()),
            storageProviderTypes(std::vector<std::string>()),
            additionalInfo(std::vector<std::map<std::string, std::string> >()),
            info(std::map<std::string, std::string>())
        {
        }

        /**
         * The data source names.
         */
        std::vector<std::string> datasourceNames;

        /**
         * The storage provider type of the data sources named in @ref
         * datasourceNames.
         * Supported values:
         * <ul>
         *     <li>@ref gpudb::show_datasource_hdfs "show_datasource_hdfs":
         *         Apache Hadoop Distributed File System
         *     <li>@ref gpudb::show_datasource_s3 "show_datasource_s3": Amazon
         *         S3 bucket
         * </ul>
         */
        std::vector<std::string> storageProviderTypes;

        /**
         * Additional information about the respective data sources in @ref
         * datasourceNames.
         * <ul>
         *     <li>@ref gpudb::show_datasource_location
         *         "show_datasource_location": Location of the remote storage
         *         in 'storage_provider_type://[storage_path[:storage_port]]'
         *         format
         *     <li>@ref gpudb::show_datasource_s3_bucket_name
         *         "show_datasource_s3_bucket_name": Name of the Amazon S3
         *         bucket used as the data source
         *     <li>@ref gpudb::show_datasource_s3_region
         *         "show_datasource_s3_region": Name of the Amazon S3 region
         *         where the bucket is located
         *     <li>@ref gpudb::show_datasource_hdfs_kerberos_keytab
         *         "show_datasource_hdfs_kerberos_keytab": Kerberos key for the
         *         given HDFS user
         *     <li>@ref gpudb::show_datasource_user_name
         *         "show_datasource_user_name": Name of the remote system user
         * </ul>
         */
        std::vector<std::map<std::string, std::string> > additionalInfo;

        /**
         * Additional information.
         */
        std::map<std::string, std::string> info;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::ShowDatasourceResponse>
    {
        static void encode(Encoder& e, const gpudb::ShowDatasourceResponse& v)
        {
            ::avro::encode(e, v.datasourceNames);
            ::avro::encode(e, v.storageProviderTypes);
            ::avro::encode(e, v.additionalInfo);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::ShowDatasourceResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.datasourceNames);
                            break;

                        case 1:
                            ::avro::decode(d, v.storageProviderTypes);
                            break;

                        case 2:
                            ::avro::decode(d, v.additionalInfo);
                            break;

                        case 3:
                            ::avro::decode(d, v.info);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.datasourceNames);
                ::avro::decode(d, v.storageProviderTypes);
                ::avro::decode(d, v.additionalInfo);
                ::avro::decode(d, v.info);
            }
        }
    };
} // end namespace avro

#endif // __SHOW_DATASOURCE_H__
