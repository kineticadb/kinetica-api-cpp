/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __ALTER_DIRECTORY_H__
#define __ALTER_DIRECTORY_H__

namespace gpudb
{
    /**
     * A set of parameters for @ref
     * GPUdb::alterDirectory(const AlterDirectoryRequest&) const
     * "GPUdb::alterDirectory".
     *
     * Alters an existing directory in <a href="../../../tools/kifs/"
     * target="_top">KiFS</a>.
     */
    struct AlterDirectoryRequest
    {
        /**
         * Constructs an AlterDirectoryRequest object with default parameters.
         */
        AlterDirectoryRequest() :
            directoryName(std::string()),
            directoryUpdatesMap(std::map<std::string, std::string>()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs an AlterDirectoryRequest object with the specified
         * parameters.
         *
         * @param[in] directoryName_  Name of the directory in KiFS to be
         *                            altered.
         * @param[in] directoryUpdatesMap_  Map containing the properties of
         *                                  the directory to be altered. Error
         *                                  if empty.
         *                                  <ul>
         *                                      <li>@ref
         *                                          gpudb::alter_directory_data_limit
         *                                          "alter_directory_data_limit":
         *                                          The maximum capacity, in
         *                                          bytes, to apply to the
         *                                          directory. Set to -1 to
         *                                          indicate no upper limit.
         *                                  </ul>
         * @param[in] options_  Optional parameters. The default value is an
         *                      empty map.
         */
        AlterDirectoryRequest(const std::string& directoryName_, const std::map<std::string, std::string>& directoryUpdatesMap_, const std::map<std::string, std::string>& options_):
            directoryName( directoryName_ ),
            directoryUpdatesMap( directoryUpdatesMap_ ),
            options( options_ )
        {
        }

        /**
         * Name of the directory in KiFS to be altered.
         */
        std::string directoryName;

        /**
         * Map containing the properties of the directory to be altered. Error
         * if empty.
         * <ul>
         *     <li>@ref gpudb::alter_directory_data_limit
         *         "alter_directory_data_limit": The maximum capacity, in
         *         bytes, to apply to the directory. Set to -1 to indicate no
         *         upper limit.
         * </ul>
         */
        std::map<std::string, std::string> directoryUpdatesMap;

        /**
         * Optional parameters. The default value is an empty map.
         */
        std::map<std::string, std::string> options;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::AlterDirectoryRequest>
    {
        static void encode(Encoder& e, const gpudb::AlterDirectoryRequest& v)
        {
            ::avro::encode(e, v.directoryName);
            ::avro::encode(e, v.directoryUpdatesMap);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::AlterDirectoryRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.directoryName);
                            break;

                        case 1:
                            ::avro::decode(d, v.directoryUpdatesMap);
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
                ::avro::decode(d, v.directoryName);
                ::avro::decode(d, v.directoryUpdatesMap);
                ::avro::decode(d, v.options);
            }
        }
    };
} // end namespace avro

namespace gpudb
{
    /**
     * A set of results returned by @ref
     * GPUdb::alterDirectory(const AlterDirectoryRequest&) const
     * "GPUdb::alterDirectory".
     */
    struct AlterDirectoryResponse
    {
        /**
         * Constructs an AlterDirectoryResponse object with default parameters.
         */
        AlterDirectoryResponse() :
            directoryName(std::string()),
            info(std::map<std::string, std::string>())
        {
        }

        /**
         * Value of @ref gpudb::AlterDirectoryRequest::directoryName
         * "directoryName".
         */
        std::string directoryName;

        /**
         * Additional information.
         */
        std::map<std::string, std::string> info;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::AlterDirectoryResponse>
    {
        static void encode(Encoder& e, const gpudb::AlterDirectoryResponse& v)
        {
            ::avro::encode(e, v.directoryName);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::AlterDirectoryResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.directoryName);
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
                ::avro::decode(d, v.directoryName);
                ::avro::decode(d, v.info);
            }
        }
    };
} // end namespace avro

#endif // __ALTER_DIRECTORY_H__
