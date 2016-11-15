//
// IResearch search engine 
// 
// Copyright � 2016 by EMC Corporation, All Rights Reserved
// 
// This software contains the intellectual property of EMC Corporation or is licensed to
// EMC Corporation from third parties. Use of this software and the intellectual property
// contained therein is expressly limited to the terms and conditions of the License
// Agreement under which it is provided by or on behalf of EMC.
// 

#ifndef IRESEARCH_FORMAT_H
#define IRESEARCH_FORMAT_H

#include "shared.hpp"
#include "store/directory.hpp"

#include "index/index_meta.hpp"
#include "index/iterators.hpp"

#include "utils/block_pool.hpp"
#include "utils/io_utils.hpp"
#include "utils/string.hpp"
#include "utils/attributes_provider.hpp"

NS_ROOT

struct serializer;
struct segment_meta;
class fields_meta;
class columns_meta;
struct field_meta;
struct column_meta;
struct flush_state;
struct reader_state;
struct data_output;
struct index_output;
struct data_input;
struct index_input;
typedef std::unordered_set<doc_id_t> document_mask;

/* -------------------------------------------------------------------
 * postings_writer
 * ------------------------------------------------------------------*/

struct IRESEARCH_API postings_writer : util::const_attributes_provider {
  DECLARE_PTR( postings_writer );
  DECLARE_FACTORY( postings_writer );

  virtual ~postings_writer();
  /* out - corresponding terms utils/utstream */
  virtual void prepare( index_output& out, const flush_state& state ) = 0;  
  virtual void begin_field(const flags& features) = 0;
  virtual void write(doc_iterator& docs, iresearch::attributes& out ) = 0;
  virtual void begin_block() = 0;
  virtual void encode( data_output& out, const iresearch::attributes& attrs ) = 0;
  virtual void end() = 0;

  virtual const iresearch::attributes& attributes() const override {
    return iresearch::attributes::empty_instance();
  }
};

/* -------------------------------------------------------------------
 * field_writer
 * ------------------------------------------------------------------*/

struct IRESEARCH_API field_writer {
  DECLARE_PTR(field_writer);
  DECLARE_FACTORY(field_writer);

  virtual ~field_writer();
  virtual void prepare(const flush_state& state) = 0;
  virtual void write(field_id id, const flags& features, term_iterator& data) = 0;
  virtual void end() = 0;
};

/* -------------------------------------------------------------------
 * postings_reader
 * ------------------------------------------------------------------*/

struct IRESEARCH_API postings_reader {
  DECLARE_PTR( postings_reader );
  DECLARE_FACTORY( postings_reader);

  virtual ~postings_reader();
  
  /* in - corresponding terms stream */
  virtual void prepare( index_input& in, const reader_state& state ) = 0;

  /* parses input stream "in" and populate "attrs" collection
   * with attributes */
  virtual void decode(
    data_input& in, 
    const flags& features,
    attributes& attrs
  ) = 0;

  virtual doc_iterator::ptr iterator( 
    const flags& field,
    const attributes& attrs,
    const flags& features 
  ) = 0;
};

/* -------------------------------------------------------------------
 * term_reader
 * ------------------------------------------------------------------*/

struct IRESEARCH_API term_reader {
  DECLARE_PTR( term_reader );
  DECLARE_FACTORY( term_reader );

  virtual ~term_reader();

  virtual seek_term_iterator::ptr iterator() const = 0;

  virtual const flags& features() const = 0;

  // total number of terms
  virtual size_t size() const = 0;

  // total number of documents
  virtual uint64_t docs_count() const = 0;

  // less significant term
  virtual const bytes_ref& (min)() const = 0;

  // most significant term
  virtual const bytes_ref& (max)() const = 0;
};

/* -------------------------------------------------------------------
 * field_reader
 * ------------------------------------------------------------------*/

struct IRESEARCH_API field_reader {
  DECLARE_PTR(field_reader);
  DECLARE_FACTORY(field_reader);

  virtual ~field_reader();

  virtual void prepare(const reader_state& state) = 0;
  virtual const term_reader* terms(field_id field) const = 0;
  virtual size_t size() const = 0;
};

/* -------------------------------------------------------------------
 * field_meta_writer
 * ------------------------------------------------------------------*/

struct directory;

