/*
 *  This file was autogenerated by the GPUdb schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __DROP_ENVIRONMENT_H__
#define __DROP_ENVIRONMENT_H__

namespace gpudb
{

    /**
     * A set of input parameters for {@link
     * #dropEnvironment(const DropEnvironmentRequest&) const}.
     * <p>
     * Drop an existing <a href="../../../concepts/udf/"
     * target="_top">user-defined function</a> (UDF) environment.
     */
    struct DropEnvironmentRequest
    {

        /**
         * Constructs a DropEnvironmentRequest object with default parameter
         * values.
         */
        DropEnvironmentRequest() :
            environmentName(std::string()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs a DropEnvironmentRequest object with the specified
         * parameters.
         * 
         * @param[in] environmentName_  Name of the environment to be dropped.
         *                              Must be an existing environment.
         * @param[in] options_  Optional parameters.
         *                      <ul>
         *                              <li>
         *                      gpudb::drop_environment_no_error_if_not_exists:
         *                      If @a true and if the environment specified in
         *                      @a environmentName does not exist, no error is
         *                      returned. If @a false and if the environment
         *                      specified in @a environmentName does not exist,
         *                      then an error is returned.
         *                      <ul>
         *                              <li> gpudb::drop_environment_true
         *                              <li> gpudb::drop_environment_false
         *                      </ul>
         *                      The default value is
         *                      gpudb::drop_environment_false.
         *                      </ul>
         * 
         */
        DropEnvironmentRequest(const std::string& environmentName_, const std::map<std::string, std::string>& options_):
            environmentName( environmentName_ ),
            options( options_ )
        {
        }

        std::string environmentName;
        std::map<std::string, std::string> options;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::DropEnvironmentRequest>
    {
        static void encode(Encoder& e, const gpudb::DropEnvironmentRequest& v)
        {
            ::avro::encode(e, v.environmentName);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::DropEnvironmentRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.environmentName);
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
                ::avro::decode(d, v.environmentName);
                ::avro::decode(d, v.options);
            }
        }
    };
}

namespace gpudb
{

    /**
     * A set of output parameters for {@link
     * #dropEnvironment(const DropEnvironmentRequest&) const}.
     * <p>
     * Drop an existing <a href="../../../concepts/udf/"
     * target="_top">user-defined function</a> (UDF) environment.
     */
    struct DropEnvironmentResponse
    {

        /**
         * Constructs a DropEnvironmentResponse object with default parameter
         * values.
         */
        DropEnvironmentResponse() :
            environmentName(std::string()),
            info(std::map<std::string, std::string>())
        {
        }

        std::string environmentName;
        std::map<std::string, std::string> info;
    };
}

namespace avro
{
    template<> struct codec_traits<gpudb::DropEnvironmentResponse>
    {
        static void encode(Encoder& e, const gpudb::DropEnvironmentResponse& v)
        {
            ::avro::encode(e, v.environmentName);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::DropEnvironmentResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.environmentName);
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
                ::avro::decode(d, v.environmentName);
                ::avro::decode(d, v.info);
            }
        }
    };
}

#endif