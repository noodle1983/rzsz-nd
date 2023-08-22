// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_OSCPACKAGE_ND_H_
#define FLATBUFFERS_GENERATED_OSCPACKAGE_ND_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 5 &&
              FLATBUFFERS_VERSION_REVISION == 26,
             "Non-compatible flatbuffers version included");

namespace nd {

struct InitReq;
struct InitReqBuilder;

struct InitRsp;
struct InitRspBuilder;

struct FileInfo;
struct FileInfoBuilder;

struct FileProposeStart;
struct FileProposeStartBuilder;

struct FileInitPos;
struct FileInitPosBuilder;

struct FileContent;
struct FileContentBuilder;

struct FileComplete;
struct FileCompleteBuilder;

struct FileCompleteAck;
struct FileCompleteAckBuilder;

struct InitRecv;
struct InitRecvBuilder;

struct SetClientWorkingDir;
struct SetClientWorkingDirBuilder;

struct EmptyDir;
struct EmptyDirBuilder;

struct ErrMsg;
struct ErrMsgBuilder;

struct Heartbeat;
struct HeartbeatBuilder;

struct Bye;
struct ByeBuilder;

struct ByeBye;
struct ByeByeBuilder;

struct OscPkg;
struct OscPkgBuilder;

enum PkgType : int8_t {
  PkgType_Invalid = 0,
  PkgType_InitReq = 1,
  PkgType_InitRsp = 2,
  PkgType_FileInfo = 3,
  PkgType_FileProposeStart = 4,
  PkgType_FileInitPos = 5,
  PkgType_FileContent = 6,
  PkgType_FileComplete = 7,
  PkgType_FileCompleteAck = 8,
  PkgType_InitRecv = 9,
  PkgType_SetClientWorkingDir = 10,
  PkgType_EmptyDir = 11,
  PkgType_Heartbeat = 12,
  PkgType_Bye = 13,
  PkgType_ByeBye = 14,
  PkgType_ErrMsg = 15,
  PkgType_MIN = PkgType_Invalid,
  PkgType_MAX = PkgType_ErrMsg
};

inline const PkgType (&EnumValuesPkgType())[16] {
  static const PkgType values[] = {
    PkgType_Invalid,
    PkgType_InitReq,
    PkgType_InitRsp,
    PkgType_FileInfo,
    PkgType_FileProposeStart,
    PkgType_FileInitPos,
    PkgType_FileContent,
    PkgType_FileComplete,
    PkgType_FileCompleteAck,
    PkgType_InitRecv,
    PkgType_SetClientWorkingDir,
    PkgType_EmptyDir,
    PkgType_Heartbeat,
    PkgType_Bye,
    PkgType_ByeBye,
    PkgType_ErrMsg
  };
  return values;
}

inline const char * const *EnumNamesPkgType() {
  static const char * const names[17] = {
    "Invalid",
    "InitReq",
    "InitRsp",
    "FileInfo",
    "FileProposeStart",
    "FileInitPos",
    "FileContent",
    "FileComplete",
    "FileCompleteAck",
    "InitRecv",
    "SetClientWorkingDir",
    "EmptyDir",
    "Heartbeat",
    "Bye",
    "ByeBye",
    "ErrMsg",
    nullptr
  };
  return names;
}

inline const char *EnumNamePkgType(PkgType e) {
  if (::flatbuffers::IsOutRange(e, PkgType_Invalid, PkgType_ErrMsg)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesPkgType()[index];
}

struct InitReq FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef InitReqBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VERSION = 4
  };
  uint32_t version() const {
    return GetField<uint32_t>(VT_VERSION, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_VERSION, 4) &&
           verifier.EndTable();
  }
};

