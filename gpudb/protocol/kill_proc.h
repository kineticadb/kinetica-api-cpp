/*
 *  This file was autogenerated by the GPUdb schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __KILL_PROC_H__
#define __KILL_PROC_H__

namespace gpudb
{

    /**
     * A set of input parameters for {@link
     * #killProc(const KillProcRequest&) const}.
     * <p>
     * Kills a running proc instance.
     */
    struct KillProcRequest
    {

        /**
         * Constructs a KillProcRequest object with default parameter values.
         */
        KillProcRequest() :
            runId(std::string()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs a KillProcRequest object with the specified parameters.
         * 
         * @param[in] runId_  The run ID of the running proc instance. If the
         *                    run ID is not found or the proc instance has
         *                    already completed, this does nothing. If not
         *                    specified, all running proc instances will be
         *                    killed.  Default value is an empty string.
         * @param[in] options_  Optional parameters.  Default value is an empty
         *                      std::map.
         * 
         */
        KillProcRequest(const std::string& runId_, const std::map<std::string, std::string>& options_):
            runId( runId_ ),
            options( options_ )
        {
        }

        std::string runId;
        std::map<std::string, std::string> options;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::KillProcRequest>
    {
        static void encode(Encoder& e, const gpudb::KillProcRequest& v)
        {
            ::avro::encode(e, v.runId);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::KillProcRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.runId);
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
                ::avro::decode(d, v.runId);
                ::avro::decode(d, v.options);
            }
        }
    };
}

namespace gpudb
{

    /**
     * A set of output parameters for {@link
     * #killProc(const KillProcRequest&) const}.
     * <p>
     * Kills a running proc instance.
     */
    struct KillProcResponse
    {

        /**
         * Constructs a KillProcResponse object with default parameter values.
         */
        KillProcResponse() :
            runIds(std::vector<std::string>())
        {
        }

        std::vector<std::string> runIds;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::KillProcResponse>
    {
        static void encode(Encoder& e, const gpudb::KillProcResponse& v)
        {
            ::avro::encode(e, v.runIds);
        }

        static void decode(Decoder& d, gpudb::KillProcResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.runIds);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.runIds);
            }
        }
    };
}

#endif