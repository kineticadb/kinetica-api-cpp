/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __GPUDB_WRAPPER_H__
#define __GPUDB_WRAPPER_H__

namespace gpudb
{
    /**
     * The wrapper used by the GPUdb server for all endpoint responses.
     */
    struct RawGpudbResponse
    {
        /**
         * Constructs a RawGpudbResponse object with default parameters.
         */
        RawGpudbResponse() :
            status(std::string()),
            message(std::string()),
            dataType(std::string()),
            data(std::vector<uint8_t>()),
            dataStr(std::string())
        {
        }

        std::string status;
        std::string message;
        std::string dataType;
        std::vector<uint8_t> data;
        std::string dataStr;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::RawGpudbResponse>
    {
        static void encode(Encoder& e, const gpudb::RawGpudbResponse& v)
        {
            ::avro::encode(e, v.status);
            ::avro::encode(e, v.message);
            ::avro::encode(e, v.dataType);
            ::avro::encode(e, v.data);
            ::avro::encode(e, v.dataStr);
        }

        static void decode(Decoder& d, gpudb::RawGpudbResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.status);
                            break;

                        case 1:
                            ::avro::decode(d, v.message);
                            break;

                        case 2:
                            ::avro::decode(d, v.dataType);
                            break;

                        case 3:
                            ::avro::decode(d, v.data);
                            break;

                        case 4:
                            ::avro::decode(d, v.dataStr);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.status);
                ::avro::decode(d, v.message);
                ::avro::decode(d, v.dataType);
                ::avro::decode(d, v.data);
                ::avro::decode(d, v.dataStr);
            }
        }
    };
} // end namespace avro

#endif // __GPUDB_WRAPPER_H__