struct InitReqBuilder {
  typedef InitReq Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_version(uint32_t version) {
    fbb_.AddElement<uint32_t>(InitReq::VT_VERSION, version, 0);
  }
  explicit InitReqBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<InitReq> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<InitReq>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<InitReq> CreateInitReq(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t version = 0) {
  InitReqBuilder builder_(_fbb);
  builder_.add_version(version);
  return builder_.Finish();
}

struct InitRsp FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef InitRspBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VERSION = 4
  };
  uint32_t version() const {
    return GetField<uint32_t>(VT_VERSION, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_VERSION, 4) &&
           verifier.EndTable();
  }
};

struct InitRspBuilder {
  typedef InitRsp Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_version(uint32_t version) {
    fbb_.AddElement<uint32_t>(InitRsp::VT_VERSION, version, 0);
  }
  explicit InitRspBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<InitRsp> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<InitRsp>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<InitRsp> CreateInitRsp(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t version = 0) {
  InitRspBuilder builder_(_fbb);
  builder_.add_version(version);
  return builder_.Finish();
}

struct FileInfo FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef FileInfoBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ID = 4,
    VT_RELATIVE_PATH = 6,
    VT_FILESIZE = 8
  };
  uint32_t id() const {
    return GetField<uint32_t>(VT_ID, 0);
  }
  const ::flatbuffers::String *relative_path() const {
    return GetPointer<const ::flatbuffers::String *>(VT_RELATIVE_PATH);
  }
  uint64_t filesize() const {
    return GetField<uint64_t>(VT_FILESIZE, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_ID, 4) &&
           VerifyOffset(verifier, VT_RELATIVE_PATH) &&
           verifier.VerifyString(relative_path()) &&
           VerifyField<uint64_t>(verifier, VT_FILESIZE, 8) &&
           verifier.EndTable();
  }
};

struct FileInfoBuilder {
  typedef FileInfo Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_id(uint32_t id) {
    fbb_.AddElement<uint32_t>(FileInfo::VT_ID, id, 0);
  }
  void add_relative_path(::flatbuffers::Offset<::flatbuffers::String> relative_path) {
    fbb_.AddOffset(FileInfo::VT_RELATIVE_PATH, relative_path);
  }
  void add_filesize(uint64_t filesize) {
    fbb_.AddElement<uint64_t>(FileInfo::VT_FILESIZE, filesize, 0);
  }
  explicit FileInfoBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<FileInfo> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<FileInfo>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<FileInfo> CreateFileInfo(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t id = 0,
    ::flatbuffers::Offset<::flatbuffers::String> relative_path = 0,
    uint64_t filesize = 0) {
  FileInfoBuilder builder_(_fbb);
  builder_.add_filesize(filesize);
  builder_.add_relative_path(relative_path);
  builder_.add_id(id);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<FileInfo> CreateFileInfoDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t id = 0,
    const char *relative_path = nullptr,
    uint64_t filesize = 0) {
  auto relative_path__ = relative_path ? _fbb.CreateString(relative_path) : 0;
  return nd::CreateFileInfo(
      _fbb,
      id,
      relative_path__,
      filesize);
}

struct FileProposeStart FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef FileProposeStartBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ID = 4,
    VT_EXIST_POS = 6,
    VT_EXIST_CRC32 = 8
  };
  uint32_t id() const {
    return GetField<uint32_t>(VT_ID, 0);
  }
  uint64_t exist_pos() const {
    return GetField<uint64_t>(VT_EXIST_POS, 0);
  }
  uint32_t exist_crc32() const {
    return GetField<uint32_t>(VT_EXIST_CRC32, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_ID, 4) &&
           VerifyField<uint64_t>(verifier, VT_EXIST_POS, 8) &&
           VerifyField<uint32_t>(verifier, VT_EXIST_CRC32, 4) &&
           verifier.EndTable();
  }
};

