/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __GRANT_PERMISSION_H__
#define __GRANT_PERMISSION_H__

namespace gpudb
{
    /**
     * A set of parameters for @ref
     * GPUdb::grantPermission(const GrantPermissionRequest&) const
     * "GPUdb::grantPermission".
     *
     * Grant user or role the specified permission on the specified object.
     */
    struct GrantPermissionRequest
    {
        /**
         * Constructs a GrantPermissionRequest object with default parameters.
         */
        GrantPermissionRequest() :
            principal(std::string()),
            object(std::string()),
            objectType(std::string()),
            permission(std::string()),
            options(std::map<std::string, std::string>())
        {
        }

        /**
         * Constructs a GrantPermissionRequest object with the specified
         * parameters.
         *
         * @param[in] principal_  Name of the user or role for which the
         *                        permission is being granted.  Must be an
         *                        existing user or role. The default value is
         *                        ''.
         * @param[in] object_  Name of object permission is being granted to.
         *                     It is recommended to use a fully-qualified name
         *                     when possible.
         * @param[in] objectType_  The type of object being granted to.
         *                         Supported values:
         *                         <ul>
         *                             <li>@ref gpudb::grant_permission_context
         *                                 "grant_permission_context": Context
         *                             <li>@ref
         *                                 gpudb::grant_permission_credential
         *                                 "grant_permission_credential":
         *                                 Credential
         *                             <li>@ref
         *                                 gpudb::grant_permission_datasink
         *                                 "grant_permission_datasink": Data
         *                                 Sink
         *                             <li>@ref
         *                                 gpudb::grant_permission_datasource
         *                                 "grant_permission_datasource": Data
         *                                 Source
         *                             <li>@ref
         *                                 gpudb::grant_permission_directory
         *                                 "grant_permission_directory": KIFS
         *                                 File Directory
         *                             <li>@ref gpudb::grant_permission_graph
         *                                 "grant_permission_graph": A Graph
         *                                 object
         *                             <li>@ref gpudb::grant_permission_proc
         *                                 "grant_permission_proc": UDF
         *                                 Procedure
         *                             <li>@ref gpudb::grant_permission_schema
         *                                 "grant_permission_schema": Schema
         *                             <li>@ref
         *                                 gpudb::grant_permission_sql_proc
         *                                 "grant_permission_sql_proc": SQL
         *                                 Procedure
         *                             <li>@ref gpudb::grant_permission_system
         *                                 "grant_permission_system":
         *                                 System-level access
         *                             <li>@ref gpudb::grant_permission_table
         *                                 "grant_permission_table": Database
         *                                 Table
         *                             <li>@ref
         *                                 gpudb::grant_permission_table_monitor
         *                                 "grant_permission_table_monitor":
         *                                 Table monitor
         *                         </ul>
         * @param[in] permission_  Permission being granted.
         *                         Supported values:
         *                         <ul>
         *                             <li>@ref gpudb::grant_permission_admin
         *                                 "grant_permission_admin": Full
         *                                 read/write and administrative access
         *                                 on the object.
         *                             <li>@ref gpudb::grant_permission_connect
         *                                 "grant_permission_connect": Connect
         *                                 access on the given data source or
         *                                 data sink.
         *                             <li>@ref gpudb::grant_permission_create
         *                                 "grant_permission_create": Ability
         *                                 to create new objects of this type.
         *                             <li>@ref gpudb::grant_permission_delete
         *                                 "grant_permission_delete": Delete
         *                                 rows from tables.
         *                             <li>@ref gpudb::grant_permission_execute
         *                                 "grant_permission_execute": Ability
         *                                 to Execute the Procedure object.
         *                             <li>@ref gpudb::grant_permission_insert
         *                                 "grant_permission_insert": Insert
         *                                 access to tables.
         *                             <li>@ref gpudb::grant_permission_read
         *                                 "grant_permission_read": Ability to
         *                                 read, list and use the object.
         *                             <li>@ref gpudb::grant_permission_update
         *                                 "grant_permission_update": Update
         *                                 access to the table.
         *                             <li>@ref
         *                                 gpudb::grant_permission_user_admin
         *                                 "grant_permission_user_admin":
         *                                 Access to administer users and roles
         *                                 that do not have system_admin
         *                                 permission.
         *                             <li>@ref gpudb::grant_permission_write
         *                                 "grant_permission_write": Access to
         *                                 write, change and delete objects.
         *                         </ul>
         * @param[in] options_  Optional parameters.
         *                      <ul>
         *                          <li>@ref gpudb::grant_permission_columns
         *                              "grant_permission_columns": Apply table
         *                              security to these columns,
         *                              comma-separated. The default value is
         *                              ''.
         *                          <li>@ref
         *                              gpudb::grant_permission_filter_expression
         *                              "grant_permission_filter_expression":
         *                              Optional filter expression to apply to
         *                              this grant.  Only rows that match the
         *                              filter will be affected. The default
         *                              value is ''.
         *                          <li>@ref
         *                              gpudb::grant_permission_with_grant_option
         *                              "grant_permission_with_grant_option":
         *                              Allow the recipient to grant the same
         *                              permission (or subset) to others.
         *                              Supported values:
         *                              <ul>
         *                                  <li>@ref
         *                                      gpudb::grant_permission_true
         *                                      "grant_permission_true"
         *                                  <li>@ref
         *                                      gpudb::grant_permission_false
         *                                      "grant_permission_false"
         *                              </ul>
         *                              The default value is @ref
         *                              gpudb::grant_permission_false
         *                              "grant_permission_false".
         *                      </ul>
         *                      The default value is an empty map.
         */
        GrantPermissionRequest(const std::string& principal_, const std::string& object_, const std::string& objectType_, const std::string& permission_, const std::map<std::string, std::string>& options_):
            principal( principal_ ),
            object( object_ ),
            objectType( objectType_ ),
            permission( permission_ ),
            options( options_ )
        {
        }

