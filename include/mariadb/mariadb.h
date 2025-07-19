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
    gnStart ; coWait( self->is_used()==1 ); self->use();

        num_fields = mysql_num_fields( res );
        row        = mysql_fetch_row ( res );

        for( x=0; x<num_fields; x++ )
           { col.push( row[x] ); }

        while( (row=mysql_fetch_row(res)) ){ do {
            auto object = map_t<string_t,string_t>();

            for( x=0; x<num_fields; x++ )
               { object[ col[x] ] = row[x] ? row[x] : "NULL"; }

        cb( object ); } while(0); coNext; } 
        
        self->release(); mysql_free_result( res );

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
        bool used = 0;
        int state = 1;
    };  ptr_t<NODE> obj;

public:

    event_t<> onUse;
    event_t<> onRelease;

    /*─······································································─*/

    virtual ~mariadb_t() noexcept {
        if( obj.count() > 1 || obj->fd == nullptr ){ return; }
        if( obj->state == 0 ){ return; } free();
    }

#ifdef NODEPP_SSL
    mariadb_t ( string_t uri, string_t name, ssl_t* ssl ) : obj( new NODE ) {

        obj->fd = mysql_init(NULL); if( obj->fd == nullptr )
          { throw except_t("Error: Can't Start MySQL"); }

        auto host = url::hostname( uri );
        auto user = url::user( uri );
        auto pass = url::pass( uri );
        auto port = url::port( uri );

        char* key = ssl->get_key_path().get();
        char* crt = ssl->get_crt_path().get(); mysql_ssl_set( obj->fd, key, crt, NULL , NULL, NULL );

        if( mysql_real_connect( obj->fd, host.get(), user.get(), pass.get(), name.get(), port, NULL, 0 ) == NULL ){
            throw except_t( mysql_error( obj->fd ) );
        }

    }
#endif

    /*─······································································─*/

    mariadb_t ( string_t uri, string_t name ) : obj( new NODE ) {

        obj->fd = mysql_init(NULL); if( obj->fd == nullptr )
          { throw except_t("Error: Can't Start MySQL"); }

        auto host = url::hostname( uri );
        auto user = url::user( uri );
        auto pass = url::pass( uri );
        auto port = url::port( uri );

        if( mysql_real_connect( obj->fd, host.get(), user.get(), pass.get(), name.get(), port, NULL, 0 ) == NULL ){
            throw except_t( mysql_error( obj->fd ) );
        }

    }

    mariadb_t () : obj( new NODE ) { obj->state = 0; }

    /*─······································································─*/

    array_t<sql_item_t> exec( const string_t& cmd ) const { queue_t<sql_item_t> arr;
        function_t<void,sql_item_t> cb = [&]( sql_item_t args ){ arr.push( args ); };
        if( cmd.empty() || obj->state==0 ){ return nullptr; }

        if( mysql_real_query( obj->fd, cmd.data(), cmd.size() ) != 0 ){
            throw except_t( mysql_error( obj->fd ) ); return nullptr;
        }

        auto self = type::bind( this );
        MYSQL_RES *res = mysql_store_result( obj->fd );

        if( res == NULL ) { throw except_t( mysql_error(fd) ); }
        _mariadb_::cb task; process::await( task, obj->fd, res, cb, self ); return arr.data();
    }

    void exec( const string_t& cmd, const function_t<void,sql_item_t>& cb ) const {
        if( cmd.empty() || obj->state==0 ){ return; }

        if( mysql_real_query( obj->fd, cmd.data(), cmd.size() ) != 0 ){
            throw except_t( mysql_error( obj->fd ) ); return;
        }

        auto self = type::bind( this );
        MYSQL_RES *res = mysql_store_result( obj->fd );

        if( res == NULL ) { throw except_t( mysql_error(fd) ); }
        _mariadb_::cb task; process::add( task, obj->fd, res, cb, self );
    }

    /*─······································································─*/

    void async( const string_t& cmd ) const { queue_t<sql_item_t> arr;
        function_t<void,sql_item_t> cb = [=]( sql_item_t ){ };
        if( cmd.empty() || obj->state==0 ){ return; }

        if( mysql_real_query( obj->fd, cmd.data(), cmd.size() ) != 0 ){
            throw except_t( mysql_error( obj->fd ) ); return;
        }

        auto self = type::bind( this );
        MYSQL_RES *res = mysql_store_result( obj->fd );

        if( res == NULL ) { throw except_t( mysql_error(fd) ); }
        _mariadb_::cb task; process::await( task, obj->fd, res, cb, self );
    }

    void await( const string_t& cmd, const function_t<void,sql_item_t>& cb ) const {
        function_t<void,sql_item_t> cb = [=]( sql_item_t ){ };
        if( cmd.empty() || obj->state==0 ){ return; }

        if( mysql_real_query( obj->fd, cmd.data(), cmd.size() ) != 0 ){
            throw except_t( mysql_error( obj->fd ) ); return;
        }

        auto self = type::bind( this );
        MYSQL_RES *res = mysql_store_result( obj->fd );

        if( res == NULL ) { throw except_t( mysql_error(fd) ); }
        _mariadb_::cb task; process::add( task, obj->fd, res, cb, self );
    }

    /*─······································································─*/

    void use()     const noexcept { if( obj->used==1 ){ return; } obj->used=1; onUse    .emit(); }
    void release() const noexcept { if( obj->used==0 ){ return; } obj->used=0; onRelease.emit(); }
    bool is_used() const noexcept { return obj->used; }

    /*─······································································─*/

    void free() const noexcept {
        if( obj->fd == nullptr ){ return; }
        if( obj->state == 0 )   { return; }
        mysql_close( obj->fd );
        release(); obj->state = 0;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
