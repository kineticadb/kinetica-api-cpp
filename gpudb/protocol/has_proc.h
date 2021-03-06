/*
 *  This file was autogenerated by the GPUdb schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __HAS_PROC_H__
#define __HAS_PROC_H__

namespace gpudb
{

    /**
     * A set of input parameters for {@link
     * #hasProc(const HasProcRequest&) const}.
     * <p>
     * Checks the existence of a proc with the given name.
     */
    struct HasProcRequest
    {

        /**
         * Constructs a HasProcRequest object with default parameter values.
         */
        HasProcRequest() :
            procName(std::string()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs a HasProcRequest object with the specified parameters.
         * 
         * @param[in] procName_  Name of the proc to check for existence.
         * @param[in] options_  Optional parameters.
         * 
         */
        HasProcRequest(const std::string& procName_, const std::map<std::string, std::string>& options_):
            procName( procName_ ),
            options( options_ )
        {
        }

        std::string procName;
        std::map<std::string, std::string> options;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::HasProcRequest>
    {
        static void encode(Encoder& e, const gpudb::HasProcRequest& v)
        {
            ::avro::encode(e, v.procName);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::HasProcRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.procName);
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
                ::avro::decode(d, v.procName);
                ::avro::decode(d, v.options);
            }
        }
    };
}

namespace gpudb
{

    /**
     * A set of output parameters for {@link
     * #hasProc(const HasProcRequest&) const}.
     * <p>
     * Checks the existence of a proc with the given name.
     */
    struct HasProcResponse
    {

        /**
         * Constructs a HasProcResponse object with default parameter values.
         */
        HasProcResponse() :
            procName(std::string()),
            procExists(bool()),
            info(std::map<std::string, std::string>())
        {
        }

        std::string procName;
        bool procExists;
        std::map<std::string, std::string> info;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::HasProcResponse>
    {
        static void encode(Encoder& e, const gpudb::HasProcResponse& v)
        {
            ::avro::encode(e, v.procName);
            ::avro::encode(e, v.procExists);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::HasProcResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.procName);
                            break;

                        case 1:
                            ::avro::decode(d, v.procExists);
                            break;

                        case 2:
                            ::avro::decode(d, v.info);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.procName);
                ::avro::decode(d, v.procExists);
                ::avro::decode(d, v.info);
            }
        }
    };
}

#endif
