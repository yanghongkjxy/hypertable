/** -*- c++ -*-
 * Copyright (C) 2008 Doug Judd (Zvents, Inc.)
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 of the
 * License, or any later version.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "Common/Compat.h"
#include "AsyncComm/CommBuf.h"
#include "AsyncComm/CommHeader.h"

#include "RangeServerProtocol.h"

namespace Hypertable {

  using namespace Serialization;

  const char *RangeServerProtocol::m_command_strings[] = {
    "load range",
    "update",
    "create scanner",
    "fetch scanblock",
    "compact",
    "status",
    "shutdown",
    "dump stats",
    "destroy scanner",
    "drop table",
    "drop range",
    "replay begin",
    "replay load range",
    "replay update",
    "replay commit",
    "get statistics",
    (const char *)0
  };

  const char *RangeServerProtocol::command_text(uint64_t command) {
    if (command < 0 || command >= COMMAND_MAX)
      return "UNKNOWN";
    return m_command_strings[command];
  }

  CommBuf *
  RangeServerProtocol::create_request_load_range(const TableIdentifier &table,
      const RangeSpec &range, const char *transfer_log,
      const RangeState &range_state) {
    CommHeader header(COMMAND_LOAD_RANGE);
    CommBuf *cbuf = new CommBuf(header, table.encoded_length()
        + range.encoded_length() + encoded_length_str16(transfer_log)
        + range_state.encoded_length());
    table.encode(cbuf->get_data_ptr_address());
    range.encode(cbuf->get_data_ptr_address());
    encode_str16(cbuf->get_data_ptr_address(), transfer_log);
    range_state.encode(cbuf->get_data_ptr_address());
    return cbuf;
  }

  CommBuf *
  RangeServerProtocol::create_request_update(const TableIdentifier &table,
      uint32_t count, StaticBuffer &buffer) {
    CommHeader header(COMMAND_UPDATE);
    CommBuf *cbuf = new CommBuf(header, 4 + table.encoded_length(), buffer);
    table.encode(cbuf->get_data_ptr_address());
    cbuf->append_i32(count);
    return cbuf;
  }

  CommBuf *RangeServerProtocol::
  create_request_create_scanner(const TableIdentifier &table,
      const RangeSpec &range, const ScanSpec &scan_spec) {
    CommHeader header(COMMAND_CREATE_SCANNER);
    CommBuf *cbuf = new CommBuf(header, table.encoded_length()
        + range.encoded_length() + scan_spec.encoded_length());
    table.encode(cbuf->get_data_ptr_address());
    range.encode(cbuf->get_data_ptr_address());
    scan_spec.encode(cbuf->get_data_ptr_address());
    return cbuf;
  }

  CommBuf *RangeServerProtocol::create_request_destroy_scanner(int scanner_id) {
    CommHeader header(COMMAND_DESTROY_SCANNER);
    header.gid = scanner_id;
    CommBuf *cbuf = new CommBuf(header, 4);
    cbuf->append_i32(scanner_id);
    return cbuf;
  }

  CommBuf *RangeServerProtocol::create_request_fetch_scanblock(int scanner_id) {
    CommHeader header(COMMAND_FETCH_SCANBLOCK);
    header.gid = scanner_id;
    CommBuf *cbuf = new CommBuf(header, 4);
    cbuf->append_i32(scanner_id);
    return cbuf;
  }

  CommBuf *
  RangeServerProtocol::create_request_drop_table(const TableIdentifier &table) {
    CommHeader header(COMMAND_DROP_TABLE);
    CommBuf *cbuf = new CommBuf(header, table.encoded_length());
    table.encode(cbuf->get_data_ptr_address());
    return cbuf;
  }

  CommBuf *RangeServerProtocol::create_request_status() {
    CommHeader header(COMMAND_STATUS);
    CommBuf *cbuf = new CommBuf(header);
    return cbuf;
  }

  CommBuf *RangeServerProtocol::create_request_shutdown() {
    CommHeader header(COMMAND_SHUTDOWN);
    CommBuf *cbuf = new CommBuf(header);
    return cbuf;
  }

  CommBuf *RangeServerProtocol::create_request_dump_stats() {
    CommHeader header(COMMAND_DUMP_STATS);
    CommBuf *cbuf = new CommBuf(header);
    return cbuf;
  }

  CommBuf *RangeServerProtocol::create_request_replay_begin(uint16_t group) {
    CommHeader header(COMMAND_REPLAY_BEGIN);
    CommBuf *cbuf = new CommBuf(header, 2);
    cbuf->append_i16(group);
    return cbuf;
  }

  CommBuf *RangeServerProtocol::
  create_request_replay_load_range(const TableIdentifier &table,
      const RangeSpec &range, const RangeState &range_state) {
    CommHeader header(COMMAND_REPLAY_LOAD_RANGE);
    CommBuf *cbuf = new CommBuf(header, table.encoded_length()
        + range.encoded_length() + range_state.encoded_length());
    table.encode(cbuf->get_data_ptr_address());
    range.encode(cbuf->get_data_ptr_address());
    range_state.encode(cbuf->get_data_ptr_address());
    return cbuf;
  }

  CommBuf *
  RangeServerProtocol::create_request_replay_update(StaticBuffer &buffer) {
    CommHeader header(COMMAND_REPLAY_UPDATE);
    CommBuf *cbuf = new CommBuf(header, 0, buffer);
    return cbuf;
  }

  CommBuf *RangeServerProtocol::create_request_replay_commit() {
    CommHeader header(COMMAND_REPLAY_COMMIT);
    CommBuf *cbuf = new CommBuf(header);
    return cbuf;
  }

  CommBuf *
  RangeServerProtocol::create_request_drop_range(const TableIdentifier &table,
                                                 const RangeSpec &range) {
    CommHeader header(COMMAND_DROP_RANGE);
    CommBuf *cbuf = new CommBuf(header, table.encoded_length()
                                + range.encoded_length());
    table.encode(cbuf->get_data_ptr_address());
    range.encode(cbuf->get_data_ptr_address());
    return cbuf;
  }

  CommBuf *RangeServerProtocol::create_request_get_statistics() {
    CommHeader header(COMMAND_GET_STATISTICS);
    CommBuf *cbuf = new CommBuf(header);
    return cbuf;
  }

} // namespace Hypertable