struct FileProposeStartBuilder {
  typedef FileProposeStart Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_id(uint32_t id) {
    fbb_.AddElement<uint32_t>(FileProposeStart::VT_ID, id, 0);
  }
  void add_exist_pos(uint64_t exist_pos) {
    fbb_.AddElement<uint64_t>(FileProposeStart::VT_EXIST_POS, exist_pos, 0);
  }
  void add_exist_crc32(uint32_t exist_crc32) {
    fbb_.AddElement<uint32_t>(FileProposeStart::VT_EXIST_CRC32, exist_crc32, 0);
  }
  explicit FileProposeStartBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<FileProposeStart> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<FileProposeStart>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<FileProposeStart> CreateFileProposeStart(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t id = 0,
    uint64_t exist_pos = 0,
    uint32_t exist_crc32 = 0) {
  FileProposeStartBuilder builder_(_fbb);
  builder_.add_exist_pos(exist_pos);
  builder_.add_exist_crc32(exist_crc32);
  builder_.add_id(id);
  return builder_.Finish();
}

struct FileInitPos FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef FileInitPosBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ID = 4,
    VT_OFFSET = 6
  };
  uint32_t id() const {
    return GetField<uint32_t>(VT_ID, 0);
  }
  uint64_t offset() const {
    return GetField<uint64_t>(VT_OFFSET, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_ID, 4) &&
           VerifyField<uint64_t>(verifier, VT_OFFSET, 8) &&
           verifier.EndTable();
  }
};

struct FileInitPosBuilder {
  typedef FileInitPos Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_id(uint32_t id) {
    fbb_.AddElement<uint32_t>(FileInitPos::VT_ID, id, 0);
  }
  void add_offset(uint64_t offset) {
    fbb_.AddElement<uint64_t>(FileInitPos::VT_OFFSET, offset, 0);
  }
  explicit FileInitPosBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<FileInitPos> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<FileInitPos>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<FileInitPos> CreateFileInitPos(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t id = 0,
    uint64_t offset = 0) {
  FileInitPosBuilder builder_(_fbb);
  builder_.add_offset(offset);
  builder_.add_id(id);
  return builder_.Finish();
}

struct FileContent FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef FileContentBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ID = 4,
    VT_OFFSET = 6,
    VT_CRC32 = 8,
    VT_CONTENT = 10
  };
  uint32_t id() const {
    return GetField<uint32_t>(VT_ID, 0);
  }
  uint64_t offset() const {
    return GetField<uint64_t>(VT_OFFSET, 0);
  }
  uint32_t crc32() const {
    return GetField<uint32_t>(VT_CRC32, 0);
  }
  const ::flatbuffers::String *content() const {
    return GetPointer<const ::flatbuffers::String *>(VT_CONTENT);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_ID, 4) &&
           VerifyField<uint64_t>(verifier, VT_OFFSET, 8) &&
           VerifyField<uint32_t>(verifier, VT_CRC32, 4) &&
           VerifyOffset(verifier, VT_CONTENT) &&
           verifier.VerifyString(content()) &&
           verifier.EndTable();
  }
};

struct FileContentBuilder {
  typedef FileContent Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_id(uint32_t id) {
    fbb_.AddElement<uint32_t>(FileContent::VT_ID, id, 0);
  }
  void add_offset(uint64_t offset) {
    fbb_.AddElement<uint64_t>(FileContent::VT_OFFSET, offset, 0);
  }
  void add_crc32(uint32_t crc32) {
    fbb_.AddElement<uint32_t>(FileContent::VT_CRC32, crc32, 0);
  }
  void add_content(::flatbuffers::Offset<::flatbuffers::String> content) {
    fbb_.AddOffset(FileContent::VT_CONTENT, content);
  }
  explicit FileContentBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<FileContent> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<FileContent>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<FileContent> CreateFileContent(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t id = 0,
    uint64_t offset = 0,
    uint32_t crc32 = 0,
    ::flatbuffers::Offset<::flatbuffers::String> content = 0) {
  FileContentBuilder builder_(_fbb);
  builder_.add_offset(offset);
  builder_.add_content(content);
  builder_.add_crc32(crc32);
  builder_.add_id(id);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<FileContent> CreateFileContentDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t id = 0,
    uint64_t offset = 0,
    uint32_t crc32 = 0,
    const char *content = nullptr) {
  auto content__ = content ? _fbb.CreateString(content) : 0;
  return nd::CreateFileContent(
      _fbb,
      id,
      offset,
      crc32,
      content__);
}

struct FileComplete FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef FileCompleteBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ID = 4,
    VT_FILESIZE = 6
  };
  uint32_t id() const {
    return GetField<uint32_t>(VT_ID, 0);
  }
  uint64_t filesize() const {
    return GetField<uint64_t>(VT_FILESIZE, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_ID, 4) &&
           VerifyField<uint64_t>(verifier, VT_FILESIZE, 8) &&
           verifier.EndTable();
  }
};

