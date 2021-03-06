/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 1999 - 2005, Digium, Inc.
 *
 * Mark Spencer <markster@digium.com>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 */

/*! \file
 * \brief Channel Variables
 */

#ifndef _ASTERISK_CHANVARS_H
#define _ASTERISK_CHANVARS_H

#include "asterisk/linkedlists.h"

struct ast_var_t {
	AST_LIST_ENTRY(ast_var_t) entries;
	char *value;
	char name[0];
};

AST_LIST_HEAD_NOLOCK(varshead, ast_var_t);

#ifdef __AST_DEBUG_MALLOC
struct ast_var_t *_ast_var_assign(const char *name, const char *value, const char *file, int lineno, const char *function);
#define ast_var_assign(a,b)	_ast_var_assign(a,b,__FILE__,__LINE__,__PRETTY_FUNCTION__)
#else
struct ast_var_t *ast_var_assign(const char *name, const char *value);
#endif
void ast_var_delete(struct ast_var_t *var);
const char *ast_var_name(const struct ast_var_t *var);
const char *ast_var_full_name(const struct ast_var_t *var);
const char *ast_var_value(const struct ast_var_t *var);

#endif /* _ASTERISK_CHANVARS_H */
