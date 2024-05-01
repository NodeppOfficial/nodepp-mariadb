#ifndef NODEPP_SQLITE
#define NODEPP_SQLITE

/*────────────────────────────────────────────────────────────────────────────*/

#include <mysql.h>
#include <nodepp/url.h>
#include <nodepp/ssl.h>
#include <nodepp/object.h>

namespace nodepp { class mysql_t {
protected:

    struct NODE {
        MYSQL *fd;
        int state = 1;
    };  ptr_t<NODE> obj;

    template< class T > void callback( T& cb ) { 
        MYSQL_RES *result = mysql_store_result(obj->fd);
        object_t arguments; array_t<string_t> col;

        if( result == NULL ) {
            string_t message = mysql_error( obj->fd );
            process::error( "SQL Error: ", message );
        }

        int num_fields = mysql_num_fields( result );
        MYSQL_ROW row  = mysql_fetch_row( result );

        for( int x=0; x<num_fields; x++ )
           { col.push( row[x] ); }

        while( (row=mysql_fetch_row(result)) ) {
          for( int i=0; i<num_fields; i++ ){
               arguments[ col[x] ] = row[x] ? row[x] : "NULL"; 
             }
        }

        cb( arguments ); mysql_free_result( result );
    }

public:
    
    virtual ~mysql_t() noexcept {
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
    
    mysql_t ( string_t uri, string_t name, ssl_t* ssl ) : obj( new NODE ) {
        obj->fd = mysql_init(NULL);
        
        if( obj->fd == nullptr )
          { process::error("Error: Can't Start MySQL"); }

        auto host = url::hostname( uri );
        auto user = url::user( uri );
        auto pass = url::pass( uri );
        auto port = url::port( uri );

        mysql_ssl_set( obj->fd, ssl->get_key_path(), ssl->get_crt_path(), ssl->get_ca_path(), NULL, NULL );

        if( mysql_real_connect( obj->fd, host, user, pass, name, port, NULL, 0 ) == NULL ){
            string_t message = mysql_error( obj->fd );
            process::error( "SQL Error: ", message );
        }

    }
    
    /*─······································································─*/
    
    mysql_t ( string_t uri, string_t name ) : obj( new NODE ) {
        obj->fd = mysql_init(NULL);
        
        if( obj->fd == nullptr )
          { process::error("Error: Can't Start MySQL"); }

        auto host = url::host( uri );
        auto user = url::user( uri );
        auto pass = url::pass( uri );
        auto port = url::port( uri );

        if( mysql_real_connect( obj->fd, host, user, pass, name, port, NULL, 0 ) == NULL ){
            string_t message = mysql_error( obj->fd );
            process::error( "SQL Error: ", message );
        }

    }
    
    /*─······································································─*/

    void exec( const string_t& cmd, const function_t<void,object_t>& cb ) { char* msg;
        if( mysql_real_query( obj->fd, cmd.data(), cmd.size() ) != 0 ){
            string_t message = mysql_error( obj->fd );
            process::error( "SQL Error: ", message );
        }   callback( cb );
    }

    void exec( const string_t& cmd ) { char* msg;
        if( mysql_real_query( obj->fd, cmd.data(), cmd.size() ) != 0 ){
            string_t message = mysql_error( obj->fd );
            process::error( "SQL Error: ", message );
        }
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif