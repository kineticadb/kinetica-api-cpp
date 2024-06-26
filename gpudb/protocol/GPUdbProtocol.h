/*
 *  This file was autogenerated by the Kinetica schema processor.
 *
 *  DO NOT EDIT DIRECTLY.
 */
#ifndef __GPUDBPROTOCOL_H__
#define __GPUDBPROTOCOL_H__

#include <avro/Specific.hh>

#include "gpudb_wrapper.h"
#include "admin_add_host.h"
#include "admin_add_ranks.h"
#include "admin_alter_host.h"
#include "admin_alter_jobs.h"
#include "admin_backup_begin.h"
#include "admin_backup_end.h"
#include "admin_ha_refresh.h"
#include "admin_offline.h"
#include "admin_rebalance.h"
#include "admin_remove_host.h"
#include "admin_remove_ranks.h"
#include "admin_repair_table.h"
#include "admin_show_alerts.h"
#include "admin_show_cluster_operations.h"
#include "admin_show_jobs.h"
#include "admin_show_shards.h"
#include "admin_shutdown.h"
#include "admin_switchover.h"
#include "admin_verify_db.h"
#include "aggregate_convex_hull.h"
#include "aggregate_group_by.h"
#include "aggregate_histogram.h"
#include "aggregate_k_means.h"
#include "aggregate_min_max.h"
#include "aggregate_min_max_geometry.h"
#include "aggregate_statistics.h"
#include "aggregate_statistics_by_range.h"
#include "aggregate_unique.h"
#include "aggregate_unpivot.h"
#include "alter_credential.h"
#include "alter_datasink.h"
#include "alter_datasource.h"
#include "alter_directory.h"
#include "alter_environment.h"
#include "alter_graph.h"
#include "alter_model.h"
#include "alter_resource_group.h"
#include "alter_role.h"
#include "alter_schema.h"
#include "alter_system_properties.h"
#include "alter_table.h"
#include "alter_table_columns.h"
#include "alter_table_metadata.h"
#include "alter_table_monitor.h"
#include "alter_tier.h"
#include "alter_user.h"
#include "alter_video.h"
#include "alter_wal.h"
#include "append_records.h"
#include "clear_statistics.h"
#include "clear_table.h"
#include "clear_table_monitor.h"
#include "clear_trigger.h"
#include "collect_statistics.h"
#include "create_container_registry.h"
#include "create_credential.h"
#include "create_datasink.h"
#include "create_datasource.h"
#include "create_delta_table.h"
#include "create_directory.h"
#include "create_environment.h"
#include "create_graph.h"
#include "create_job.h"
#include "create_join_table.h"
#include "create_materialized_view.h"
#include "create_proc.h"
#include "create_projection.h"
#include "create_resource_group.h"
#include "create_role.h"
#include "create_schema.h"
#include "create_state_table.h"
#include "create_table.h"
#include "create_table_external.h"
#include "create_table_monitor.h"
#include "create_trigger_by_area.h"
#include "create_trigger_by_range.h"
#include "create_type.h"
#include "create_union.h"
#include "create_user_external.h"
#include "create_user_internal.h"
#include "create_video.h"
#include "delete_directory.h"
#include "delete_files.h"
#include "delete_graph.h"
#include "delete_proc.h"
#include "delete_records.h"
#include "delete_resource_group.h"
#include "delete_role.h"
#include "delete_user.h"
#include "download_files.h"
#include "drop_container_registry.h"
#include "drop_credential.h"
#include "drop_datasink.h"
#include "drop_datasource.h"
#include "drop_environment.h"
#include "drop_model.h"
#include "drop_schema.h"
#include "evaluate_model.h"
#include "execute_proc.h"
#include "execute_sql.h"
#include "export_query_metrics.h"
#include "export_records_to_files.h"
#include "export_records_to_table.h"
#include "filter.h"
#include "filter_by_area.h"
#include "filter_by_area_geometry.h"
#include "filter_by_box.h"
#include "filter_by_box_geometry.h"
#include "filter_by_geometry.h"
#include "filter_by_list.h"
#include "filter_by_radius.h"
#include "filter_by_radius_geometry.h"
#include "filter_by_range.h"
#include "filter_by_series.h"
#include "filter_by_string.h"
#include "filter_by_table.h"
#include "filter_by_value.h"
#include "get_job.h"
#include "get_records.h"
#include "get_records_by_column.h"
#include "get_records_by_series.h"
#include "get_records_from_collection.h"
#include "get_vectortile.h"
#include "grant_permission.h"
#include "grant_permission_credential.h"
#include "grant_permission_datasource.h"
#include "grant_permission_directory.h"
#include "grant_permission_proc.h"
#include "grant_permission_system.h"
#include "grant_permission_table.h"
#include "grant_role.h"
#include "has_permission.h"
#include "has_proc.h"
#include "has_role.h"
#include "has_schema.h"
#include "has_table.h"
#include "has_type.h"
#include "import_model.h"
#include "insert_records.h"
#include "insert_records_from_files.h"
#include "insert_records_from_payload.h"
#include "insert_records_from_query.h"
#include "insert_records_random.h"
#include "insert_symbol.h"
#include "kill_proc.h"
#include "list_graph.h"
#include "lock_table.h"
#include "match_graph.h"
#include "merge_records.h"
#include "modify_graph.h"
#include "query_graph.h"
#include "repartition_graph.h"
#include "reserve_resource.h"
#include "revoke_permission.h"
#include "revoke_permission_credential.h"
#include "revoke_permission_datasource.h"
#include "revoke_permission_directory.h"
#include "revoke_permission_proc.h"
#include "revoke_permission_system.h"
#include "revoke_permission_table.h"
#include "revoke_role.h"
#include "show_container_registry.h"
#include "show_credential.h"
#include "show_datasink.h"
#include "show_datasource.h"
#include "show_directories.h"
#include "show_environment.h"
#include "show_files.h"
#include "show_functions.h"
#include "show_graph.h"
#include "show_graph_grammar.h"
#include "show_model.h"
#include "show_proc.h"
#include "show_proc_status.h"
#include "show_resource_objects.h"
#include "show_resource_statistics.h"
#include "show_resource_groups.h"
#include "show_schema.h"
#include "show_security.h"
#include "show_sql_proc.h"
#include "show_statistics.h"
#include "show_system_properties.h"
#include "show_system_status.h"
#include "show_system_timing.h"
#include "show_table.h"
#include "show_table_metadata.h"
#include "show_table_monitors.h"
#include "show_tables_by_type.h"
#include "show_triggers.h"
#include "show_types.h"
#include "show_video.h"
#include "show_wal.h"
#include "solve_graph.h"
#include "update_records.h"
#include "update_records_by_series.h"
#include "upload_files.h"
#include "upload_files_fromurl.h"
#include "visualize_get_feature_info.h"
#include "visualize_image.h"
#include "visualize_image_chart.h"
#include "visualize_image_classbreak.h"
#include "visualize_image_contour.h"
#include "visualize_image_heatmap.h"
#include "visualize_image_labels.h"
#include "visualize_isochrone.h"

#endif // __GPUDBPROTOCOL_H__
