/*
 * Copyright © 2023 Michael Smith <mikesmiffy128@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef INC_ABI_H
#define INC_ABI_H

#include "intdefs.h"

/*
 * This file defines miscellaneous C++ ABI stuff. Looking at it may cause brain
 * damage and/or emotional trauma.
 */

#ifdef _WIN32 // Windows RTTI stuff, obviously only used on Windows.

// MSVC RTTI is quite a black box, but thankfully there's some useful sources:
// - https://doxygen.reactos.org/d0/dcf/cxx_8h_source.html
// - https://blog.quarkslab.com/visual-c-rtti-inspection.html
// - https://www.geoffchappell.com/studies/msvc/language/predefined/
// - https://docs.rs/pelite/0.5.0/src/pelite/pe32/msvc.rs.html

// By the way, while I'm here I'd just like to point out how ridiculous this
// layout is. Whoever decided to put this many levels of indirection over what
// should be a small lookup table is an absolute nutcase. I hope that individual
// has gotten some help by now, mostly for the sake of others.

struct msvc_rtti_descriptor_head {
	void **vtab;
	void *unknown; // ???
	// descriptor includes this, but constant flexible arrays are annoying, so
	// this structure is just the header part and the string is tacked on in the
	// DEF_MSVC_BASIC_RTTI macro below
	//char classname[];
};

// "pointer to member displacement"
struct msvc_pmd { int mdisp, pdisp, vdisp; };

struct msvc_basedesc {
	const struct msvc_rtti_descriptor_head *desc;
	uint nbases;
	struct msvc_pmd where;
	uint attr;
};

struct msvc_rtti_hierarchy {
	uint sig;
	uint attrs;
	uint nbaseclasses;
	const struct msvc_basedesc **baseclasses;
};

struct msvc_rtti_locator {
	uint sig;
	int baseoff;
	// ctor offset, or some flags; reactos and rust pelite say different things?
	int unknown;
	const struct msvc_rtti_descriptor_head *desc;
	const struct msvc_rtti_hierarchy *hier;
};

// I mean seriously look at this crap!
#define DEF_MSVC_BASIC_RTTI(mod, name, vtab, typestr) \
const mod struct msvc_rtti_locator name; \
static const struct { \
	struct msvc_rtti_descriptor_head d; \
	char classname[sizeof("" typestr)]; \
} _desc_##name = {(vtab), 0, .classname = "" typestr}; \
static const struct msvc_basedesc _basedesc_##name = { \
	&_desc_##name.d, 0, {0, 0, 0}, 0 \
}; \
mod const struct msvc_rtti_locator name = { \
	0, 0, 0, \
	&_desc_##name.d, \
	&(struct msvc_rtti_hierarchy){ \
		0, 1 /* match engine */, 1, \
		(const struct msvc_basedesc *[]){&_basedesc_##name} \
	} \
};

#else

#warning FIXME! More stuff needs to be implemented/fixed here!

struct itanium_type_info {
	struct itanium_type_info_vtable {
		void *dtor1, *dtor2; // ???
		// there's some more functions here in libstdc++: is_pointer,
		// is_function, do_catch, etc. however they're not specified in itanium
		// abi doc. hoping to do without them, but we'll see I guess
	} *vtable;
	const char *name;
};

#define DEF_ITANIUM_BASIC_RTTI(mod, name, typestr) \
	mod struct itanium_type_info name = { \
		&(struct itanium_type_info_vtable){ \
			0, 0 /* FIXME/TEMP: definitely need real functions here! */ \
		}, \
		typestr \
	};

#endif

#endif

// vi: sw=4 ts=4 noet tw=80 cc=80