        /**
         * Name of the user or role for which the permission is being granted.
         * Must be an existing user or role. The default value is ''.
         */
        std::string principal;

        /**
         * Name of object permission is being granted to.  It is recommended to
         * use a fully-qualified name when possible.
         */
        std::string object;

        /**
         * The type of object being granted to.
         * Supported values:
         * <ul>
         *     <li>@ref gpudb::grant_permission_context
         *         "grant_permission_context": Context
         *     <li>@ref gpudb::grant_permission_credential
         *         "grant_permission_credential": Credential
         *     <li>@ref gpudb::grant_permission_datasink
         *         "grant_permission_datasink": Data Sink
         *     <li>@ref gpudb::grant_permission_datasource
         *         "grant_permission_datasource": Data Source
         *     <li>@ref gpudb::grant_permission_directory
         *         "grant_permission_directory": KIFS File Directory
         *     <li>@ref gpudb::grant_permission_graph "grant_permission_graph":
         *         A Graph object
         *     <li>@ref gpudb::grant_permission_proc "grant_permission_proc":
         *         UDF Procedure
         *     <li>@ref gpudb::grant_permission_schema
         *         "grant_permission_schema": Schema
         *     <li>@ref gpudb::grant_permission_sql_proc
         *         "grant_permission_sql_proc": SQL Procedure
         *     <li>@ref gpudb::grant_permission_system
         *         "grant_permission_system": System-level access
         *     <li>@ref gpudb::grant_permission_table "grant_permission_table":
         *         Database Table
         *     <li>@ref gpudb::grant_permission_table_monitor
         *         "grant_permission_table_monitor": Table monitor
         * </ul>
         */
        std::string objectType;

        /**
         * Permission being granted.
         * Supported values:
         * <ul>
         *     <li>@ref gpudb::grant_permission_admin "grant_permission_admin":
         *         Full read/write and administrative access on the object.
         *     <li>@ref gpudb::grant_permission_connect
         *         "grant_permission_connect": Connect access on the given data
         *         source or data sink.
         *     <li>@ref gpudb::grant_permission_create
         *         "grant_permission_create": Ability to create new objects of
         *         this type.
         *     <li>@ref gpudb::grant_permission_delete
         *         "grant_permission_delete": Delete rows from tables.
         *     <li>@ref gpudb::grant_permission_execute
         *         "grant_permission_execute": Ability to Execute the Procedure
         *         object.
         *     <li>@ref gpudb::grant_permission_insert
         *         "grant_permission_insert": Insert access to tables.
         *     <li>@ref gpudb::grant_permission_read "grant_permission_read":
         *         Ability to read, list and use the object.
         *     <li>@ref gpudb::grant_permission_update
         *         "grant_permission_update": Update access to the table.
         *     <li>@ref gpudb::grant_permission_user_admin
         *         "grant_permission_user_admin": Access to administer users
         *         and roles that do not have system_admin permission.
         *     <li>@ref gpudb::grant_permission_write "grant_permission_write":
         *         Access to write, change and delete objects.
         * </ul>
         */
        std::string permission;