struct FileCompleteBuilder {
  typedef FileComplete Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_id(uint32_t id) {
    fbb_.AddElement<uint32_t>(FileComplete::VT_ID, id, 0);
  }
  void add_filesize(uint64_t filesize) {
    fbb_.AddElement<uint64_t>(FileComplete::VT_FILESIZE, filesize, 0);
  }
  explicit FileCompleteBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<FileComplete> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<FileComplete>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<FileComplete> CreateFileComplete(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t id = 0,
    uint64_t filesize = 0) {
  FileCompleteBuilder builder_(_fbb);
  builder_.add_filesize(filesize);
  builder_.add_id(id);
  return builder_.Finish();
}

struct FileCompleteAck FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef FileCompleteAckBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ID = 4
  };
  uint32_t id() const {
    return GetField<uint32_t>(VT_ID, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_ID, 4) &&
           verifier.EndTable();
  }
};

struct FileCompleteAckBuilder {
  typedef FileCompleteAck Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_id(uint32_t id) {
    fbb_.AddElement<uint32_t>(FileCompleteAck::VT_ID, id, 0);
  }
  explicit FileCompleteAckBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<FileCompleteAck> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<FileCompleteAck>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<FileCompleteAck> CreateFileCompleteAck(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t id = 0) {
  FileCompleteAckBuilder builder_(_fbb);
  builder_.add_id(id);
  return builder_.Finish();
}

struct InitRecv FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef InitRecvBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_IS_DIR_MODE = 4,
    VT_PRESET_FILES = 6
  };
  bool is_dir_mode() const {
    return GetField<uint8_t>(VT_IS_DIR_MODE, 0) != 0;
  }
  const ::flatbuffers::String *preset_files() const {
    return GetPointer<const ::flatbuffers::String *>(VT_PRESET_FILES);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_IS_DIR_MODE, 1) &&
           VerifyOffset(verifier, VT_PRESET_FILES) &&
           verifier.VerifyString(preset_files()) &&
           verifier.EndTable();
  }
};

struct InitRecvBuilder {
  typedef InitRecv Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_is_dir_mode(bool is_dir_mode) {
    fbb_.AddElement<uint8_t>(InitRecv::VT_IS_DIR_MODE, static_cast<uint8_t>(is_dir_mode), 0);
  }
  void add_preset_files(::flatbuffers::Offset<::flatbuffers::String> preset_files) {
    fbb_.AddOffset(InitRecv::VT_PRESET_FILES, preset_files);
  }
  explicit InitRecvBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<InitRecv> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<InitRecv>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<InitRecv> CreateInitRecv(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    bool is_dir_mode = false,
    ::flatbuffers::Offset<::flatbuffers::String> preset_files = 0) {
  InitRecvBuilder builder_(_fbb);
  builder_.add_preset_files(preset_files);
  builder_.add_is_dir_mode(is_dir_mode);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<InitRecv> CreateInitRecvDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    bool is_dir_mode = false,
    const char *preset_files = nullptr) {
  auto preset_files__ = preset_files ? _fbb.CreateString(preset_files) : 0;
  return nd::CreateInitRecv(
      _fbb,
      is_dir_mode,
      preset_files__);
}

struct SetClientWorkingDir FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef SetClientWorkingDirBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_WORKING_DIR = 4
  };
  const ::flatbuffers::String *working_dir() const {
    return GetPointer<const ::flatbuffers::String *>(VT_WORKING_DIR);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_WORKING_DIR) &&
           verifier.VerifyString(working_dir()) &&
           verifier.EndTable();
  }
};

