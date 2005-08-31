/*
 * keygen is a small programs that generate a dnskey and private key
 * for a particulary domain. It (currently) prints out the public key
 * (c) NLnet Labs, 2005
 * Licensed under the GPL version 2
 */

#include <ldns/config.h>

#include <ldns/dns.h>

void
usage(FILE *fp, char *prog) {
	fprintf(fp, "%s keygen [-D|-R] -b bits domain\n", prog);
	fprintf(fp, "  generate a new key pair for domain\n");
	fprintf(fp, "  -D\tgenerate a DSA key\n");
	fprintf(fp, "  -R\tgenerate a RSA key\n");
	fprintf(fp, "  -k\tset the flags to 257; key signing key\n");
	fprintf(fp, "  -b <bits>\tspecify the keylength\n");
	fprintf(fp, "  The public key is printed to stdout\n");
	fprintf(fp, "  The private key is printed to stderr\n");
	fprintf(fp, "\nWARNING, WARNING, this program does NOT use a good random source for the key generation.\nUse at your OWN RISK\n\n");
}

int
main(int argc, char *argv[])
{
	int c;
	char *prog;

	/* default key size */
	uint16_t def_bits = 1024;
	uint16_t bits = def_bits;
	bool ksk;

	ldns_signing_algorithm algorithm;
	ldns_rdf *domain;
	ldns_rr *pubkey;
	ldns_key *key;

	prog = strdup(argv[0]);
	algorithm = 0;
	ksk = false; /* don't create a ksk per default */
	
	while ((c = getopt(argc, argv, "DRkb:")) != -1) {
		switch (c) {
		case 'D':
			if (algorithm != 0) {
				fprintf(stderr, "%s: %s", prog, "Only one -D or -A is allowed\n");
				exit(EXIT_FAILURE);
			}
			algorithm = LDNS_SIGN_DSA;
			break;
		case 'R':
			if (algorithm != 0) {
				fprintf(stderr, "%s: %s", prog, "Only one -D or -A is allowed\n");
				exit(EXIT_FAILURE);
			}
			algorithm = LDNS_SIGN_RSASHA1;
			break;
		case 'b':
			bits = atoi(optarg);
			if (bits == 0) {
				fprintf(stderr, "%s: %s %d", prog, "Can not parse the -b argument, setting it to the default\n", def_bits);
			}
			bits = def_bits;
			break;
		case 'k':
			ksk = true;
			break;
		default:
			usage(stderr, prog);
			exit(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;

	if (algorithm == 0) {
		algorithm = LDNS_SIGN_RSASHA1; /* default to RSA SHA1 */
	}

	if (argc != 1) {
		usage(stderr, prog);
		exit(EXIT_FAILURE);
	} 

	/* create an rdf from the domain name */
	domain = ldns_dname_new_frm_str(argv[0]);

	/* generate a new key */
	key = ldns_key_new_frm_algorithm(algorithm, bits);

	/* set the owner name in the key - this is a /seperate/ step */
	ldns_key_set_pubkey_owner(key, domain);

	/* ksk flag */
	if (ksk) {
		ldns_key_set_flags(key, ldns_key_flags(key) + 1);
	}

	/* create the public from the ldns_key */
	pubkey = ldns_key2rr(key);
	
	/* print it to stdout */
	ldns_rr_print(stdout, pubkey);

	/* print the priv key to stderr */
	ldns_key_print(stderr, key);
        exit(EXIT_SUCCESS);
}
