/*
 *  This file was autogenerated by the GPUdb schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __CLEAR_TABLE_MONITOR_H__
#define __CLEAR_TABLE_MONITOR_H__

namespace gpudb
{

    /**
     * A set of input parameters for {@link
     * #clearTableMonitor(const ClearTableMonitorRequest&) const}.
     * <p>
     * Deactivates a table monitor previously created with {@link
     * #createTableMonitor(const CreateTableMonitorRequest&) const}.
     */
    struct ClearTableMonitorRequest
    {

        /**
         * Constructs a ClearTableMonitorRequest object with default parameter
         * values.
         */
        ClearTableMonitorRequest() :
            topicId(std::string()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs a ClearTableMonitorRequest object with the specified
         * parameters.
         * 
         * @param[in] topicId  The topic ID returned by /create/tablemonitor.
         * @param[in] options  Optional parameters.  Default value is an empty
         *                     std::map.
         * 
         */
        ClearTableMonitorRequest(const std::string& topicId, const std::map<std::string, std::string>& options):
            topicId(topicId),
            options(options)
        {
        }

        std::string topicId;
        std::map<std::string, std::string> options;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::ClearTableMonitorRequest>
    {
        static void encode(Encoder& e, const gpudb::ClearTableMonitorRequest& v)
        {
            ::avro::encode(e, v.topicId);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::ClearTableMonitorRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.topicId);
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
                ::avro::decode(d, v.topicId);
                ::avro::decode(d, v.options);
            }
        }
    };
}

namespace gpudb
{

    /**
     * A set of output parameters for {@link
     * #clearTableMonitor(const ClearTableMonitorRequest&) const}.
     * <p>
     * Deactivates a table monitor previously created with {@link
     * #createTableMonitor(const CreateTableMonitorRequest&) const}.
     */
    struct ClearTableMonitorResponse
    {

        /**
         * Constructs a ClearTableMonitorResponse object with default parameter
         * values.
         */
        ClearTableMonitorResponse() :
            topicId(std::string())
        {
        }

        std::string topicId;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::ClearTableMonitorResponse>
    {
        static void encode(Encoder& e, const gpudb::ClearTableMonitorResponse& v)
        {
            ::avro::encode(e, v.topicId);
        }

        static void decode(Decoder& d, gpudb::ClearTableMonitorResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.topicId);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.topicId);
            }
        }
    };
}

#endif