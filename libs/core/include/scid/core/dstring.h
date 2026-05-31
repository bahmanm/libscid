/** @file
 * Deprecated dynamic string helper.
 */

#pragma once

#include "scid/core/primitives.h"

#include <cstddef>
#include <cstdio>
#include <string>

namespace scid::core {

/** Legacy growable string builder.
 *
 * DString is kept for older text-rendering code that appends fragments through
 * a Scid-style API. New code should usually prefer std::string directly.
 */
class DString { // DEPRECATED
	std::string s_;

public:
	/** Remove all accumulated text. */
	void Clear() { s_.clear(); }

	/** Return a null-terminated view of the current text.
	 *
	 * The returned pointer is invalidated by any later non-const operation on
	 * this DString.
	 */
	const char* Data() { return s_.c_str(); }

	/** Return the number of bytes currently stored. */
	size_t Length(void) { return s_.size(); }

	/** Append one character. */
	void AddChar(char ch) { s_.append(1, ch); }

	/** Append an unsigned integer in decimal notation. */
	void AppendUint(uint i) {
		char s[16];
		std::snprintf(s, sizeof(s), "%u", i);
		s_.append(s);
	}

	/** Append a signed integer in decimal notation. */
	void AppendInt(int i) {
		char s[16];
		std::snprintf(s, sizeof(s), "%d", i);
		s_.append(s);
	}

	/** Append an unsigned integer in decimal notation. */
	void Append(uint i) { AppendUint(i); }

	/** Append a null-terminated string. */
	void Append(const char* str) { s_.append(str); }

	// To allow convenient appending of multiple strings without resorting
	// to messy variable-length argument lists, we define DString::Append()
	// for up to five string arguments, and for up to four arguments where
	// one is an unsigned integer and the rest are strings.
	/** Append two null-terminated strings. */
	void Append(const char* s1, const char* s2) {
		Append(s1);
		Append(s2);
	}
	/** Append a string followed by an unsigned integer. */
	void Append(const char* s1, uint i2) {
		Append(s1);
		Append(i2);
	}
	/** Append an unsigned integer followed by a string. */
	void Append(uint i1, const char* s2) {
		Append(i1);
		Append(s2);
	}
	/** Append three fragments. */
	void Append(const char* s1, const char* s2, const char* s3) {
		Append(s1);
		Append(s2);
		Append(s3);
	}
	/** Append three fragments, with the final fragment formatted as an unsigned integer. */
	void Append(const char* s1, const char* s2, uint i3) {
		Append(s1);
		Append(s2);
		Append(i3);
	}
	/** Append three fragments, with the middle fragment formatted as an unsigned integer. */
	void Append(const char* s1, uint i2, const char* s3) {
		Append(s1);
		Append(i2);
		Append(s3);
	}
	/** Append three fragments, with the first fragment formatted as an unsigned integer. */
	void Append(uint i1, const char* s2, const char* s3) {
		Append(i1);
		Append(s2);
		Append(s3);
	}
	/** Append four null-terminated strings. */
	void Append(const char* s1, const char* s2, const char* s3,
	            const char* s4) {
		Append(s1);
		Append(s2);
		Append(s3);
		Append(s4);
	}
	/** Append four fragments, with the final fragment formatted as an unsigned integer. */
	void Append(const char* s1, const char* s2, const char* s3, uint i4) {
		Append(s1);
		Append(s2);
		Append(s3);
		Append(i4);
	}
	/** Append four fragments, with the third fragment formatted as an unsigned integer. */
	void Append(const char* s1, const char* s2, uint i3, const char* s4) {
		Append(s1);
		Append(s2);
		Append(i3);
		Append(s4);
	}
	/** Append four fragments, with the second fragment formatted as an unsigned integer. */
	void Append(const char* s1, uint i2, const char* s3, const char* s4) {
		Append(s1);
		Append(i2);
		Append(s3);
		Append(s4);
	}
	/** Append four fragments, with the first fragment formatted as an unsigned integer. */
	void Append(uint i1, const char* s2, const char* s3, const char* s4) {
		Append(i1);
		Append(s2);
		Append(s3);
		Append(s4);
	}
	/** Append five null-terminated strings. */
	void Append(const char* s1, const char* s2, const char* s3, const char* s4,
	            const char* s5) {
		Append(s1);
		Append(s2);
		Append(s3);
		Append(s4);
		Append(s5);
	}
};


} // namespace scid::core
