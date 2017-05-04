#include "pch.h"
#include "SqliteConnection.h"
#include "utils.h"


SqliteConnection::SqliteConnection(const std::string& path, const bool allow_ext, const int flags, const std::string& vfs)
  : pConn_(NULL) {

  // Get the underlying database connection
  int rc = sqlite3_open_v2(path.c_str(), &pConn_, flags, vfs.size() ? vfs.c_str() : NULL);
  if (rc != SQLITE_OK) {
    stop("Could not connect to database:\n%s", getException());
  }
  if (allow_ext) {
    sqlite3_enable_load_extension(pConn_, 1);
  }
}

SqliteConnection::~SqliteConnection() {
  if (is_valid()) {
    warning_once("call dbDisconnect() when finished working with a connection");
    disconnect();
  }
}

sqlite3* SqliteConnection::conn() const {
  if (!is_valid()) stop("disconnected");
  return pConn_;
}

bool SqliteConnection::is_valid() const {
  return (pConn_ != NULL);
}

std::string SqliteConnection::getException() const {
  if (is_valid())
    return std::string(sqlite3_errmsg(pConn_));
  else
    return std::string();
}

void SqliteConnection::copy_to(const SqliteConnectionPtr& pDest) {
  sqlite3_backup* backup =
    sqlite3_backup_init(pDest->conn(), "main", pConn_, "main");

  int rc = sqlite3_backup_step(backup, -1);
  if (rc != SQLITE_DONE) {
    stop("Failed to copy all data:\n%s", getException());
  }
  rc = sqlite3_backup_finish(backup);
  if (rc != SQLITE_OK) {
    stop("Could not finish copy:\n%s", getException());
  }
}

void SqliteConnection::disconnect() {
  if (!is_valid()) {
    stop("cannot disconnect twice");
  }

  sqlite3_close_v2(pConn_);
  pConn_ = NULL;
}