struct SetClientWorkingDirBuilder {
  typedef SetClientWorkingDir Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_working_dir(::flatbuffers::Offset<::flatbuffers::String> working_dir) {
    fbb_.AddOffset(SetClientWorkingDir::VT_WORKING_DIR, working_dir);
  }
  explicit SetClientWorkingDirBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<SetClientWorkingDir> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<SetClientWorkingDir>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<SetClientWorkingDir> CreateSetClientWorkingDir(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> working_dir = 0) {
  SetClientWorkingDirBuilder builder_(_fbb);
  builder_.add_working_dir(working_dir);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<SetClientWorkingDir> CreateSetClientWorkingDirDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *working_dir = nullptr) {
  auto working_dir__ = working_dir ? _fbb.CreateString(working_dir) : 0;
  return nd::CreateSetClientWorkingDir(
      _fbb,
      working_dir__);
}

struct EmptyDir FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef EmptyDirBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_DIR = 4
  };
  const ::flatbuffers::String *dir() const {
    return GetPointer<const ::flatbuffers::String *>(VT_DIR);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_DIR) &&
           verifier.VerifyString(dir()) &&
           verifier.EndTable();
  }
};

struct EmptyDirBuilder {
  typedef EmptyDir Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_dir(::flatbuffers::Offset<::flatbuffers::String> dir) {
    fbb_.AddOffset(EmptyDir::VT_DIR, dir);
  }
  explicit EmptyDirBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<EmptyDir> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<EmptyDir>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<EmptyDir> CreateEmptyDir(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> dir = 0) {
  EmptyDirBuilder builder_(_fbb);
  builder_.add_dir(dir);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<EmptyDir> CreateEmptyDirDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *dir = nullptr) {
  auto dir__ = dir ? _fbb.CreateString(dir) : 0;
  return nd::CreateEmptyDir(
      _fbb,
      dir__);
}

struct ErrMsg FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ErrMsgBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_MSG = 4
  };
  const ::flatbuffers::String *msg() const {
    return GetPointer<const ::flatbuffers::String *>(VT_MSG);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_MSG) &&
           verifier.VerifyString(msg()) &&
           verifier.EndTable();
  }
};

struct ErrMsgBuilder {
  typedef ErrMsg Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_msg(::flatbuffers::Offset<::flatbuffers::String> msg) {
    fbb_.AddOffset(ErrMsg::VT_MSG, msg);
  }
  explicit ErrMsgBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ErrMsg> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ErrMsg>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ErrMsg> CreateErrMsg(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> msg = 0) {
  ErrMsgBuilder builder_(_fbb);
  builder_.add_msg(msg);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ErrMsg> CreateErrMsgDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *msg = nullptr) {
  auto msg__ = msg ? _fbb.CreateString(msg) : 0;
  return nd::CreateErrMsg(
      _fbb,
      msg__);
}

struct Heartbeat FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef HeartbeatBuilder Builder;
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           verifier.EndTable();
  }
};

struct HeartbeatBuilder {
  typedef Heartbeat Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  explicit HeartbeatBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Heartbeat> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Heartbeat>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Heartbeat> CreateHeartbeat(
    ::flatbuffers::FlatBufferBuilder &_fbb) {
  HeartbeatBuilder builder_(_fbb);
  return builder_.Finish();
}

struct Bye FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ByeBuilder Builder;
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           verifier.EndTable();
  }
};

struct ByeBuilder {
  typedef Bye Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  explicit ByeBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Bye> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Bye>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Bye> CreateBye(
    ::flatbuffers::FlatBufferBuilder &_fbb) {
  ByeBuilder builder_(_fbb);
  return builder_.Finish();
}

struct ByeBye FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ByeByeBuilder Builder;
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           verifier.EndTable();
  }
};

