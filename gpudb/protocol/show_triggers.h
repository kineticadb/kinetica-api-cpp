/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __SHOW_TRIGGERS_H__
#define __SHOW_TRIGGERS_H__

namespace gpudb
{
    /**
     * A set of parameters for @ref
     * GPUdb::showTriggers(const ShowTriggersRequest&) const
     * "GPUdb::showTriggers".
     *
     * Retrieves information regarding the specified triggers or all existing
     * triggers currently active.
     */
    struct ShowTriggersRequest
    {
        /**
         * Constructs a ShowTriggersRequest object with default parameters.
         */
        ShowTriggersRequest() :
            triggerIds(std::vector<std::string>()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs a ShowTriggersRequest object with the specified
         * parameters.
         *
         * @param[in] triggerIds_  List of IDs of the triggers whose
         *                         information is to be retrieved. An empty
         *                         list means information will be retrieved on
         *                         all active triggers.
         * @param[in] options_  Optional parameters. The default value is an
         *                      empty map.
         */
        ShowTriggersRequest(const std::vector<std::string>& triggerIds_, const std::map<std::string, std::string>& options_):
            triggerIds( triggerIds_ ),
            options( options_ )
        {
        }

        /**
         * List of IDs of the triggers whose information is to be retrieved. An
         * empty list means information will be retrieved on all active
         * triggers.
         */
        std::vector<std::string> triggerIds;

        /**
         * Optional parameters. The default value is an empty map.
         */
        std::map<std::string, std::string> options;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::ShowTriggersRequest>
    {
        static void encode(Encoder& e, const gpudb::ShowTriggersRequest& v)
        {
            ::avro::encode(e, v.triggerIds);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::ShowTriggersRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.triggerIds);
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
                ::avro::decode(d, v.triggerIds);
                ::avro::decode(d, v.options);
            }
        }
    };
} // end namespace avro

namespace gpudb
{
    /**
     * A set of results returned by @ref
     * GPUdb::showTriggers(const ShowTriggersRequest&) const
     * "GPUdb::showTriggers".
     */
    struct ShowTriggersResponse
    {
        /**
         * Constructs a ShowTriggersResponse object with default parameters.
         */
        ShowTriggersResponse() :
            triggerMap(std::map<std::string, std::map<std::string, std::string> >()),
            info(std::map<std::string, std::string>())
        {
        }

        /**
         * This dictionary contains (key, value) pairs of (trigger ID,
         * information map/dictionary) where the key is a Unicode string
         * representing a Trigger ID. The value is another embedded dictionary
         * containing (key, value) pairs where the keys consist of
         * 'table_name', 'type' and the parameter names relating to the trigger
         * type, e.g.\ *nai*, *min*, *max*. The values are unicode strings
         * (numeric values are also converted to strings) representing the
         * value of the respective parameter. If a trigger is associated with
         * multiple tables, then the string value for *table_name* contains a
         * comma separated list of table names.
         */
        std::map<std::string, std::map<std::string, std::string> > triggerMap;

        /**
         * Additional information.
         */
        std::map<std::string, std::string> info;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::ShowTriggersResponse>
    {
        static void encode(Encoder& e, const gpudb::ShowTriggersResponse& v)
        {
            ::avro::encode(e, v.triggerMap);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::ShowTriggersResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.triggerMap);
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
                ::avro::decode(d, v.triggerMap);
                ::avro::decode(d, v.info);
            }
        }
    };
} // end namespace avro

#endif // __SHOW_TRIGGERS_H__
