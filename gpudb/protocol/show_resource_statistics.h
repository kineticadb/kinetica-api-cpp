/*
 *  This file was autogenerated by the GPUdb schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __SHOW_RESOURCE_STATISTICS_H__
#define __SHOW_RESOURCE_STATISTICS_H__

namespace gpudb
{

    /**
     * A set of input parameters for {@link
     * #showResourceStatistics(const ShowResourceStatisticsRequest&) const}.
     * <p>
     * Requests various statistics for storage/memory tiers and resource
     * groups.
     * Returns statistics on a per-rank basis.
     */
    struct ShowResourceStatisticsRequest
    {

        /**
         * Constructs a ShowResourceStatisticsRequest object with default
         * parameter values.
         */
        ShowResourceStatisticsRequest() :
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs a ShowResourceStatisticsRequest object with the specified
         * parameters.
         * 
         * @param[in] options_  Optional parameters.
         * 
         */
        ShowResourceStatisticsRequest(const std::map<std::string, std::string>& options_):
            options( options_ )
        {
        }

        std::map<std::string, std::string> options;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::ShowResourceStatisticsRequest>
    {
        static void encode(Encoder& e, const gpudb::ShowResourceStatisticsRequest& v)
        {
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::ShowResourceStatisticsRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.options);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.options);
            }
        }
    };
}

namespace gpudb
{

    /**
     * A set of output parameters for {@link
     * #showResourceStatistics(const ShowResourceStatisticsRequest&) const}.
     * <p>
     * Requests various statistics for storage/memory tiers and resource
     * groups.
     * Returns statistics on a per-rank basis.
     */
    struct ShowResourceStatisticsResponse
    {

        /**
         * Constructs a ShowResourceStatisticsResponse object with default
         * parameter values.
         */
        ShowResourceStatisticsResponse() :
            statisticsMap(std::map<std::string, std::string>()),
            info(std::map<std::string, std::string>())
        {
        }

        std::map<std::string, std::string> statisticsMap;
        std::map<std::string, std::string> info;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::ShowResourceStatisticsResponse>
    {
        static void encode(Encoder& e, const gpudb::ShowResourceStatisticsResponse& v)
        {
            ::avro::encode(e, v.statisticsMap);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::ShowResourceStatisticsResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.statisticsMap);
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
                ::avro::decode(d, v.statisticsMap);
                ::avro::decode(d, v.info);
            }
        }
    };
}

#endif