struct ByeByeBuilder {
  typedef ByeBye Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  explicit ByeByeBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ByeBye> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ByeBye>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ByeBye> CreateByeBye(
    ::flatbuffers::FlatBufferBuilder &_fbb) {
  ByeByeBuilder builder_(_fbb);
  return builder_.Finish();
}

struct OscPkg FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef OscPkgBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_PKG_TYPE = 4,
    VT_INIT_REQ = 6,
    VT_INIT_RSP = 8,
    VT_FILE_INFO = 10,
    VT_FILE_PROPOSE_START = 12,
    VT_FILE_INIT_POS = 14,
    VT_FILE_CONTENT = 16,
    VT_FILE_COMPLETE = 18,
    VT_FILE_COMPLETE_ACK = 20,
    VT_INIT_RECV = 22,
    VT_SET_CLIENT_WORKING_DIR = 24,
    VT_EMPTY_DIR = 26,
    VT_ERR_MSG = 28,
    VT_HEARTBEAT = 30,
    VT_BYE = 32,
    VT_BYEBYE = 34
  };
  nd::PkgType pkg_type() const {
    return static_cast<nd::PkgType>(GetField<int8_t>(VT_PKG_TYPE, 0));
  }
  const nd::InitReq *init_req() const {
    return GetPointer<const nd::InitReq *>(VT_INIT_REQ);
  }
  const nd::InitRsp *init_rsp() const {
    return GetPointer<const nd::InitRsp *>(VT_INIT_RSP);
  }
  const nd::FileInfo *file_info() const {
    return GetPointer<const nd::FileInfo *>(VT_FILE_INFO);
  }
  const nd::FileProposeStart *file_propose_start() const {
    return GetPointer<const nd::FileProposeStart *>(VT_FILE_PROPOSE_START);
  }
  const nd::FileInitPos *file_init_pos() const {
    return GetPointer<const nd::FileInitPos *>(VT_FILE_INIT_POS);
  }
  const nd::FileContent *file_content() const {
    return GetPointer<const nd::FileContent *>(VT_FILE_CONTENT);
  }
  const nd::FileComplete *file_complete() const {
    return GetPointer<const nd::FileComplete *>(VT_FILE_COMPLETE);
  }
  const nd::FileCompleteAck *file_complete_ack() const {
    return GetPointer<const nd::FileCompleteAck *>(VT_FILE_COMPLETE_ACK);
  }
  const nd::InitRecv *init_recv() const {
    return GetPointer<const nd::InitRecv *>(VT_INIT_RECV);
  }
  const nd::SetClientWorkingDir *set_client_working_dir() const {
    return GetPointer<const nd::SetClientWorkingDir *>(VT_SET_CLIENT_WORKING_DIR);
  }
  const nd::EmptyDir *empty_dir() const {
    return GetPointer<const nd::EmptyDir *>(VT_EMPTY_DIR);
  }
  const nd::ErrMsg *err_msg() const {
    return GetPointer<const nd::ErrMsg *>(VT_ERR_MSG);
  }
  const nd::Heartbeat *heartbeat() const {
    return GetPointer<const nd::Heartbeat *>(VT_HEARTBEAT);
  }
  const nd::Bye *bye() const {
    return GetPointer<const nd::Bye *>(VT_BYE);
  }
  const nd::ByeBye *byebye() const {
    return GetPointer<const nd::ByeBye *>(VT_BYEBYE);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_PKG_TYPE, 1) &&
           VerifyOffset(verifier, VT_INIT_REQ) &&
           verifier.VerifyTable(init_req()) &&
           VerifyOffset(verifier, VT_INIT_RSP) &&
           verifier.VerifyTable(init_rsp()) &&
           VerifyOffset(verifier, VT_FILE_INFO) &&
           verifier.VerifyTable(file_info()) &&
           VerifyOffset(verifier, VT_FILE_PROPOSE_START) &&
           verifier.VerifyTable(file_propose_start()) &&
           VerifyOffset(verifier, VT_FILE_INIT_POS) &&
           verifier.VerifyTable(file_init_pos()) &&
           VerifyOffset(verifier, VT_FILE_CONTENT) &&
           verifier.VerifyTable(file_content()) &&
           VerifyOffset(verifier, VT_FILE_COMPLETE) &&
           verifier.VerifyTable(file_complete()) &&
           VerifyOffset(verifier, VT_FILE_COMPLETE_ACK) &&
           verifier.VerifyTable(file_complete_ack()) &&
           VerifyOffset(verifier, VT_INIT_RECV) &&
           verifier.VerifyTable(init_recv()) &&
           VerifyOffset(verifier, VT_SET_CLIENT_WORKING_DIR) &&
           verifier.VerifyTable(set_client_working_dir()) &&
           VerifyOffset(verifier, VT_EMPTY_DIR) &&
           verifier.VerifyTable(empty_dir()) &&
           VerifyOffset(verifier, VT_ERR_MSG) &&
           verifier.VerifyTable(err_msg()) &&
           VerifyOffset(verifier, VT_HEARTBEAT) &&
           verifier.VerifyTable(heartbeat()) &&
           VerifyOffset(verifier, VT_BYE) &&
           verifier.VerifyTable(bye()) &&
           VerifyOffset(verifier, VT_BYEBYE) &&
           verifier.VerifyTable(byebye()) &&
           verifier.EndTable();
  }
};