struct IRESEARCH_API field_meta_writer {
  DECLARE_PTR(field_meta_writer);
  DECLARE_FACTORY(field_meta_writer);

  virtual ~field_meta_writer();

  virtual void prepare(const flush_state& state) = 0;
  virtual void write(field_id id, const std::string& name, const flags& features) = 0;
  virtual void end() = 0;
};

/* -------------------------------------------------------------------
 * field_meta_reader
 * ------------------------------------------------------------------*/

struct IRESEARCH_API field_meta_reader {
  DECLARE_PTR( field_meta_reader );
  DECLARE_FACTORY( field_meta_reader );

  virtual ~field_meta_reader();

  virtual void prepare(const directory& dir, const string_ref& seg_name) = 0;

  virtual size_t begin() = 0;
  virtual void read( field_meta& meta ) = 0;
  virtual void end() = 0;
};

/* -------------------------------------------------------------------
 * stored_fields_writer
 * ------------------------------------------------------------------*/

struct index_reader;

struct IRESEARCH_API stored_fields_writer {
  DECLARE_PTR(stored_fields_writer);
  DECLARE_FACTORY(stored_fields_writer);

  virtual ~stored_fields_writer();
  virtual void prepare(directory& dir, const string_ref& seg_name) = 0;
  virtual bool write(const serializer& body) = 0;
  virtual void end(const serializer* header) = 0;
  virtual void finish() = 0;
  virtual void reset() = 0;
};

/* -------------------------------------------------------------------
* stored_fields_reader
* ------------------------------------------------------------------*/


struct IRESEARCH_API stored_fields_reader {
  DECLARE_PTR(stored_fields_reader);
  DECLARE_FACTORY(stored_fields_reader);

  typedef std::function<bool(
    data_input& /* header */, 
    data_input& /* body */
  )> visitor_f;
  
  virtual ~stored_fields_reader();

  virtual void prepare(const reader_state& state) = 0;
  virtual bool visit(doc_id_t doc, const visitor_f& visitor) = 0;
};

// -----------------------------------------------------------------------------
// --SECTION--                                                    columns_writer 
// -----------------------------------------------------------------------------

struct IRESEARCH_API columnstore_writer {
  DECLARE_SPTR(columnstore_writer);

  typedef std::function<bool(doc_id_t, const serializer&)> values_writer_f;
  typedef std::pair<field_id, values_writer_f> column_t;

  virtual ~columnstore_writer();

  virtual bool prepare(directory& dir, const string_ref& filename) = 0;
  virtual column_t push_column() = 0;
  virtual void flush() = 0;
}; // columnstore_writer

// -----------------------------------------------------------------------------
// --SECTION--                                                column_meta_writer 
// -----------------------------------------------------------------------------

struct IRESEARCH_API column_meta_writer {
  DECLARE_SPTR(column_meta_writer);
  
  virtual ~column_meta_writer();
  
  virtual bool prepare(directory& dir, const string_ref& filename) = 0;
  virtual void write(const std::string& name, field_id id) = 0;
  virtual void flush() = 0;
}; // column_meta_writer 

// -----------------------------------------------------------------------------
// --SECTION--                                                column_meta_reader
// -----------------------------------------------------------------------------

struct IRESEARCH_API column_meta_reader {
  DECLARE_SPTR(column_meta_reader);
  
  virtual ~column_meta_reader();
  
  virtual bool prepare(const directory& dir, const string_ref& seg_name) = 0;
  // returns false if there is no more data to read
  virtual bool read(column_meta& column) = 0;
}; // column_meta_reader 

// -----------------------------------------------------------------------------
// --SECTION--                                                    columns_writer 
// -----------------------------------------------------------------------------

struct IRESEARCH_API columnstore_reader {
  DECLARE_PTR(columnstore_reader);

  typedef std::function<bool(data_input&)> value_reader_f;
  typedef std::function<bool(doc_id_t, data_input&)> raw_reader_f;
  typedef std::function<bool(doc_id_t, const value_reader_f&)> values_reader_f;

  virtual ~columnstore_reader();

  virtual bool prepare(const reader_state& state) = 0;
  virtual values_reader_f values(field_id field) const = 0;
  virtual bool visit(field_id field, const raw_reader_f& visitor) const = 0;
}; // columnstore_reader

