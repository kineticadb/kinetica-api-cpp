/*
 *  This file was autogenerated by the GPUdb schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __SHOW_GRAPH_GRAMMAR_H__
#define __SHOW_GRAPH_GRAMMAR_H__

namespace gpudb
{

    /**
     * A set of input parameters for {@link
     * #showGraphGrammar(const ShowGraphGrammarRequest&) const}.
     * <p>
     * @private
     */
    struct ShowGraphGrammarRequest
    {

        /**
         * @private
         * Constructs a ShowGraphGrammarRequest object with default parameter
         * values.
         */
        ShowGraphGrammarRequest() :
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * @private
         * Constructs a ShowGraphGrammarRequest object with the specified
         * parameters.
         * 
         * @param[in] options_
         * 
         */
        ShowGraphGrammarRequest(const std::map<std::string, std::string>& options_):
            options( options_ )
        {
        }

        std::map<std::string, std::string> options;
    };
}

    /**
     * @private
     */

namespace avro
{
    template<> struct codec_traits<gpudb::ShowGraphGrammarRequest>
    {
        static void encode(Encoder& e, const gpudb::ShowGraphGrammarRequest& v)
        {
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::ShowGraphGrammarRequest& v)
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
     * #showGraphGrammar(const ShowGraphGrammarRequest&) const}.
     * <p>
     * @private
     */
    struct ShowGraphGrammarResponse
    {

        /**
         * @private
         * Constructs a ShowGraphGrammarResponse object with default parameter
         * values.
         */
        ShowGraphGrammarResponse() :
            result(bool()),
            componentsJson(std::string()),
            info(std::map<std::string, std::string>())
        {
        }

        bool result;
        std::string componentsJson;
        std::map<std::string, std::string> info;
    };
}

    /**
     * @private
     */

namespace avro
{
    template<> struct codec_traits<gpudb::ShowGraphGrammarResponse>
    {
        static void encode(Encoder& e, const gpudb::ShowGraphGrammarResponse& v)
        {
            ::avro::encode(e, v.result);
            ::avro::encode(e, v.componentsJson);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::ShowGraphGrammarResponse& v)
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
                            ::avro::decode(d, v.componentsJson);
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
                ::avro::decode(d, v.result);
                ::avro::decode(d, v.componentsJson);
                ::avro::decode(d, v.info);
            }
        }
    };
}

#endif