/*
 *  This file was autogenerated by the GPUdb schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __ALTER_DATASINK_H__
#define __ALTER_DATASINK_H__

namespace gpudb
{

    /**
     * A set of input parameters for {@link
     * #alterDatasink(const AlterDatasinkRequest&) const}.
     * <p>
     * Alters the properties of an existing <a
     * href="../../../concepts/data_sinks/" target="_top">data sink</a>
     */
    struct AlterDatasinkRequest
    {

        /**
         * Constructs an AlterDatasinkRequest object with default parameter
         * values.
         */
        AlterDatasinkRequest() :
            name(std::string()),
            datasinkUpdatesMap(std::map<std::string, std::string>()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs an AlterDatasinkRequest object with the specified
         * parameters.
         * 
         * @param[in] name_  Name of the data sink to be altered. Must be an
         *                   existing data sink.
         * @param[in] datasinkUpdatesMap_  Map containing the properties of the
         *                                 data sink to be updated. Error if
         *                                 empty.
         *                                 <ul>
         *                                         <li>
         *                                 gpudb::alter_datasink_destination:
         *                                 Destination for the output data in
         *                                 format
         *                                 'destination_type://path[:port]'.
         *                                 Supported destination types are
         *                                 'http', 'https' and 'kafka'.
         *                                         <li>
         *                                 gpudb::alter_datasink_connection_timeout:
         *                                 Timeout in seconds for connecting to
         *                                 this sink
         *                                         <li>
         *                                 gpudb::alter_datasink_wait_timeout:
         *                                 Timeout in seconds for waiting for a
         *                                 response from this sink
         *                                         <li>
         *                                 gpudb::alter_datasink_credential:
         *                                 Name of the <a
         *                                 href="../../../concepts/credentials/"
         *                                 target="_top">credential</a> object
         *                                 to be used in this data sink
         *                                         <li>
         *                                 gpudb::alter_datasink_s3_bucket_name:
         *                                 Name of the Amazon S3 bucket to use
         *                                 as the data sink
         *                                         <li>
         *                                 gpudb::alter_datasink_s3_region:
         *                                 Name of the Amazon S3 region where
         *                                 the given bucket is located
         *                                         <li>
         *                                 gpudb::alter_datasink_s3_aws_role_arn:
         *                                 Amazon IAM Role ARN which has
         *                                 required S3 permissions that can be
         *                                 assumed for the given S3 IAM user
         *                                         <li>
         *                                 gpudb::alter_datasink_hdfs_kerberos_keytab:
         *                                 Kerberos keytab file location for
         *                                 the given HDFS user.  This may be a
         *                                 KIFS file.
         *                                         <li>
         *                                 gpudb::alter_datasink_hdfs_delegation_token:
         *                                 Delegation token for the given HDFS
         *                                 user
         *                                         <li>
         *                                 gpudb::alter_datasink_hdfs_use_kerberos:
         *                                 Use kerberos authentication for the
         *                                 given HDFS cluster
         *                                 <ul>
         *                                         <li>
         *                                 gpudb::alter_datasink_true
         *                                         <li>
         *                                 gpudb::alter_datasink_false
         *                                 </ul>
         *                                 The default value is
         *                                 gpudb::alter_datasink_false.
         *                                         <li>
         *                                 gpudb::alter_datasink_azure_storage_account_name:
         *                                 Name of the Azure storage account to
         *                                 use as the data sink, this is valid
         *                                 only if tenant_id is specified
         *                                         <li>
         *                                 gpudb::alter_datasink_azure_container_name:
         *                                 Name of the Azure storage container
         *                                 to use as the data sink
         *                                         <li>
         *                                 gpudb::alter_datasink_azure_tenant_id:
         *                                 Active Directory tenant ID (or
         *                                 directory ID)
         *                                         <li>
         *                                 gpudb::alter_datasink_azure_sas_token:
         *                                 Shared access signature token for
         *                                 Azure storage account to use as the
         *                                 data sink
         *                                         <li>
         *                                 gpudb::alter_datasink_azure_oauth_token:
         *                                 Oauth token to access given storage
         *                                 container
         *                                         <li>
         *                                 gpudb::alter_datasink_gcs_bucket_name:
         *                                 Name of the Google Cloud Storage
         *                                 bucket to use as the data sink
         *                                         <li>
         *                                 gpudb::alter_datasink_gcs_project_id:
         *                                 Name of the Google Cloud project to
         *                                 use as the data sink
         *                                         <li>
         *                                 gpudb::alter_datasink_gcs_service_account_keys:
         *                                 Google Cloud service account keys to
         *                                 use for authenticating the data sink
         *                                         <li>
         *                                 gpudb::alter_datasink_kafka_url: The
         *                                 publicly-accessible full path URL to
         *                                 the kafka broker, e.g.,
         *                                 'http://172.123.45.67:9300'.
         *                                         <li>
         *                                 gpudb::alter_datasink_kafka_topic_name:
         *                                 Name of the Kafka topic to use for
         *                                 this data sink, if it references a
         *                                 Kafka broker
         *                                         <li>
         *                                 gpudb::alter_datasink_anonymous:
         *                                 Create an anonymous connection to
         *                                 the storage provider--DEPRECATED:
         *                                 this is now the default.  Specify
         *                                 use_managed_credentials for
         *                                 non-anonymous connection
         *                                 <ul>
         *                                         <li>
         *                                 gpudb::alter_datasink_true
         *                                         <li>
         *                                 gpudb::alter_datasink_false
         *                                 </ul>
         *                                 The default value is
         *                                 gpudb::alter_datasink_true.
         *                                         <li>
         *                                 gpudb::alter_datasink_use_managed_credentials:
         *                                 When no credentials are supplied, we
         *                                 use anonymous access by default.  If
         *                                 this is set, we will use cloud
         *                                 provider user settings.
         *                                 <ul>
         *                                         <li>
         *                                 gpudb::alter_datasink_true
         *                                         <li>
         *                                 gpudb::alter_datasink_false
         *                                 </ul>
         *                                 The default value is
         *                                 gpudb::alter_datasink_false.
         *                                         <li>
         *                                 gpudb::alter_datasink_use_https: Use
         *                                 https to connect to datasink if
         *                                 true, otherwise use http
         *                                 <ul>
         *                                         <li>
         *                                 gpudb::alter_datasink_true
         *                                         <li>
         *                                 gpudb::alter_datasink_false
         *                                 </ul>
         *                                 The default value is
         *                                 gpudb::alter_datasink_true.
         *                                         <li>
         *                                 gpudb::alter_datasink_max_batch_size:
         *                                 Maximum number of records per
         *                                 notification message.  The default
         *                                 value is '1'.
         *                                         <li>
         *                                 gpudb::alter_datasink_max_message_size:
         *                                 Maximum size in bytes of each
         *                                 notification message.  The default
         *                                 value is '1000000'.
         *                                         <li>
         *                                 gpudb::alter_datasink_json_format:
         *                                 The desired format of JSON encoded
         *                                 notifications message.
         *                                 If @a nested, records are returned
         *                                 as an array.
         *                                 Otherwise, only a single record per
         *                                 messages is returned.
         *                                 <ul>
         *                                         <li>
         *                                 gpudb::alter_datasink_flat
         *                                         <li>
         *                                 gpudb::alter_datasink_nested
         *                                 </ul>
         *                                 The default value is
         *                                 gpudb::alter_datasink_flat.
         *                                         <li>
         *                                 gpudb::alter_datasink_skip_validation:
         *                                 Bypass validation of connection to
         *                                 this data sink.
         *                                 <ul>
         *                                         <li>
         *                                 gpudb::alter_datasink_true
         *                                         <li>
         *                                 gpudb::alter_datasink_false
         *                                 </ul>
         *                                 The default value is
         *                                 gpudb::alter_datasink_false.
         *                                         <li>
         *                                 gpudb::alter_datasink_schema_name:
         *                                 Updates the schema name.  If @a
         *                                 schema_name
         *                                 doesn't exist, an error will be
         *                                 thrown. If @a schema_name is empty,
         *                                 then the user's
         *                                 default schema will be used.
         *                                 </ul>
         * @param[in] options_  Optional parameters.
         * 
         */
        AlterDatasinkRequest(const std::string& name_, const std::map<std::string, std::string>& datasinkUpdatesMap_, const std::map<std::string, std::string>& options_):
            name( name_ ),
            datasinkUpdatesMap( datasinkUpdatesMap_ ),
            options( options_ )
        {
        }

        std::string name;
        std::map<std::string, std::string> datasinkUpdatesMap;
        std::map<std::string, std::string> options;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::AlterDatasinkRequest>
    {
        static void encode(Encoder& e, const gpudb::AlterDatasinkRequest& v)
        {
            ::avro::encode(e, v.name);
            ::avro::encode(e, v.datasinkUpdatesMap);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::AlterDatasinkRequest& v)
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
                            ::avro::decode(d, v.datasinkUpdatesMap);
                            break;

                        case 2:
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
                ::avro::decode(d, v.datasinkUpdatesMap);
                ::avro::decode(d, v.options);
            }
        }
    };
}

namespace gpudb
{

    /**
     * A set of output parameters for {@link
     * #alterDatasink(const AlterDatasinkRequest&) const}.
     * <p>
     * Alters the properties of an existing <a
     * href="../../../concepts/data_sinks/" target="_top">data sink</a>
     */
    struct AlterDatasinkResponse
    {

        /**
         * Constructs an AlterDatasinkResponse object with default parameter
         * values.
         */
        AlterDatasinkResponse() :
            updatedPropertiesMap(std::map<std::string, std::string>()),
            info(std::map<std::string, std::string>())
        {
        }

        std::map<std::string, std::string> updatedPropertiesMap;
        std::map<std::string, std::string> info;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::AlterDatasinkResponse>
    {
        static void encode(Encoder& e, const gpudb::AlterDatasinkResponse& v)
        {
            ::avro::encode(e, v.updatedPropertiesMap);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::AlterDatasinkResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.updatedPropertiesMap);
                            break;

                        case 1:
                            ::avro::decode(d, v.info);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.updatedPropertiesMap);
                ::avro::decode(d, v.info);
            }
        }
    };
}

#endif