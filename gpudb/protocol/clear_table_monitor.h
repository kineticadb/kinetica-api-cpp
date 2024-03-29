/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __CLEAR_TABLE_MONITOR_H__
#define __CLEAR_TABLE_MONITOR_H__

namespace gpudb
{
    /**
     * A set of parameters for @ref
     * GPUdb::clearTableMonitor(const ClearTableMonitorRequest&) const
     * "GPUdb::clearTableMonitor".
     *
     * Deactivates a table monitor previously created with @ref
     * GPUdb::createTableMonitor(const CreateTableMonitorRequest&) const
     * "GPUdb::createTableMonitor".
     */
    struct ClearTableMonitorRequest
    {
        /**
         * Constructs a ClearTableMonitorRequest object with default
         * parameters.
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
         * @param[in] topicId_  The topic ID returned by @ref
         *                      GPUdb::createTableMonitor(const CreateTableMonitorRequest&) const
         *                      "GPUdb::createTableMonitor".
         * @param[in] options_  Optional parameters.
         *                      <ul>
         *                          <li>@ref
         *                              gpudb::clear_table_monitor_keep_autogenerated_sink
         *                              "clear_table_monitor_keep_autogenerated_sink":
         *                              If @ref gpudb::clear_table_monitor_true
         *                              "true", the auto-generated <a
         *                              href="../../../concepts/data_sinks/"
         *                              target="_top">datasink</a> associated
         *                              with this monitor, if there is one,
         *                              will be retained for further use. If
         *                              @ref gpudb::clear_table_monitor_false
         *                              "false", then the auto-generated sink
         *                              will be dropped if there are no other
         *                              monitors referencing it.
         *                              Supported values:
         *                              <ul>
         *                                  <li>@ref
         *                                      gpudb::clear_table_monitor_true
         *                                      "clear_table_monitor_true"
         *                                  <li>@ref
         *                                      gpudb::clear_table_monitor_false
         *                                      "clear_table_monitor_false"
         *                              </ul>
         *                              The default value is @ref
         *                              gpudb::clear_table_monitor_false
         *                              "clear_table_monitor_false".
         *                          <li>@ref
         *                              gpudb::clear_table_monitor_clear_all_references
         *                              "clear_table_monitor_clear_all_references":
         *                              If @ref gpudb::clear_table_monitor_true
         *                              "true", all references that share the
         *                              same @a topicId_ will be cleared.
         *                              Supported values:
         *                              <ul>
         *                                  <li>@ref
         *                                      gpudb::clear_table_monitor_true
         *                                      "clear_table_monitor_true"
         *                                  <li>@ref
         *                                      gpudb::clear_table_monitor_false
         *                                      "clear_table_monitor_false"
         *                              </ul>
         *                              The default value is @ref
         *                              gpudb::clear_table_monitor_false
         *                              "clear_table_monitor_false".
         *                      </ul>
         *                      The default value is an empty map.
         */
        ClearTableMonitorRequest(const std::string& topicId_, const std::map<std::string, std::string>& options_):
            topicId( topicId_ ),
            options( options_ )
        {
        }

        /**
         * The topic ID returned by @ref
         * GPUdb::createTableMonitor(const CreateTableMonitorRequest&) const
         * "GPUdb::createTableMonitor".
         */
        std::string topicId;

        /**
         * Optional parameters.
         * <ul>
         *     <li>@ref gpudb::clear_table_monitor_keep_autogenerated_sink
         *         "clear_table_monitor_keep_autogenerated_sink": If @ref
         *         gpudb::clear_table_monitor_true "true", the auto-generated
         *         <a href="../../../concepts/data_sinks/"
         *         target="_top">datasink</a> associated with this monitor, if
         *         there is one, will be retained for further use. If @ref
         *         gpudb::clear_table_monitor_false "false", then the
         *         auto-generated sink will be dropped if there are no other
         *         monitors referencing it.
         *         Supported values:
         *         <ul>
         *             <li>@ref gpudb::clear_table_monitor_true
         *                 "clear_table_monitor_true"
         *             <li>@ref gpudb::clear_table_monitor_false
         *                 "clear_table_monitor_false"
         *         </ul>
         *         The default value is @ref gpudb::clear_table_monitor_false
         *         "clear_table_monitor_false".
         *     <li>@ref gpudb::clear_table_monitor_clear_all_references
         *         "clear_table_monitor_clear_all_references": If @ref
         *         gpudb::clear_table_monitor_true "true", all references that
         *         share the same @ref topicId will be cleared.
         *         Supported values:
         *         <ul>
         *             <li>@ref gpudb::clear_table_monitor_true
         *                 "clear_table_monitor_true"
         *             <li>@ref gpudb::clear_table_monitor_false
         *                 "clear_table_monitor_false"
         *         </ul>
         *         The default value is @ref gpudb::clear_table_monitor_false
         *         "clear_table_monitor_false".
         * </ul>
         * The default value is an empty map.
         */
        std::map<std::string, std::string> options;
    };
} // end namespace gpudb

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
} // end namespace avro

namespace gpudb
{
    /**
     * A set of results returned by @ref
     * GPUdb::clearTableMonitor(const ClearTableMonitorRequest&) const
     * "GPUdb::clearTableMonitor".
     */
    struct ClearTableMonitorResponse
    {
        /**
         * Constructs a ClearTableMonitorResponse object with default
         * parameters.
         */
        ClearTableMonitorResponse() :
            topicId(std::string()),
            info(std::map<std::string, std::string>())
        {
        }

        /**
         * Value of @ref gpudb::ClearTableMonitorRequest::topicId "topicId".
         */
        std::string topicId;

        /**
         * Additional information.
         */
        std::map<std::string, std::string> info;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::ClearTableMonitorResponse>
    {
        static void encode(Encoder& e, const gpudb::ClearTableMonitorResponse& v)
        {
            ::avro::encode(e, v.topicId);
            ::avro::encode(e, v.info);
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
                ::avro::decode(d, v.topicId);
                ::avro::decode(d, v.info);
            }
        }
    };
} // end namespace avro

#endif // __CLEAR_TABLE_MONITOR_H__
