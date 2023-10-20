#define DUCKDB_BUILD_LOADABLE_EXTENSION
#include "duckdb.hpp"

#include "mysql_scanner.hpp"
#include "mysql_storage.hpp"
#include "mysql_scanner_extension.hpp"

#include "duckdb/catalog/catalog.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/main/database_manager.hpp"
#include "duckdb/main/attached_database.hpp"
#include "storage/mysql_catalog.hpp"

using namespace duckdb;

static void SetMySQLDebugQueryPrint(ClientContext &context, SetScope scope, Value &parameter) {
	MySQLConnection::DebugSetPrintQueries(BooleanValue::Get(parameter));
}

static void LoadInternal(DatabaseInstance &db) {
	mysql_library_init(0, NULL, NULL);
	MySQLClearCacheFunction clear_cache_func;
	ExtensionUtil::RegisterFunction(db, clear_cache_func);

	auto &config = DBConfig::GetConfig(db);
	config.storage_extensions["mysql_scanner"] = make_uniq<MySQLStorageExtension>();

	//	config.AddExtensionOption("pg_use_binary_copy", "Whether or not to use BINARY copy to read data",
	//LogicalType::BOOLEAN, Value::BOOLEAN(true)); 	config.AddExtensionOption("pg_pages_per_task", "The amount of pages
	//per task", LogicalType::UBIGINT, Value::UBIGINT(MySQLBindData::DEFAULT_PAGES_PER_TASK));
	//	config.AddExtensionOption("pg_connection_limit", "The maximum amount of concurrent MySQL connections",
	//LogicalType::UBIGINT, Value::UBIGINT(MySQLConnectionPool::DEFAULT_MAX_CONNECTIONS), SetMySQLConnectionLimit);
	//	config.AddExtensionOption("pg_array_as_varchar", "Read MySQL arrays as varchar - enables reading mixed
	//dimensional arrays", LogicalType::BOOLEAN, Value::BOOLEAN(false));
	//	config.AddExtensionOption("pg_experimental_filter_pushdown", "Whether or not to use filter pushdown (currently
	//experimental)", LogicalType::BOOLEAN, Value::BOOLEAN(false));
	config.AddExtensionOption("mysql_debug_show_queries", "DEBUG SETTING: print all queries sent to MySQL to stdout",
	                          LogicalType::BOOLEAN, Value::BOOLEAN(false), SetMySQLDebugQueryPrint);
}

void MySQLScannerExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}

extern "C" {

DUCKDB_EXTENSION_API void mysql_scanner_init(duckdb::DatabaseInstance &db) {
	LoadInternal(db);
}

DUCKDB_EXTENSION_API const char *mysql_scanner_version() {
	return DuckDB::LibraryVersion();
}

DUCKDB_EXTENSION_API void mysql_scanner_storage_init(DBConfig &config) {
	mysql_library_init(0, NULL, NULL);
	config.storage_extensions["mysql_scanner"] = make_uniq<MySQLStorageExtension>();
}
}
