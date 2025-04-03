#ifndef NODEPP_MARIADB
#define NODEPP_MARIADB

/*────────────────────────────────────────────────────────────────────────────*/

#include <mariadb/mysql.h>
#include <nodepp/nodepp.h>
#include <nodepp/url.h>

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_MARIADB_GENERATOR
#define NODEPP_MARIADB_GENERATOR

namespace nodepp { namespace _mariadb_ { GENERATOR( cb ){
protected:
    
    array_t<string_t> col;
    int num_fields, x;
    MYSQL_ROW row;

public:

    template< class T, class U, class V, class Q > coEmit( T& fd, U& res, V& cb, Q& self ){
    gnStart 

        num_fields = mysql_num_fields( res ); 
        row        = mysql_fetch_row( res );

        for( x=0; x<num_fields; x++ )
           { col.push( row[x] ); }

        while( (row=mysql_fetch_row(res)) ){ do {
            auto object = map_t<string_t,string_t>();
            
            for( x=0; x<num_fields; x++ ){
                 object[ col[x] ] = row[x] ? row[x] : "NULL"; 
            }
            
        cb( object ); } while(0); coNext; }

        mysql_free_result( res );

    gnStop
    }

};}}

#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { using sql_item_t = map_t<string_t,string_t>; }

namespace nodepp { class mariadb_t {
protected:

    struct NODE {
        MYSQL *fd = nullptr;
        int state = 1;
    };  ptr_t<NODE> obj;

public:
    
    virtual ~mariadb_t() noexcept {
        if( obj.count() > 1 || obj->fd == nullptr ){ return; }
        if( obj->state == 0 ){ return; } free();
    }
    
    /*─······································································─*/

    virtual void free() const noexcept {
        if( obj->fd == nullptr ){ return; }
        if( obj->state == 0 )   { return; }
        mysql_close( obj->fd );
        obj->state = 0; 
    }
    
    /*─······································································─*/
    
#ifdef NODEPP_SSL
    mariadb_t ( string_t uri, string_t name, ssl_t* ssl ) : obj( new NODE ) {
        
        obj->fd = mysql_init(NULL); if( obj->fd == nullptr )
          { process::error("Error: Can't Start MySQL"); }

        auto host = url::hostname( uri );
        auto user = url::user( uri );
        auto pass = url::pass( uri );
        auto port = url::port( uri );

        char* key = ssl->get_key_path().get();
        char* crt = ssl->get_crt_path().get(); mysql_ssl_set( obj->fd, key, crt, NULL , NULL, NULL );

        if( mysql_real_connect( obj->fd, host.get(), user.get(), pass.get(), name.get(), port, NULL, 0 ) == NULL ){
            process::error( mysql_error( obj->fd ) );
        }

    }
#endif
    
    /*─······································································─*/
    
    mariadb_t ( string_t uri, string_t name ) : obj( new NODE ) {
        
        obj->fd = mysql_init(NULL); if( obj->fd == nullptr )
          { process::error("Error: Can't Start MySQL"); }

        auto host = url::hostname( uri );
        auto user = url::user( uri );
        auto pass = url::pass( uri );
        auto port = url::port( uri );

        if( mysql_real_connect( obj->fd, host.get(), user.get(), pass.get(), name.get(), port, NULL, 0 ) == NULL ){
            process::error( mysql_error( obj->fd ) );
        }

    }
    
    mariadb_t () : obj( new NODE ) { obj->state = 0; }
    
    /*─······································································─*/

    void exec( const string_t& cmd, const function_t<void,sql_item_t>& cb ) const {
        if( obj->state == 0 ){ return; }

        if( mysql_real_query( obj->fd, cmd.data(), cmd.size() ) != 0 ){
            process::error( mysql_error( obj->fd ) );
        }

        auto self = type::bind( this );
        MYSQL_RES *res = mysql_store_result( obj->fd );

        if( res == NULL ) { process::error( mysql_error(fd) ); }
        _mariadb_::cb task; process::add( task, obj->fd, res, cb, self );
    }

    array_t<sql_item_t> exec( const string_t& cmd ) const { array_t<sql_item_t> arr;
        function_t<void,sql_item_t> cb = [&]( sql_item_t args ){ arr.push( args ); };

        if( obj->state == 0 ){ return nullptr; }

        if( mysql_real_query( obj->fd, cmd.data(), cmd.size() ) != 0 ){
            process::error( mysql_error( obj->fd ) );
        }

        auto self = type::bind( this );
        MYSQL_RES *res = mysql_store_result( obj->fd );

        if( res == NULL ) { process::error( mysql_error(fd) ); }
        _mariadb_::cb task; process::await( task, obj->fd, res, cb, self ); return arr;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
