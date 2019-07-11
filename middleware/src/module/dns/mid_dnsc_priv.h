#ifndef CYGONCE_NS_DNS_DNS_PRIV_H
#define CYGONCE_NS_DNS_DNS_PRIV_H
struct dns_header {
#if (CYG_BYTEORDER == CYG_LSBFIRST)
  unsigned        id :16;         /* query identification number */
  /* fields in third byte */
  unsigned        rd :1;          /* recursion desired */
  unsigned        tc :1;          /* truncated message */
  unsigned        aa :1;          /* authoritive answer */
  unsigned        opcode :4;      /* purpose of message */
  unsigned        qr :1;          /* response flag */
  /* fields in fourth byte */
  unsigned        rcode :4;       /* response code */
  unsigned        cd: 1;          /* checking disabled by resolver */
  unsigned        ad: 1;          /* authentic data from named */
  unsigned        unused :1;      /* unused bits */
  unsigned        ra :1;          /* recursion available */
  /* remaining bytes */
  unsigned        qdcount :16;    /* number of question entries */
  unsigned        ancount :16;    /* number of answer entries */
  unsigned        nscount :16;    /* number of authority entries */
  unsigned        arcount :16;    /* number of resource entries */
#else
  unsigned        id :16;         /* query identification number */
  /* fields in third byte */
  unsigned        qr :1;          /* response flag */
  unsigned        opcode :4;      /* purpose of message */
  unsigned        aa :1;          /* authoritive answer */
  unsigned        tc :1;          /* truncated message */
  unsigned        rd :1;          /* recursion desired */
  /* fields in fourth byte */
  unsigned        ra :1;          /* recursion available */
  unsigned        unused :1;      /* unused bits */
  unsigned        ad: 1;          /* authentic data from named */
  unsigned        cd: 1;          /* checking disabled by resolver */
  unsigned        rcode :4;       /* response code */
  /* remaining bytes */
  unsigned        qdcount :16;    /* number of question entries */
  unsigned        ancount :16;    /* number of answer entries */
  unsigned        nscount :16;    /* number of authority entries */
  unsigned        arcount :16;    /* number of resource entries */
#endif
};

struct resource_record {
  unsigned rr_type : 16; /* Type of resourse */
  unsigned class   : 16; /* Class of resource */
  unsigned ttl     : 32; /* Time to live of this record */
  unsigned rdlength: 16; /* Lenght of data to follow */
  char     rdata [2];   /* Resource DATA */
};

/* Opcodes */
#define DNS_QUERY  0   /* Standard query */
#define DNS_IQUERY 1   /* Inverse query */
#define DNS_STATUS 2   /* Name server status */
#define DNS_NOTIFY 4   /* Zone change notification */
#define DNS_UPDATE 5   /* Zone update message */

/* DNS TYPEs */
#define DNS_TYPE_A     1   /* Host address */
#define DNS_TYPE_NS    2   /* Authoritative name server */
#define DNS_TYPE_CNAME 5   /* Canonical name for an alias */
#define DNS_TYPE_PTR   12  /* Domain name pointer */

/* DNS CLASSs */
#define DNS_CLASS_IN   1   /* Internet */

/* DNS reply codes */
#define DNS_REPLY_NOERR      0
#define DNS_REPLY_NAME_ERROR 3

#define MAXDNSMSGSIZE 512

#endif 
