#include "Debug.h"

void dump_buf( const char *title, unsigned char *buf, size_t len )
{
    size_t i;

    mbedtls_printf( "%s", title );
    for( i = 0; i < len; i++ )
        mbedtls_printf("%c%c", "0123456789ABCDEF" [buf[i] / 16],
                       "0123456789ABCDEF" [buf[i] % 16] );
    mbedtls_printf( "\n" );
}

void dump_pubkey( const char *title, mbedtls_ecdsa_context *key )
{
    // each point on our curve is 256 bit (32 Bytes)
    // two points plus the leading 0x04 byte
    unsigned char buf[2*32 + 1];
    unsigned char addr[32];
    size_t len;

    if( mbedtls_ecp_point_write_binary( &key->grp, &key->Q,
                MBEDTLS_ECP_PF_UNCOMPRESSED, &len, buf, sizeof buf ) != 0 )
    {
        mbedtls_printf("internal error\n");
        return;
    }

    // buf + 1 to skip the first 0x04 byte
    dump_buf( title, buf + 1, len -1);
}

void dump_mpi (const char* title, mbedtls_mpi* X)
{
    size_t len = mbedtls_mpi_bitlen(X);
    unsigned char* buf;

    if (len == 0)
    {
        printf("%s%d\n", title, 0);
        return;
    }
    
    len = ((len + 7) & ~0x07) / 8;
    buf = (unsigned char*) malloc(len);
    mbedtls_mpi_write_binary (X, buf, len);
    dump_buf (title, buf, len);
    free(buf);
}

void dump_group( const char* title, mbedtls_ecp_group* grp)
{
    unsigned char buf[128];
    size_t len;

    mbedtls_printf("%s", title);

    dump_mpi("A=", &grp->A);
    dump_mpi("B=", &grp->B);

    mbedtls_ecp_point_write_binary( grp, &grp->G,
                MBEDTLS_ECP_PF_UNCOMPRESSED, &len, buf, sizeof buf );
    dump_buf("G=", buf, len);

    dump_mpi("N=", &grp->N);
    printf("h=%d\n", grp->h);
}