/* -------------------------------------------------------------------
 * document_mask_writer
 * ------------------------------------------------------------------*/

struct IRESEARCH_API document_mask_writer {
  DECLARE_PTR(document_mask_writer);
  DECLARE_FACTORY(document_mask_writer);

  virtual ~document_mask_writer();
  virtual std::string filename(const segment_meta& meta) const = 0;
  virtual void prepare(directory& dir, const segment_meta& meta) = 0;
  virtual void begin(uint32_t count) = 0;
  virtual void write(const doc_id_t& mask) = 0;
  virtual void end() = 0;
};

/* -------------------------------------------------------------------
 * document_mask_reader
 * ------------------------------------------------------------------*/

struct IRESEARCH_API document_mask_reader {
  DECLARE_PTR(document_mask_reader);
  DECLARE_FACTORY(document_mask_reader);

  virtual ~document_mask_reader();

  // @return success
  virtual bool prepare(const directory& dir, const segment_meta& meta) = 0;
  virtual uint32_t begin() = 0;
  virtual void read(doc_id_t& mask) = 0;
  virtual void end() = 0;
};

/* -------------------------------------------------------------------
 * segment_meta_writer
 * ------------------------------------------------------------------*/

struct IRESEARCH_API segment_meta_writer {
  DECLARE_SPTR( segment_meta_writer );
  DECLARE_FACTORY( segment_meta_writer );

  virtual ~segment_meta_writer();
  virtual std::string filename(const segment_meta& meta) const = 0;
  virtual void write(directory& dir, const segment_meta& meta) = 0;
};

/* -------------------------------------------------------------------
 * segment_meta_reader
 * ------------------------------------------------------------------*/

struct IRESEARCH_API segment_meta_reader {
  DECLARE_SPTR( segment_meta_reader );
  DECLARE_FACTORY( segment_meta_reader );

  virtual ~segment_meta_reader();

  virtual void read(
    const directory& dir,
    segment_meta& meta,
    const string_ref& filename = string_ref::nil // null == use meta
  ) = 0;
};

/* -------------------------------------------------------------------
 * index_meta_writer
 * ------------------------------------------------------------------*/

struct IRESEARCH_API index_meta_writer {
  DECLARE_SPTR(index_meta_writer);
  DECLARE_FACTORY(index_meta_writer);

  virtual ~index_meta_writer();
  virtual std::string filename(const index_meta& meta) const = 0;
  virtual bool prepare(directory& dir, index_meta& meta) = 0;
  virtual void commit() = 0;
  virtual void rollback() NOEXCEPT = 0;
 protected:
   static void complete(index_meta& meta) NOEXCEPT;
   static void prepare(index_meta& meta);
};

/* -------------------------------------------------------------------
 * index_meta_reader
 * ------------------------------------------------------------------*/

struct IRESEARCH_API index_meta_reader {
  DECLARE_SPTR(index_meta_reader);
  DECLARE_FACTORY(index_meta_reader);

  virtual ~index_meta_reader();

  virtual bool index_exists(const directory::files& files) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// Returns a pointer to one of the files in the specified "files" collection.
  /// If no index segments file could be found then returns nullptr.
  ///
  /// In order to use the returned pointer correctly
  /// the "files" collection should be alive at the
  /// moment of use since it is just a reference to
  /// an entry in the specified "files" collection.
  ////////////////////////////////////////////////////////////////////////////////
  virtual const std::string* last_segments_file(
    const directory::files& files
  ) = 0;

  virtual void read(
    const directory& dir, 
    index_meta& meta,
    const string_ref& filename = string_ref::nil // null == use meta
  ) = 0;

 protected:
  static void complete(
    index_meta& meta,
    uint64_t generation,
    uint64_t counter,
    index_meta::index_segments_t&& segments
  );
};

/* -------------------------------------------------------------------
 * format
 * ------------------------------------------------------------------*/

class IRESEARCH_API format {
 public:
  DECLARE_SPTR(format);

  //////////////////////////////////////////////////////////////////////////////
  /// @class type_id
  //////////////////////////////////////////////////////////////////////////////
  class type_id: public iresearch::type_id, util::noncopyable {
   public:
    type_id(const string_ref& name): name_(name) {}
    operator const type_id*() const { return this; }
    const string_ref& name() const { return name_; }

