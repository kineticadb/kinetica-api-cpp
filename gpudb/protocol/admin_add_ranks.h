/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __ADMIN_ADD_RANKS_H__
#define __ADMIN_ADD_RANKS_H__

namespace gpudb
{
    /**
     * A set of parameters for @ref
     * GPUdb::adminAddRanks(const AdminAddRanksRequest&) const
     * "GPUdb::adminAddRanks".
     *
     * Add one or more ranks to an existing Kinetica cluster. The new ranks
     * will not contain any data initially (other than replicated tables) and
     * will not be assigned any shards. To rebalance data and shards across the
     * cluster, use @ref
     * GPUdb::adminRebalance(const AdminRebalanceRequest&) const
     * "GPUdb::adminRebalance".
     *
     * The database must be offline for this operation, see @ref
     * GPUdb::adminOffline(const AdminOfflineRequest&) const
     * "GPUdb::adminOffline"
     *
     * For example, if attempting to add three new ranks (two ranks on host
     * 172.123.45.67 and one rank on host 172.123.45.68) to a Kinetica cluster
     * with additional configuration parameters:
     *
     * * @ref hosts would be an array including 172.123.45.67 in the first two
     * indices (signifying two ranks being added to host 172.123.45.67) and
     * 172.123.45.68 in the last index (signifying one rank being added to host
     * 172.123.45.67)
     * * @ref configParams would be an array of maps, with each map
     * corresponding to the ranks being added in @ref hosts. The key of each
     * map would be the configuration parameter name and the value would be the
     * parameter's value, e.g. '{"rank.gpu":"1"}'
     *
     * This endpoint's processing includes copying all replicated table data to
     * the new rank(s) and therefore could take a long time. The API call may
     * time out if run directly.  It is recommended to run this endpoint
     * asynchronously via @ref GPUdb::createJob(const CreateJobRequest&) const
     * "GPUdb::createJob".
     */
    struct AdminAddRanksRequest
    {
        /**
         * Constructs an AdminAddRanksRequest object with default parameters.
         */
        AdminAddRanksRequest() :
            hosts(std::vector<std::string>()),
            configParams(std::vector<std::map<std::string, std::string> >()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs an AdminAddRanksRequest object with the specified
         * parameters.
         *
         * @param[in] hosts_  Array of host IP addresses (matching a
         *                    hostN.address from the gpudb.conf file), or host
         *                    identifiers (e.g. 'host0' from the gpudb.conf
         *                    file), on which to add ranks to the cluster. The
         *                    hosts must already be in the cluster. If needed
         *                    beforehand, to add a new host to the cluster use
         *                    @ref
         *                    GPUdb::adminAddHost(const AdminAddHostRequest&) const
         *                    "GPUdb::adminAddHost". Include the same entry as
         *                    many times as there are ranks to add to the
         *                    cluster, e.g., if two ranks on host 172.123.45.67
         *                    should be added, @a hosts_ could look like
         *                    '["172.123.45.67", "172.123.45.67"]'. All ranks
         *                    will be added simultaneously, i.e. they're not
         *                    added in the order of this array. Each entry in
         *                    this array corresponds to the entry at the same
         *                    index in the @a configParams_.
         * @param[in] configParams_  Array of maps containing configuration
         *                           parameters to apply to the new ranks found
         *                           in @a hosts_. For example,
         *                           '{"rank.gpu":"2",
         *                           "tier.ram.rank.limit":"10000000000"}'.
         *                           Currently, the available parameters are
         *                           rank-specific parameters in the <a
         *                           href="../../../config/#config-main-network"
         *                           target="_top">Network</a>, <a
         *                           href="../../../config/#config-main-hardware"
         *                           target="_top">Hardware</a>, <a
         *                           href="../../../config/#config-main-text-search"
         *                           target="_top">Text Search</a>, and <a
         *                           href="../../../config/#config-main-ram-tier"
         *                           target="_top">RAM Tiered Storage</a>
         *                           sections in the gpudb.conf file, with the
         *                           key exception of the 'rankN.host' settings
         *                           in the Network section that will be
         *                           determined by @a hosts_ instead. Though
         *                           many of these configuration parameters
         *                           typically are affixed with 'rankN' in the
         *                           gpudb.conf file (where N is the rank
         *                           number), the 'N' should be omitted in @a
         *                           configParams_ as the new rank number(s)
         *                           are not allocated until the ranks have
         *                           been added to the cluster. Each entry in
         *                           this array corresponds to the entry at the
         *                           same index in the @a hosts_. This array
         *                           must either be completely empty or have
         *                           the same number of elements as the @a
         *                           hosts_.  An empty @a configParams_ array
         *                           will result in the new ranks being set
         *                           with default parameters.
         * @param[in] options_  Optional parameters.
         *                      <ul>
         *                          <li>@ref gpudb::admin_add_ranks_dry_run
         *                              "admin_add_ranks_dry_run": If @ref
         *                              gpudb::admin_add_ranks_true "true",
         *                              only validation checks will be
         *                              performed. No ranks are added.
         *                              Supported values:
         *                              <ul>
         *                                  <li>@ref
         *                                      gpudb::admin_add_ranks_true
         *                                      "admin_add_ranks_true"
         *                                  <li>@ref
         *                                      gpudb::admin_add_ranks_false
         *                                      "admin_add_ranks_false"
         *                              </ul>
         *                              The default value is @ref
         *                              gpudb::admin_add_ranks_false
         *                              "admin_add_ranks_false".
         *                      </ul>
         *                      The default value is an empty map.
         */
        AdminAddRanksRequest(const std::vector<std::string>& hosts_, const std::vector<std::map<std::string, std::string> >& configParams_, const std::map<std::string, std::string>& options_):
            hosts( hosts_ ),
            configParams( configParams_ ),
            options( options_ )
        {
        }

