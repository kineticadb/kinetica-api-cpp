/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __GRANT_PERMISSION_CREDENTIAL_H__
#define __GRANT_PERMISSION_CREDENTIAL_H__

namespace gpudb
{
    /**
     * A set of parameters for @ref
     * GPUdb::grantPermissionCredential(const GrantPermissionCredentialRequest&) const
     * "GPUdb::grantPermissionCredential".
     *
     * Grants a <a
     * href="../../../security/sec_concepts/#security-concepts-permissions-credential"
     * target="_top">credential-level permission</a> to a user or role.
     */
    struct GrantPermissionCredentialRequest
    {
        /**
         * Constructs a GrantPermissionCredentialRequest object with default
         * parameters.
         */
        GrantPermissionCredentialRequest() :
            name(std::string()),
            permission(std::string()),
            credentialName(std::string()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs a GrantPermissionCredentialRequest object with the
         * specified parameters.
         *
         * @param[in] name_  Name of the user or role to which the permission
         *                   will be granted. Must be an existing user or role.
         * @param[in] permission_  Permission to grant to the user or role.
         *                         Supported values:
         *                         <ul>
         *                             <li>@ref
         *                                 gpudb::grant_permission_credential_credential_admin
         *                                 "grant_permission_credential_credential_admin":
         *                                 Full read/write and administrative
         *                                 access on the credential.
         *                             <li>@ref
         *                                 gpudb::grant_permission_credential_credential_read
         *                                 "grant_permission_credential_credential_read":
         *                                 Ability to read and use the
         *                                 credential.
         *                         </ul>
         * @param[in] credentialName_  Name of the credential on which the
         *                             permission will be granted. Must be an
         *                             existing credential, or an empty string
         *                             to grant access on all credentials.
         * @param[in] options_  Optional parameters. The default value is an
         *                      empty map.
         */
        GrantPermissionCredentialRequest(const std::string& name_, const std::string& permission_, const std::string& credentialName_, const std::map<std::string, std::string>& options_):
            name( name_ ),
            permission( permission_ ),
            credentialName( credentialName_ ),
            options( options_ )
        {
        }

        /**
         * Name of the user or role to which the permission will be granted.
         * Must be an existing user or role.
         */
        std::string name;

        /**
         * Permission to grant to the user or role.
         * Supported values:
         * <ul>
         *     <li>@ref gpudb::grant_permission_credential_credential_admin
         *         "grant_permission_credential_credential_admin": Full
         *         read/write and administrative access on the credential.
         *     <li>@ref gpudb::grant_permission_credential_credential_read
         *         "grant_permission_credential_credential_read": Ability to
         *         read and use the credential.
         * </ul>
         */
        std::string permission;

        /**
         * Name of the credential on which the permission will be granted. Must
         * be an existing credential, or an empty string to grant access on all
         * credentials.
         */
        std::string credentialName;

        /**
         * Optional parameters. The default value is an empty map.
         */
        std::map<std::string, std::string> options;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::GrantPermissionCredentialRequest>
    {
        static void encode(Encoder& e, const gpudb::GrantPermissionCredentialRequest& v)
        {
            ::avro::encode(e, v.name);
            ::avro::encode(e, v.permission);
            ::avro::encode(e, v.credentialName);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::GrantPermissionCredentialRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.name);
                            break;

                        case 1:
                            ::avro::decode(d, v.permission);
                            break;

                        case 2:
                            ::avro::decode(d, v.credentialName);
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
                ::avro::decode(d, v.name);
                ::avro::decode(d, v.permission);
                ::avro::decode(d, v.credentialName);
                ::avro::decode(d, v.options);
            }
        }
    };
} // end namespace avro

namespace gpudb
{
    /**
     * A set of results returned by @ref
     * GPUdb::grantPermissionCredential(const GrantPermissionCredentialRequest&) const
     * "GPUdb::grantPermissionCredential".
     */
    struct GrantPermissionCredentialResponse
    {
        /**
         * Constructs a GrantPermissionCredentialResponse object with default
         * parameters.
         */
        GrantPermissionCredentialResponse() :
            name(std::string()),
            permission(std::string()),
            credentialName(std::string()),
            info(std::map<std::string, std::string>())
        {
        }

        /**
         * Value of @ref gpudb::GrantPermissionCredentialRequest::name "name".
         */
        std::string name;

        /**
         * Value of @ref gpudb::GrantPermissionCredentialRequest::permission
         * "permission".
         */
        std::string permission;

        /**
         * Value of @ref
         * gpudb::GrantPermissionCredentialRequest::credentialName
         * "credentialName".
         */
        std::string credentialName;

        /**
         * Additional information.
         */
        std::map<std::string, std::string> info;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::GrantPermissionCredentialResponse>
    {
        static void encode(Encoder& e, const gpudb::GrantPermissionCredentialResponse& v)
        {
            ::avro::encode(e, v.name);
            ::avro::encode(e, v.permission);
            ::avro::encode(e, v.credentialName);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::GrantPermissionCredentialResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.name);
                            break;

                        case 1:
                            ::avro::decode(d, v.permission);
                            break;

                        case 2:
                            ::avro::decode(d, v.credentialName);
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
                ::avro::decode(d, v.name);
                ::avro::decode(d, v.permission);
                ::avro::decode(d, v.credentialName);
                ::avro::decode(d, v.info);
            }
        }
    };
} // end namespace avro

#endif // __GRANT_PERMISSION_CREDENTIAL_H__
