/*
 *  This file was autogenerated by the GPUdb schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __RESERVE_RESOURCE_H__
#define __RESERVE_RESOURCE_H__

namespace gpudb
{

    /**
     * A set of input parameters for {@link
     * #reserveResource(const ReserveResourceRequest&) const}.
     * <p>
     * @private
     */
    struct ReserveResourceRequest
    {

        /**
         * @private
         * Constructs a ReserveResourceRequest object with default parameter
         * values.
         */
        ReserveResourceRequest() :
            component(std::string()),
            name(std::string()),
            action(std::string()),
            bytesRequested(int64_t()),
            ownerId(int64_t()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * @private
         * Constructs a ReserveResourceRequest object with the specified
         * parameters.
         * 
         * @param[in] component_
         * @param[in] name_
         * @param[in] action_
         *                     <ul>
         *                             <li> gpudb::reserve_resource_get_size
         *                             <li>
         *                     gpudb::reserve_resource_notify_untiered
         *                             <li> gpudb::reserve_resource_tier
         *                             <li> gpudb::reserve_resource_evict
         *                             <li> gpudb::reserve_resource_delete
         *                             <li>
         *                     gpudb::reserve_resource_change_owner
         *                     </ul>
         * @param[in] bytesRequested_
         * @param[in] ownerId_
         * @param[in] options_
         *                      <ul>
         *                      </ul>
         * 
         */
        ReserveResourceRequest(const std::string& component_, const std::string& name_, const std::string& action_, const int64_t bytesRequested_, const int64_t ownerId_, const std::map<std::string, std::string>& options_):
            component( component_ ),
            name( name_ ),
            action( action_ ),
            bytesRequested( bytesRequested_ ),
            ownerId( ownerId_ ),
            options( options_ )
        {
        }

        std::string component;
        std::string name;
        std::string action;
        int64_t bytesRequested;
        int64_t ownerId;
        std::map<std::string, std::string> options;
    };
}

    /**
     * @private
     */

namespace avro
{
    template<> struct codec_traits<gpudb::ReserveResourceRequest>
    {
        static void encode(Encoder& e, const gpudb::ReserveResourceRequest& v)
        {
            ::avro::encode(e, v.component);
            ::avro::encode(e, v.name);
            ::avro::encode(e, v.action);
            ::avro::encode(e, v.bytesRequested);
            ::avro::encode(e, v.ownerId);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::ReserveResourceRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.component);
                            break;

                        case 1:
                            ::avro::decode(d, v.name);
                            break;

                        case 2:
                            ::avro::decode(d, v.action);
                            break;

                        case 3:
                            ::avro::decode(d, v.bytesRequested);
                            break;

                        case 4:
                            ::avro::decode(d, v.ownerId);
                            break;

                        case 5:
                            ::avro::decode(d, v.options);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.component);
                ::avro::decode(d, v.name);
                ::avro::decode(d, v.action);
                ::avro::decode(d, v.bytesRequested);
                ::avro::decode(d, v.ownerId);
                ::avro::decode(d, v.options);
            }
        }
    };
}

namespace gpudb
{

    /**
     * A set of output parameters for {@link
     * #reserveResource(const ReserveResourceRequest&) const}.
     * <p>
     * @private
     */
    struct ReserveResourceResponse
    {

        /**
         * @private
         * Constructs a ReserveResourceResponse object with default parameter
         * values.
         */
        ReserveResourceResponse() :
            component(std::string()),
            name(std::string()),
            reservation(int64_t()),
            info(std::map<std::string, std::string>())
        {
        }

        std::string component;
        std::string name;
        int64_t reservation;
        std::map<std::string, std::string> info;
    };
}

    /**
     * @private
     */

namespace avro
{
    template<> struct codec_traits<gpudb::ReserveResourceResponse>
    {
        static void encode(Encoder& e, const gpudb::ReserveResourceResponse& v)
        {
            ::avro::encode(e, v.component);
            ::avro::encode(e, v.name);
            ::avro::encode(e, v.reservation);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::ReserveResourceResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.component);
                            break;

                        case 1:
                            ::avro::decode(d, v.name);
                            break;

                        case 2:
                            ::avro::decode(d, v.reservation);
                            break;

                        case 3:
                            ::avro::decode(d, v.info);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.component);
                ::avro::decode(d, v.name);
                ::avro::decode(d, v.reservation);
                ::avro::decode(d, v.info);
            }
        }
    };
}

#endif