   private:
    string_ref name_;
  };

  format(const type_id& type): type_(&type) {}
  virtual ~format();

  virtual index_meta_writer::ptr get_index_meta_writer() const = 0;
  virtual index_meta_reader::ptr get_index_meta_reader() const = 0;

  virtual segment_meta_writer::ptr get_segment_meta_writer() const = 0;
  virtual segment_meta_reader::ptr get_segment_meta_reader() const = 0;

  virtual document_mask_writer::ptr get_document_mask_writer() const = 0;
  virtual document_mask_reader::ptr get_document_mask_reader() const = 0;

  virtual field_meta_writer::ptr get_field_meta_writer() const = 0;
  virtual field_meta_reader::ptr get_field_meta_reader() const = 0;

  virtual field_writer::ptr get_field_writer(bool volatile_attributes = false) const = 0;
  virtual field_reader::ptr get_field_reader() const = 0;

  virtual stored_fields_writer::ptr get_stored_fields_writer() const = 0;
  virtual stored_fields_reader::ptr get_stored_fields_reader() const = 0;
  
  virtual column_meta_writer::ptr get_column_meta_writer() const = 0;
  virtual column_meta_reader::ptr get_column_meta_reader() const = 0;

  virtual columnstore_writer::ptr get_columnstore_writer() const = 0;
  virtual columnstore_reader::ptr get_columnstore_reader() const = 0;

  const type_id& type() const { return *type_; }

 private:
  const type_id* type_;
};

struct IRESEARCH_API flush_state {
  directory* dir;
  string_ref name; // segment name
  const flags* features; // segment features
  size_t fields_count;
  size_t doc_count;
  int ver;
};

struct IRESEARCH_API reader_state {
  const format* codec;
  const directory* dir;
  const document_mask* docs_mask;
  const fields_meta* fields;
  const segment_meta* meta;
};

// -----------------------------------------------------------------------------
// --SECTION--                                               convinience methods
// -----------------------------------------------------------------------------

class IRESEARCH_API formats {
 public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief find a format by name, or nullptr if not found
  ///        indirect call to <class>::make(...)
  ///        requires use of DECLARE_FACTORY_DEFAULT() in class definition
  ///        NOTE: make(...) MUST be defined in CPP to ensire proper code scope
  //////////////////////////////////////////////////////////////////////////////
  static format::ptr get(const string_ref& name);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief for static lib reference all known formats in lib
  ///        for shared lib NOOP
  ///        no explicit call of fn is required, existence of fn is sufficient
  ////////////////////////////////////////////////////////////////////////////////
  static void init();

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief load all formats from plugins directory
  ////////////////////////////////////////////////////////////////////////////////
  static void load_all(const std::string& path);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief visit all loaded formats, terminate early if visitor returns false
  ////////////////////////////////////////////////////////////////////////////////
  static bool visit(const std::function<bool(const string_ref&)>& visitor);

 private:
  formats() = delete;
};

// -----------------------------------------------------------------------------
// --SECTION--                                                 format definition
// -----------------------------------------------------------------------------

#define DECLARE_FORMAT_TYPE() DECLARE_TYPE_ID(iresearch::format::type_id)
#define DEFINE_FORMAT_TYPE_NAMED(class_type, class_name) DEFINE_TYPE_ID(class_type, iresearch::format::type_id) { \
  static iresearch::format::type_id type(class_name); \
  return type; \
}
#define DEFINE_FORMAT_TYPE(class_type) DEFINE_FORMAT_TYPE_NAMED(class_type, #class_type)

// -----------------------------------------------------------------------------
// --SECTION--                                               format registration
// -----------------------------------------------------------------------------

class IRESEARCH_API format_registrar {
 public:
   format_registrar(const format::type_id& type, format::ptr(*factory)());
};

#define REGISTER_FORMAT__(format_name, line) static iresearch::format_registrar format_registrar ## _ ## line(format_name::type(), &format_name::make)
#define REGISTER_FORMAT_EXPANDER__(format_name, line) REGISTER_FORMAT__(format_name, line)
#define REGISTER_FORMAT(format_name) REGISTER_FORMAT_EXPANDER__(format_name, __LINE__)

NS_END

#endif
