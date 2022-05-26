#include <stdio.h>
 
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
 
//- ?????? ? IP-???????? --------------------------------------------------------------
 
#define byte unsigned char
 
typedef struct
{
    byte a, b, c, d;
} ip_t;
 
// ?????????? ?????? ? ??????? ?? ??? IP
void ip_make( ip_t * out, const char * addr )
{
    int b = 0;
    byte *ptr = &(out->a);
    char buff[4] = { 0 };
 
    do
    {
        if( *addr == '.' || *addr == 0 )
        {
           *ptr = atoi( buff );
           b = 0;
           ptr++;
        }
        else if( b < 4 )
        {
           buff[ b ] = *addr;
           b++;
           buff[ b ] = 0;
        }
 
        addr++;
 
    } while( *(addr - 1) != 0 );
}
 
// ????????? ??????????? ?
void ip_and( const ip_t * in_1, const ip_t * in_2, ip_t * out )
{
    out->a = in_1->a & in_2->a;
    out->b = in_1->b & in_2->b;
    out->c = in_1->c & in_2->c;
    out->d = in_1->d & in_2->d;
}
 
// ????????? ??????????? ??
void ip_not( ip_t * ip_to_invert )
{
    ip_to_invert->a = ~ip_to_invert->a;
    ip_to_invert->b = ~ip_to_invert->b;
    ip_to_invert->c = ~ip_to_invert->c;
    ip_to_invert->d = ~ip_to_invert->d;
}
 
// ????????? ?????????
// true = ?????????
char ip_cmp( const ip_t * in_1, const ip_t * in_2 )
{
    return !( ( in_1->a != in_2->a ) ||
              ( in_1->b != in_2->b ) ||
              ( in_1->c != in_2->c ) ||
              ( in_1->d != in_2->d ) );
}
 
//- END ?????? ? IP-???????? ----------------------------------------------------------
 
int main( int argc, char ** argv )
{
    ip_t addr, mask,
         subnet, host;
 
    if( argc < 3 )
    {
        printf( "Usage: %s [ip] [mask]", argv[0] );
        return 1;
    }
 
    // ????????? ?????????
    ip_make( &addr, argv[ 1 ] );
    ip_make( &mask, argv[ 2 ] );
 
    // ???????? ???????
    ip_and( &addr, &mask, &subnet );
 
    // ???????? ?????
    ip_not( &mask );
    ip_and( &addr, &mask, &host );
    ip_not( &mask );
 
/*
    printf( "IP:     %i.%i.%i.%i\nMASK:   %i.%i.%i.%i\n-----------------------"
            "\nSUBNET: %i.%i.%i.%i\nHOST:   %i.%i.%i.%i\n-----------------------\n",
            addr.a, addr.b, addr.c, addr.d,
            mask.a, mask.b, mask.c, mask.d,
            subnet.a, subnet.b, subnet.c, subnet.d,
            host.a, host.b, host.c, host.d );*/
 
 
    // ???????? ?????? ????? ???????
    struct ifaddrs *addrs, *ptr;
    getifaddrs( &addrs );
 
    for( ptr = addrs; ptr != 0; ptr = ptr->ifa_next )
    {
        if( ptr->ifa_addr->sa_family != AF_INET ) // ?????? IPv4
           continue;
 
        ip_t this_addr, this_mask;
        ip_make( &this_addr, inet_ntoa( ((struct sockaddr_in *)ptr->ifa_addr)->sin_addr ) );
        ip_make( &this_mask, inet_ntoa( ((struct sockaddr_in *)ptr->ifa_netmask)->sin_addr ) );
 
        printf( "ip %i.%i.%i.%i mask %i.%i.%i.%i",
               this_addr.a, this_addr.b, this_addr.c, this_addr.d,
               this_mask.a, this_mask.b, this_mask.c, this_mask.d );
 
        if( ip_cmp( &mask, &this_mask ) ) // ????? ?????????
        {
            ip_t this_subnet;
            ip_and( &this_addr, &this_mask, &this_subnet ); // ??????? ???????
 
            if( ip_cmp( &subnet, &this_subnet ) ) // ??????? ?????????
            {
                printf( " <--- given host should be in this subnet" );
            }
        }
 
        printf( "\n" );
    }
 
    freeifaddrs( addrs );
 
    return 0;
}
