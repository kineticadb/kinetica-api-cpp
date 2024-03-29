/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __LIST_GRAPH_H__
#define __LIST_GRAPH_H__

namespace gpudb
{
    /** @private */
    struct ListGraphRequest
    {
        ListGraphRequest() :
            graphName(std::string()),
            options(std::map<std::string, std::string>())
        {
        }

        ListGraphRequest(const std::string& graphName_, const std::map<std::string, std::string>& options_):
            graphName( graphName_ ),
            options( options_ )
        {
        }

        std::string graphName;
        std::map<std::string, std::string> options;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::ListGraphRequest>
    {
        static void encode(Encoder& e, const gpudb::ListGraphRequest& v)
        {
            ::avro::encode(e, v.graphName);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::ListGraphRequest& v)
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
                ::avro::decode(d, v.options);
            }
        }
    };
} // end namespace avro

namespace gpudb
{
    /** @private */
    struct ListGraphResponse
    {
        ListGraphResponse() :
            result(bool()),
            graphNames(std::vector<std::string>()),
            numNodes(std::vector<int64_t>()),
            numEdges(std::vector<int64_t>()),
            info(std::map<std::string, std::string>())
        {
        }

        bool result;
        std::vector<std::string> graphNames;
        std::vector<int64_t> numNodes;
        std::vector<int64_t> numEdges;
        std::map<std::string, std::string> info;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::ListGraphResponse>
    {
        static void encode(Encoder& e, const gpudb::ListGraphResponse& v)
        {
            ::avro::encode(e, v.result);
            ::avro::encode(e, v.graphNames);
            ::avro::encode(e, v.numNodes);
            ::avro::encode(e, v.numEdges);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::ListGraphResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.result);
                            break;

                        case 1:
                            ::avro::decode(d, v.graphNames);
                            break;

                        case 2:
                            ::avro::decode(d, v.numNodes);
                            break;

                        case 3:
                            ::avro::decode(d, v.numEdges);
                            break;

                        case 4:
                            ::avro::decode(d, v.info);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.result);
                ::avro::decode(d, v.graphNames);
                ::avro::decode(d, v.numNodes);
                ::avro::decode(d, v.numEdges);
                ::avro::decode(d, v.info);
            }
        }
    };
} // end namespace avro

#endif // __LIST_GRAPH_H__
