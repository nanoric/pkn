#pragma once
#include "basic_type.h"

#include "encrypted_type/encrypted_number.hpp"
#include "encrypted_type/encrypted_string.hpp"
#include "encrypted_type/encrypted_string_view.hpp"
#include "encrypted_type/encrypted_string_utils.hpp"

using erptr_t = encrypted_number<rptr_t>;
using ecrptr_t = const_encrypted_number<rptr_t>;
using euint8_t = encrypted_number<uint8_t>;
using euint16_t = encrypted_number<uint16_t>;
using euint32_t = encrypted_number<uint32_t>;
using euint64_t = encrypted_number<uint64_t>;

using eint8_t = encrypted_number<int8_t>;
using eint16_t = encrypted_number<int16_t>;
using eint32_t = encrypted_number<int32_t>;
using eint64_t = encrypted_number<int64_t>;

using random_t = compile_time::random_t;
using pid_t = euint64_t;

using estr_t = basic_encrypted_string<wchar_t>;
using estrv_t = basic_encrypted_string_view<wchar_t>;

#define e(string_literal) L#string_literal/*this marco should not be used for user*/

#define make_hash(string_literal) (const_hash(compile_time::hash(e(string_literal))))
#define make_estr(string_literal) (make_const_encrypted_string(e(string_literal)))
#define make_eqstr(string_literal) QString::fromStdU32String(make_const_encrypted_string(e(string_literal)).to_u32string())
#define make_estdstr(string_literal) (make_const_encrypted_string(e(string_literal)).to_string())
#define make_estdwstr(string_literal) (make_const_encrypted_string(e(string_literal)).to_wstring())
#define make_ecstr(string_literal) (make_const_encrypted_string(e(string_literal)).to_string().c_str())
#define make_ecwstr(string_literal) (make_const_encrypted_string(e(string_literal)).to_wstring().c_str())