        /**
         * Array of host IP addresses (matching a hostN.address from the
         * gpudb.conf file), or host identifiers (e.g.\ 'host0' from the
         * gpudb.conf file), on which to add ranks to the cluster. The hosts
         * must already be in the cluster. If needed beforehand, to add a new
         * host to the cluster use @ref
         * GPUdb::adminAddHost(const AdminAddHostRequest&) const
         * "GPUdb::adminAddHost". Include the same entry as many times as there
         * are ranks to add to the cluster, e.g., if two ranks on host
         * 172.123.45.67 should be added, @ref hosts could look like
         * '["172.123.45.67", "172.123.45.67"]'. All ranks will be added
         * simultaneously, i.e.\ they're not added in the order of this array.
         * Each entry in this array corresponds to the entry at the same index
         * in the @ref configParams.
         */
        std::vector<std::string> hosts;

        /**
         * Array of maps containing configuration parameters to apply to the
         * new ranks found in @ref hosts. For example, '{"rank.gpu":"2",
         * "tier.ram.rank.limit":"10000000000"}'. Currently, the available
         * parameters are rank-specific parameters in the <a
         * href="../../../config/#config-main-network"
         * target="_top">Network</a>, <a
         * href="../../../config/#config-main-hardware"
         * target="_top">Hardware</a>, <a
         * href="../../../config/#config-main-text-search" target="_top">Text
         * Search</a>, and <a href="../../../config/#config-main-ram-tier"
         * target="_top">RAM Tiered Storage</a> sections in the gpudb.conf
         * file, with the key exception of the 'rankN.host' settings in the
         * Network section that will be determined by @ref hosts instead.
         * Though many of these configuration parameters typically are affixed
         * with 'rankN' in the gpudb.conf file (where N is the rank number),
         * the 'N' should be omitted in @ref configParams as the new rank
         * number(s) are not allocated until the ranks have been added to the
         * cluster. Each entry in this array corresponds to the entry at the
         * same index in the @ref hosts. This array must either be completely
         * empty or have the same number of elements as the @ref hosts.  An
         * empty @ref configParams array will result in the new ranks being set
         * with default parameters.
         */
        std::vector<std::map<std::string, std::string> > configParams;

        /**
         * Optional parameters.
         * <ul>
         *     <li>@ref gpudb::admin_add_ranks_dry_run
         *         "admin_add_ranks_dry_run": If @ref
         *         gpudb::admin_add_ranks_true "true", only validation checks
         *         will be performed. No ranks are added.
         *         Supported values:
         *         <ul>
         *             <li>@ref gpudb::admin_add_ranks_true
         *                 "admin_add_ranks_true"
         *             <li>@ref gpudb::admin_add_ranks_false
         *                 "admin_add_ranks_false"
         *         </ul>
         *         The default value is @ref gpudb::admin_add_ranks_false
         *         "admin_add_ranks_false".
         * </ul>
         * The default value is an empty map.
         */
        std::map<std::string, std::string> options;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::AdminAddRanksRequest>
    {
        static void encode(Encoder& e, const gpudb::AdminAddRanksRequest& v)
        {
            ::avro::encode(e, v.hosts);
            ::avro::encode(e, v.configParams);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::AdminAddRanksRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.hosts);
                            break;

                        case 1:
                            ::avro::decode(d, v.configParams);
                            break;

                        case 2:
                            ::avro::decode(d, v.options);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.hosts);
                ::avro::decode(d, v.configParams);
                ::avro::decode(d, v.options);
            }
        }
    };
} // end namespace avro

namespace gpudb
{
    /**
     * A set of results returned by @ref
     * GPUdb::adminAddRanks(const AdminAddRanksRequest&) const
     * "GPUdb::adminAddRanks".
     */
    struct AdminAddRanksResponse
    {
        /**
         * Constructs an AdminAddRanksResponse object with default parameters.
         */
        AdminAddRanksResponse() :
            addedRanks(std::vector<std::string>()),
            info(std::map<std::string, std::string>())
        {
        }

        /**
         * The number assigned to each added rank, formatted as 'rankN', in the
         * same order as the ranks in @ref gpudb::AdminAddRanksRequest::hosts
         * "hosts" and @ref gpudb::AdminAddRanksRequest::configParams
         * "configParams".
         */
        std::vector<std::string> addedRanks;

        /**
         * Additional information.
         */
        std::map<std::string, std::string> info;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::AdminAddRanksResponse>
    {
        static void encode(Encoder& e, const gpudb::AdminAddRanksResponse& v)
        {
            ::avro::encode(e, v.addedRanks);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::AdminAddRanksResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.addedRanks);
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
                ::avro::decode(d, v.addedRanks);
                ::avro::decode(d, v.info);
            }
        }
    };
} // end namespace avro

#endif // __ADMIN_ADD_RANKS_H__
