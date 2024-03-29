/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __EXPORT_QUERY_METRICS_H__
#define __EXPORT_QUERY_METRICS_H__

namespace gpudb
{
    /**
     * A set of parameters for @ref
     * GPUdb::exportQueryMetrics(const ExportQueryMetricsRequest&) const
     * "GPUdb::exportQueryMetrics".
     *
     * Export query metrics to a given destination.
     * Returns query metrics.
     */
    struct ExportQueryMetricsRequest
    {
        /**
         * Constructs an ExportQueryMetricsRequest object with default
         * parameters.
         */
        ExportQueryMetricsRequest() :
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs an ExportQueryMetricsRequest object with the specified
         * parameters.
         *
         * @param[in] options_  Optional parameters.
         *                      <ul>
         *                          <li>@ref
         *                              gpudb::export_query_metrics_expression
         *                              "export_query_metrics_expression":
         *                              Filter for multi query export
         *                          <li>@ref
         *                              gpudb::export_query_metrics_filepath
         *                              "export_query_metrics_filepath": Path
         *                              to export target specified as a
         *                              filename or existing directory.
         *                          <li>@ref gpudb::export_query_metrics_format
         *                              "export_query_metrics_format":
         *                              Specifies which format to export the
         *                              metrics.
         *                              Supported values:
         *                              <ul>
         *                                  <li>@ref
         *                                      gpudb::export_query_metrics_json
         *                                      "export_query_metrics_json":
         *                                      Generic json output
         *                                  <li>@ref
         *                                      gpudb::export_query_metrics_json_trace_event
         *                                      "export_query_metrics_json_trace_event":
         *                                      Chromium/Perfetto trace event
         *                                      format
         *                              </ul>
         *                              The default value is @ref
         *                              gpudb::export_query_metrics_json
         *                              "export_query_metrics_json".
         *                          <li>@ref gpudb::export_query_metrics_job_id
         *                              "export_query_metrics_job_id": Export
         *                              query metrics for the currently running
         *                              job
         *                          <li>@ref gpudb::export_query_metrics_limit
         *                              "export_query_metrics_limit": Record
         *                              limit per file for multi query export
         *                      </ul>
         *                      The default value is an empty map.
         */
        ExportQueryMetricsRequest(const std::map<std::string, std::string>& options_):
            options( options_ )
        {
        }

        /**
         * Optional parameters.
         * <ul>
         *     <li>@ref gpudb::export_query_metrics_expression
         *         "export_query_metrics_expression": Filter for multi query
         *         export
         *     <li>@ref gpudb::export_query_metrics_filepath
         *         "export_query_metrics_filepath": Path to export target
         *         specified as a filename or existing directory.
         *     <li>@ref gpudb::export_query_metrics_format
         *         "export_query_metrics_format": Specifies which format to
         *         export the metrics.
         *         Supported values:
         *         <ul>
         *             <li>@ref gpudb::export_query_metrics_json
         *                 "export_query_metrics_json": Generic json output
         *             <li>@ref gpudb::export_query_metrics_json_trace_event
         *                 "export_query_metrics_json_trace_event":
         *                 Chromium/Perfetto trace event format
         *         </ul>
         *         The default value is @ref gpudb::export_query_metrics_json
         *         "export_query_metrics_json".
         *     <li>@ref gpudb::export_query_metrics_job_id
         *         "export_query_metrics_job_id": Export query metrics for the
         *         currently running job
         *     <li>@ref gpudb::export_query_metrics_limit
         *         "export_query_metrics_limit": Record limit per file for
         *         multi query export
         * </ul>
         * The default value is an empty map.
         */
        std::map<std::string, std::string> options;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::ExportQueryMetricsRequest>
    {
        static void encode(Encoder& e, const gpudb::ExportQueryMetricsRequest& v)
        {
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::ExportQueryMetricsRequest& v)
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
} // end namespace avro

namespace gpudb
{
    /**
     * A set of results returned by @ref
     * GPUdb::exportQueryMetrics(const ExportQueryMetricsRequest&) const
     * "GPUdb::exportQueryMetrics".
     */
    struct ExportQueryMetricsResponse
    {
        /**
         * Constructs an ExportQueryMetricsResponse object with default
         * parameters.
         */
        ExportQueryMetricsResponse() :
            info(std::map<std::string, std::string>())
        {
        }

        /**
         * Additional information.
         * <ul>
         *     <li>@ref gpudb::export_query_metrics_exported_files
         *         "export_query_metrics_exported_files": Comma separated list
         *         of filenames exported if applicable
         *     <li>@ref gpudb::export_query_metrics_output
         *         "export_query_metrics_output": Exported metrics if no other
         *         destination specified
         * </ul>
         * The default value is an empty map.
         */
        std::map<std::string, std::string> info;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::ExportQueryMetricsResponse>
    {
        static void encode(Encoder& e, const gpudb::ExportQueryMetricsResponse& v)
        {
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::ExportQueryMetricsResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.info);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.info);
            }
        }
    };
} // end namespace avro

#endif // __EXPORT_QUERY_METRICS_H__
