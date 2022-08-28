// Project AIDA
// Created by Long Duong on 7/5/22.
// Purpose: 
//

#ifndef AIDA_XMLSAX_H
#define AIDA_XMLSAX_H

#include <memory>
#include <functional>
#include <fmt/format.h>
#include <ostream>
#include <iostream>
#include <exception>
#include <optional>

#include <libxml2/libxml/SAX2.h>
#include <libxml2/libxml/xmlschemas.h>
#include <libxml2/libxml/xmlerror.h>
#include <libxml2/libxml/xmlIO.h>

#include <boost/mpl/string.hpp>
#include <boost/mpl/for_each.hpp>
#include "debugprint.h"

namespace common::sax {

    struct ParseException : std::exception {
        std::string msg;
        xmlErrorPtr _err;

        template <typename ...T>
        explicit ParseException(std::string_view fmt, T... args)
            : msg (fmt::vformat(fmt, fmt::make_format_args(args...))) {};
        ParseException(xmlErrorPtr err) : _err { err } {}
        virtual const char* what() const noexcept { return msg.c_str(); }
    };

    template <typename T, std::size_t MAX_NB_ATTRS = 50>
    struct XmlPushParser {
        xmlParserCtxtPtr _parser_ctx = nullptr;
        xmlSAXHandler sax_handlers;
        using KeyValuePair = std::pair<const xmlChar*, const xmlChar*>;

        XmlPushParser()
        {
            memset(&sax_handlers, 0, sizeof(sax_handlers));

            sax_handlers.initialized = XML_SAX2_MAGIC;
            // sax_handlers.startDocument = +[](void *ctx) { return std::invoke(&T::start_document, static_cast<T *>(ctx)); };
            // sax_handlers.endDocument = +[](void *ctx) { return std::invoke(&T::end_document, static_cast<T *>(ctx)); };
            sax_handlers.startElement = +[](void *ctx, const xmlChar *name, const xmlChar **attrs) { return std::invoke(&T::_start_element, static_cast<T *>(ctx), name, attrs); };
            // sax_handlers.startElementNs = +[](void *ctx, const xmlChar *local_name, const xmlChar *prefix, const xmlChar *uri, int nb_namespaces, const xmlChar **namespaces, int nb_attributes, int nb_defaulted, const xmlChar **attributes) { return std::invoke(&T::start_element_ns, static_cast<T *>(ctx), local_name, prefix, uri, nb_namespaces, namespaces, nb_attributes, nb_defaulted, attributes); };
            sax_handlers.endElement = +[](void *ctx, const xmlChar *name) { return std::invoke(&T::end_element, static_cast<T *>(ctx), name); };
            // sax_handlers.endElementNs = +[](void *ctx, const xmlChar *local_name, const xmlChar *prefix, const xmlChar *uri) { return std::invoke(&T::end_element_ns, static_cast<T *>(ctx), local_name, prefix, uri); };
            // sax_handlers.attributeDecl = +[](void *ctx, const xmlChar *elem, const xmlChar *fullname, int type, int def, const xmlChar *default_value, xmlEnumerationPtr tree) { return std::invoke(&T::attribute_decl, static_cast<T *>(ctx), elem, fullname, type, def, default_value, tree); };
            sax_handlers.cdataBlock = +[](void *ctx, const xmlChar* value, int len) { return std::invoke(&T::cdata_block, static_cast<T *>(ctx), value, len); };
            sax_handlers.characters = +[](void *ctx, const xmlChar *ch, int len) { return std::invoke(&T::characters, static_cast<T *>(ctx), ch, len); };
            // sax_handlers.comment = +[](void *ctx, const xmlChar *value) { return std::invoke(&T::comment, static_cast<T *>(ctx), value); };
            // sax_handlers.elementDecl = +[](void *ctx, const xmlChar *name, int type, xmlElementContentPtr content) { return std::invoke(&T::element_decl, static_cast<T*>(ctx), name, type, content); };
            // sax_handlers.ignorableWhitespace = +[](void* ctx, const xmlChar* ch, int len) { return std::invoke(&T::ignorable_whitespace, static_cast<T*>(ctx), ch, len); };
            sax_handlers.error = &T::error_;
            sax_handlers.warning = &T::warning_;

            _parser_ctx = xmlCreatePushParserCtxt(&sax_handlers, this, nullptr, 0, nullptr);
             xmlCtxtUseOptions(_parser_ctx, xmlParserOption::XML_PARSE_NOBLANKS);

            // This function must be set separately for each thread ==> Only 1!!! instance of this class per thread just to be safe
            xmlSetStructuredErrorFunc(this, +[](void* ctx, xmlErrorPtr err){
                if (err->level > xmlErrorLevel::XML_ERR_WARNING) std::invoke(&T::error_raised, static_cast<T*>(ctx), err);
                else std::invoke(&T::warning_raised, static_cast<T*>(ctx), err);
            });
        }

        ~XmlPushParser() {
            xmlFreeParserCtxt(_parser_ctx);
            memset(&sax_handlers, 0, sizeof(sax_handlers));
        }

        int parse_chunk(const char* chunk, int size, int terminate) {
            return xmlParseChunk(_parser_ctx, chunk, size, terminate);
        }

        // void element_decl(const xmlChar* name, int type, xmlElementContentPtr content) {}
        // void entity_decl(const xmlChar* name, int type, const xmlChar* publicId, const xmlChar* systemId, xmlChar* content) {}
        // void reference(const xmlChar* name) {}

        // void start_document() {}
        // void end_document() {}

