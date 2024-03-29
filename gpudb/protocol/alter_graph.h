/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __ALTER_GRAPH_H__
#define __ALTER_GRAPH_H__

namespace gpudb
{
    /** @private */
    struct AlterGraphRequest
    {
        AlterGraphRequest() :
            graphName(std::string()),
            action(std::string()),
            actionArg(std::string()),
            options(std::map<std::string, std::string>())
        {
        }

        AlterGraphRequest(const std::string& graphName_, const std::string& action_, const std::string& actionArg_, const std::map<std::string, std::string>& options_):
            graphName( graphName_ ),
            action( action_ ),
            actionArg( actionArg_ ),
            options( options_ )
        {
        }

        std::string graphName;
        std::string action;
        std::string actionArg;
        std::map<std::string, std::string> options;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::AlterGraphRequest>
    {
        static void encode(Encoder& e, const gpudb::AlterGraphRequest& v)
        {
            ::avro::encode(e, v.graphName);
            ::avro::encode(e, v.action);
            ::avro::encode(e, v.actionArg);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::AlterGraphRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.graphName);
                            break;

                        case 1:
                            ::avro::decode(d, v.action);
                            break;

                        case 2:
                            ::avro::decode(d, v.actionArg);
                            break;

                        case 3:
                            ::avro::decode(d, v.options);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.graphName);
                ::avro::decode(d, v.action);
                ::avro::decode(d, v.actionArg);
                ::avro::decode(d, v.options);
            }
        }
    };
} // end namespace avro

namespace gpudb
{
    /** @private */
    struct AlterGraphResponse
    {
        AlterGraphResponse() :
            action(std::string()),
            actionArg(std::string()),
            info(std::map<std::string, std::string>())
        {
        }

        std::string action;
        std::string actionArg;
        std::map<std::string, std::string> info;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::AlterGraphResponse>
    {
        static void encode(Encoder& e, const gpudb::AlterGraphResponse& v)
        {
            ::avro::encode(e, v.action);
            ::avro::encode(e, v.actionArg);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::AlterGraphResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.action);
                            break;

                        case 1:
                            ::avro::decode(d, v.actionArg);
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
                ::avro::decode(d, v.action);
                ::avro::decode(d, v.actionArg);
                ::avro::decode(d, v.info);
            }
        }
    };
} // end namespace avro

#endif // __ALTER_GRAPH_H__