        /**
         * Optional parameters.
         * <ul>
         *     <li>@ref gpudb::grant_permission_columns
         *         "grant_permission_columns": Apply table security to these
         *         columns, comma-separated. The default value is ''.
         *     <li>@ref gpudb::grant_permission_filter_expression
         *         "grant_permission_filter_expression": Optional filter
         *         expression to apply to this grant.  Only rows that match the
         *         filter will be affected. The default value is ''.
         *     <li>@ref gpudb::grant_permission_with_grant_option
         *         "grant_permission_with_grant_option": Allow the recipient to
         *         grant the same permission (or subset) to others.
         *         Supported values:
         *         <ul>
         *             <li>@ref gpudb::grant_permission_true
         *                 "grant_permission_true"
         *             <li>@ref gpudb::grant_permission_false
         *                 "grant_permission_false"
         *         </ul>
         *         The default value is @ref gpudb::grant_permission_false
         *         "grant_permission_false".
         * </ul>
         * The default value is an empty map.
         */
        std::map<std::string, std::string> options;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::GrantPermissionRequest>
    {
        static void encode(Encoder& e, const gpudb::GrantPermissionRequest& v)
        {
            ::avro::encode(e, v.principal);
            ::avro::encode(e, v.object);
            ::avro::encode(e, v.objectType);
            ::avro::encode(e, v.permission);
            ::avro::encode(e, v.options);
        }

        static void decode(Decoder& d, gpudb::GrantPermissionRequest& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.principal);
                            break;

                        case 1:
                            ::avro::decode(d, v.object);
                            break;

                        case 2:
                            ::avro::decode(d, v.objectType);
                            break;

                        case 3:
                            ::avro::decode(d, v.permission);
                            break;

                        case 4:
                            ::avro::decode(d, v.options);
                            break;

                        default:
                            break;
                    }
                }
            }
            else
            {
                ::avro::decode(d, v.principal);
                ::avro::decode(d, v.object);
                ::avro::decode(d, v.objectType);
                ::avro::decode(d, v.permission);
                ::avro::decode(d, v.options);
            }
        }
    };
} // end namespace avro

namespace gpudb
{
    /**
     * A set of results returned by @ref
     * GPUdb::grantPermission(const GrantPermissionRequest&) const
     * "GPUdb::grantPermission".
     */
    struct GrantPermissionResponse
    {
        /**
         * Constructs a GrantPermissionResponse object with default parameters.
         */
        GrantPermissionResponse() :
            principal(std::string()),
            object(std::string()),
            objectType(std::string()),
            permission(std::string()),
            info(std::map<std::string, std::string>())
        {
        }

        /**
         * Value of @ref gpudb::GrantPermissionRequest::principal "principal".
         */
        std::string principal;

        /**
         * Value of @ref gpudb::GrantPermissionRequest::object "object".
         */
        std::string object;

        /**
         * Value of @ref gpudb::GrantPermissionRequest::objectType
         * "objectType".
         */
        std::string objectType;

        /**
         * Value of @ref gpudb::GrantPermissionRequest::permission
         * "permission".
         */
        std::string permission;

        /**
         * Additional information.
         */
        std::map<std::string, std::string> info;
    };
} // end namespace gpudb

namespace avro
{
    template<> struct codec_traits<gpudb::GrantPermissionResponse>
    {
        static void encode(Encoder& e, const gpudb::GrantPermissionResponse& v)
        {
            ::avro::encode(e, v.principal);
            ::avro::encode(e, v.object);
            ::avro::encode(e, v.objectType);
            ::avro::encode(e, v.permission);
            ::avro::encode(e, v.info);
        }

        static void decode(Decoder& d, gpudb::GrantPermissionResponse& v)
        {
            if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
            {
                const std::vector<size_t> fo = rd->fieldOrder();

                for (std::vector<size_t>::const_iterator it = fo.begin(); it != fo.end(); ++it)
                {
                    switch (*it)
                    {
                        case 0:
                            ::avro::decode(d, v.principal);
                            break;

                        case 1:
                            ::avro::decode(d, v.object);
                            break;

                        case 2:
                            ::avro::decode(d, v.objectType);
                            break;

                        case 3:
                            ::avro::decode(d, v.permission);
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
                ::avro::decode(d, v.principal);
                ::avro::decode(d, v.object);
                ::avro::decode(d, v.objectType);
                ::avro::decode(d, v.permission);
                ::avro::decode(d, v.info);
            }
        }
    };
} // end namespace avro

#endif // __GRANT_PERMISSION_H__