        void start_element(const xmlChar* name, const KeyValuePair *attrs, int nb_attrs) {}
        void _start_element(const xmlChar* name, const xmlChar** attrs) {
            int nb_attrs = 0;
            std::pair<const xmlChar*, const xmlChar*> attr_arr[MAX_NB_ATTRS];
            // std::fill(attr_arr, attr_arr + MAX_NB_ATTRS, std::make_pair(nullptr, nullptr));
            if (attrs != nullptr) {
                for (auto i = 0; attrs[i + 1] != nullptr; i += 2) {
                    if (attrs[i + 1] != nullptr) attr_arr[i / 2] = std::make_pair(attrs[i], attrs[i + 1]);
                    else attr_arr[i / 2] = std::make_pair(attrs[i], nullptr);
                    nb_attrs = 1 + i / 2;
                }
            }
            derived().start_element(name, attr_arr, nb_attrs);
        }
        // void start_element_ns(const xmlChar* local_name, const xmlChar* prefix, const xmlChar* uri, int nb_namespaces, const xmlChar** namespaces, int nb_attrs, int nb_defaulted, const xmlChar** attributes) {}
        void end_element(const xmlChar* name) {}
        // void end_element_ns(const xmlChar* local_name, const xmlChar* prefix, const xmlChar* uri) {}

        // void attribute(const xmlChar* name, const xmlChar* value) {}
        // void attribute_decl(const xmlChar* elem, const xmlChar* fullname, int type, int def, const xmlChar* default_value, xmlEnumerationPtr tree) {}
        void cdata_block(const xmlChar* value, int len) {}
        void characters(const xmlChar* ch, int len) {}
        // void comment(const xmlChar* value) {}
        // void ignorable_whitespace(const xmlChar* ch, int len) {}

        static void error_(void* ctx, const char* msg, ...) { return std::invoke(&T::error, static_cast<T*>(ctx), msg); }
        void error(const char* msg, ...) {}
        static void warning_(void* ctx, const char* msg, ...) { return std::invoke(&T::warning, static_cast<T*>(ctx), msg); }
        void warning(const char* msg, ...) {}

        void error_raised(xmlErrorPtr err) {
            if (err->level > xmlErrorLevel::XML_ERR_WARNING)
                throw ParseException(
                     "xmlError emitted: "
                     "library_domain={} \n"
                     "code={} \n"
                     "msg={} \n"
                     "err_level={} \n"
                     "file_name={} \n"
                     "line_number={} \n"
                     "str1={} \n"
                     "str2={} \n"
                     "str3={} \n",
                     err->domain,
                     err->code,
                     IN_CASE_OF_NULL(err->message, "(nullstring)"),
                     err->level,
                     IN_CASE_OF_NULL(err->file, "(nullstring)"),
                     err->line,
                     IN_CASE_OF_NULL(err->str1, "(nullstring)"),
                     IN_CASE_OF_NULL(err->str2, "(nullstring)"),
                     IN_CASE_OF_NULL(err->str3, "(nullstring)"));
        }
        void warning_raised(xmlErrorPtr warning) {  }

        const char* get_value_from_key(const char* key, const KeyValuePair* kvps, int nb_attrs) {
            auto p = std::find_if(kvps, kvps + nb_attrs, [=](const KeyValuePair& kvps){ return strcmp(reinterpret_cast<const char*>(kvps.first), key) == 0;});
            if (p != kvps + nb_attrs) return reinterpret_cast<const char*>(p->second);
            else return nullptr;
        }

        T& derived() { return static_cast<T&>(*this); }
    };

    // Unoptimized and should only be used for debug
    struct XmlPrint : public XmlPushParser<XmlPrint> {
        std::reference_wrapper<std::ostream> os;
        int depth = 0;

        XmlPrint(std::ostream& os)
            : os { os }
            , depth { 0 }
            , XmlPushParser<XmlPrint>() {
            assert(sax_handlers.cdataBlock != nullptr);
        };

        std::string indent(int depth) { return std::string (depth * 3, ' '); }
        std::string string_from_xmlChar(const xmlChar* s) {
            char b[500];
            memset(b, 0, 500);
            auto size = xmlUTF8Strlen(s);
            for (int i = 0; i < size; i++) {
                b[i] = static_cast<char>(s[i]);
            }
            return std::string(b, size);
        }

        void start_element(const xmlChar* name, const KeyValuePair* attrs, int nb_attrs) {
            std::string attrs_str = "";
            for (int i = 0; i < nb_attrs; i ++) {
                auto [key, value] = attrs[i];
                if (value != nullptr) attrs_str += fmt::format(" {}={}", key, value);
                else attrs_str += fmt::format(" {}", key);
            }
            os.get() << indent(depth) << fmt::format("+<{} {}>", name, attrs_str) << std::endl;
            depth++;
        }
        void end_element(const xmlChar* name) {
            os.get() << indent(depth) << fmt::format("-</{}>", name) << std::endl;
            depth--;
        }
        void characters(const xmlChar* ch, int len) {
            const static auto isspace_or_newline = []<typename T>(T c) { return (c == ' ' || c == '\n' || c == '\t'); };
            if (std::all_of(ch, ch+len, isspace_or_newline)) {return;}
            char b[500];
            memset(b, 0, 500);
            for (int i = 0; i < len; i++) {
                b[i] = static_cast<char>(ch[i]);
            }
            os.get() << indent(depth + 1) << fmt::format("\"{}\"", std::string(b, len)) << std::endl;
        }
    };
}

#endif //AIDA_XMLSAX_H
