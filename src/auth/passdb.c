/* Copyright (C) 2002-2003 Timo Sirainen */

#include "common.h"
#include "mech.h"
#include "passdb.h"

#include <stdlib.h>

struct passdb_module *passdb;

const char *passdb_credentials_to_str(enum passdb_credentials credentials)
{
	switch (credentials) {
	case PASSDB_CREDENTIALS_PLAINTEXT:
		return "plaintext";
	case PASSDB_CREDENTIALS_DIGEST_MD5:
		return "digest-md5";
	}

	return "??";
}

void passdb_init(void)
{
	const char *name, *args;

	passdb = NULL;

	name = getenv("PASSDB");
	if (name == NULL)
		i_fatal("PASSDB environment is unset");

	args = strchr(name, ' ');
	name = t_strcut(name, ' ');

#ifdef PASSDB_PASSWD
	if (strcasecmp(name, "passwd") == 0)
		passdb = &passdb_passwd;
#endif
#ifdef PASSDB_PASSWD_FILE
	if (strcasecmp(name, "passwd-file") == 0)
		passdb = &passdb_passwd_file;
#endif
#ifdef PASSDB_PAM
	if (strcasecmp(name, "pam") == 0)
		passdb = &passdb_pam;
#endif
#ifdef PASSDB_SHADOW
	if (strcasecmp(name, "shadow") == 0)
		passdb = &passdb_shadow;
#endif
#ifdef PASSDB_VPOPMAIL
	if (strcasecmp(name, "vpopmail") == 0)
		passdb = &passdb_vpopmail;
#endif

	if (passdb == NULL)
		i_fatal("Unknown passdb type '%s'", name);

	/* initialize */
	if (passdb->init != NULL)
		passdb->init(args != NULL ? args+1 : "");

	if ((auth_mechanisms & AUTH_MECH_PLAIN) &&
	    passdb->verify_plain == NULL)
		i_fatal("Passdb %s doesn't support PLAIN method", name);

	if ((auth_mechanisms & AUTH_MECH_DIGEST_MD5) &&
	    passdb->lookup_credentials == NULL)
		i_fatal("Passdb %s doesn't support DIGEST-MD5 method", name);
}

void passdb_deinit(void)
{
	if (passdb != NULL && passdb->deinit != NULL)
		passdb->deinit();
}