struct OscPkgBuilder {
  typedef OscPkg Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_pkg_type(nd::PkgType pkg_type) {
    fbb_.AddElement<int8_t>(OscPkg::VT_PKG_TYPE, static_cast<int8_t>(pkg_type), 0);
  }
  void add_init_req(::flatbuffers::Offset<nd::InitReq> init_req) {
    fbb_.AddOffset(OscPkg::VT_INIT_REQ, init_req);
  }
  void add_init_rsp(::flatbuffers::Offset<nd::InitRsp> init_rsp) {
    fbb_.AddOffset(OscPkg::VT_INIT_RSP, init_rsp);
  }
  void add_file_info(::flatbuffers::Offset<nd::FileInfo> file_info) {
    fbb_.AddOffset(OscPkg::VT_FILE_INFO, file_info);
  }
  void add_file_propose_start(::flatbuffers::Offset<nd::FileProposeStart> file_propose_start) {
    fbb_.AddOffset(OscPkg::VT_FILE_PROPOSE_START, file_propose_start);
  }
  void add_file_init_pos(::flatbuffers::Offset<nd::FileInitPos> file_init_pos) {
    fbb_.AddOffset(OscPkg::VT_FILE_INIT_POS, file_init_pos);
  }
  void add_file_content(::flatbuffers::Offset<nd::FileContent> file_content) {
    fbb_.AddOffset(OscPkg::VT_FILE_CONTENT, file_content);
  }
  void add_file_complete(::flatbuffers::Offset<nd::FileComplete> file_complete) {
    fbb_.AddOffset(OscPkg::VT_FILE_COMPLETE, file_complete);
  }
  void add_file_complete_ack(::flatbuffers::Offset<nd::FileCompleteAck> file_complete_ack) {
    fbb_.AddOffset(OscPkg::VT_FILE_COMPLETE_ACK, file_complete_ack);
  }
  void add_init_recv(::flatbuffers::Offset<nd::InitRecv> init_recv) {
    fbb_.AddOffset(OscPkg::VT_INIT_RECV, init_recv);
  }
  void add_set_client_working_dir(::flatbuffers::Offset<nd::SetClientWorkingDir> set_client_working_dir) {
    fbb_.AddOffset(OscPkg::VT_SET_CLIENT_WORKING_DIR, set_client_working_dir);
  }
  void add_empty_dir(::flatbuffers::Offset<nd::EmptyDir> empty_dir) {
    fbb_.AddOffset(OscPkg::VT_EMPTY_DIR, empty_dir);
  }
  void add_err_msg(::flatbuffers::Offset<nd::ErrMsg> err_msg) {
    fbb_.AddOffset(OscPkg::VT_ERR_MSG, err_msg);
  }
  void add_heartbeat(::flatbuffers::Offset<nd::Heartbeat> heartbeat) {
    fbb_.AddOffset(OscPkg::VT_HEARTBEAT, heartbeat);
  }
  void add_bye(::flatbuffers::Offset<nd::Bye> bye) {
    fbb_.AddOffset(OscPkg::VT_BYE, bye);
  }
  void add_byebye(::flatbuffers::Offset<nd::ByeBye> byebye) {
    fbb_.AddOffset(OscPkg::VT_BYEBYE, byebye);
  }
  explicit OscPkgBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<OscPkg> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<OscPkg>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<OscPkg> CreateOscPkg(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    nd::PkgType pkg_type = nd::PkgType_Invalid,
    ::flatbuffers::Offset<nd::InitReq> init_req = 0,
    ::flatbuffers::Offset<nd::InitRsp> init_rsp = 0,
    ::flatbuffers::Offset<nd::FileInfo> file_info = 0,
    ::flatbuffers::Offset<nd::FileProposeStart> file_propose_start = 0,
    ::flatbuffers::Offset<nd::FileInitPos> file_init_pos = 0,
    ::flatbuffers::Offset<nd::FileContent> file_content = 0,
    ::flatbuffers::Offset<nd::FileComplete> file_complete = 0,
    ::flatbuffers::Offset<nd::FileCompleteAck> file_complete_ack = 0,
    ::flatbuffers::Offset<nd::InitRecv> init_recv = 0,
    ::flatbuffers::Offset<nd::SetClientWorkingDir> set_client_working_dir = 0,
    ::flatbuffers::Offset<nd::EmptyDir> empty_dir = 0,
    ::flatbuffers::Offset<nd::ErrMsg> err_msg = 0,
    ::flatbuffers::Offset<nd::Heartbeat> heartbeat = 0,
    ::flatbuffers::Offset<nd::Bye> bye = 0,
    ::flatbuffers::Offset<nd::ByeBye> byebye = 0) {
  OscPkgBuilder builder_(_fbb);
  builder_.add_byebye(byebye);
  builder_.add_bye(bye);
  builder_.add_heartbeat(heartbeat);
  builder_.add_err_msg(err_msg);
  builder_.add_empty_dir(empty_dir);
  builder_.add_set_client_working_dir(set_client_working_dir);
  builder_.add_init_recv(init_recv);
  builder_.add_file_complete_ack(file_complete_ack);
  builder_.add_file_complete(file_complete);
  builder_.add_file_content(file_content);
  builder_.add_file_init_pos(file_init_pos);
  builder_.add_file_propose_start(file_propose_start);
  builder_.add_file_info(file_info);
  builder_.add_init_rsp(init_rsp);
  builder_.add_init_req(init_req);
  builder_.add_pkg_type(pkg_type);
  return builder_.Finish();
}

inline const nd::OscPkg *GetOscPkg(const void *buf) {
  return ::flatbuffers::GetRoot<nd::OscPkg>(buf);
}

inline const nd::OscPkg *GetSizePrefixedOscPkg(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<nd::OscPkg>(buf);
}

inline bool VerifyOscPkgBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<nd::OscPkg>(nullptr);
}

inline bool VerifySizePrefixedOscPkgBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<nd::OscPkg>(nullptr);
}

inline void FinishOscPkgBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<nd::OscPkg> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedOscPkgBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<nd::OscPkg> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace nd

#endif  // FLATBUFFERS_GENERATED_OSCPACKAGE_ND_H_
