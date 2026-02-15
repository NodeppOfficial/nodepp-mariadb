/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_MARIADB
#define NODEPP_MARIADB

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
#include <nodepp/expected.h>
#include <nodepp/optional.h>
#include <nodepp/promise.h>
#include <nodepp/url.h>

#include <mariadb/mysql.h>

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_MARIADB_GENERATOR
#define NODEPP_MARIADB_GENERATOR

namespace nodepp { namespace _mariadb_ { GENERATOR( cb ){
protected:

    array_t<string_t> col;
    int num_fields, x ;
    MYSQL_ROW     row ;
    MYSQL_FIELD *field;

public:

    template< class T, class U, class V, class Q > coEmit( T& fd, U& ctx, V& cb, Q& self ){
    coBegin ; coWait( self->is_used()==1 ); self->use();
        if( cb.null() ){ mysql_free_result(ctx); self->release(); coEnd; }

        num_fields = mysql_num_fields( ctx );

        for( x=0; x<num_fields; x++ ) {
             field = mysql_fetch_field_direct( ctx, x );
             col.push( field->name ); 
        }

        while( (row=mysql_fetch_row(ctx)) ){ do {
            auto object = map_t<string_t,string_t>();

            for( x=0; x<num_fields; x++ )
               { object[ col[x] ] = row[x] ? row[x] : "NULL"; }

        cb( object ); } while(0); coNext; } 
        
        mysql_free_result( ctx ); self->release();

    coFinish }

};}}

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_SQL_ITEM
#define NODEPP_SQL_ITEM
namespace nodepp { using sql_item_t = map_t<string_t,string_t>; }
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class mariadb_t {
protected:

    enum STATE {
        SQL_STATE_UNKNOWN = 0b00000000,
        SQL_STATE_OPEN    = 0b00000001,
        SQL_STATE_USED    = 0b10000000,
        SQL_STATE_CLOSE   = 0b00000010
    };

    struct NODE {
        MYSQL *fd   = nullptr;
        int   state = 0;
    };  ptr_t<NODE> obj;

public:

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

        if( mysql_real_connect( obj->fd, host.get(), user.get(), pass.get(), name.get(), port, NULL, 0 )==NULL )
          { throw except_t( mysql_error( obj->fd ) ); }

        obj->state = STATE::SQL_STATE_OPEN;

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

        if( mysql_real_connect( obj->fd, host.get(), user.get(), pass.get(), name.get(), port, NULL, 0 )==NULL )
          { throw except_t( mysql_error( obj->fd ) ); }

        obj->state = STATE::SQL_STATE_OPEN;

    }

    mariadb_t () : obj( new NODE ) { obj->state = STATE::SQL_STATE_CLOSE; }

   ~mariadb_t () noexcept { if( obj.count() > 1 ){ return; } free(); }

    /*─······································································─*/

    promise_t<array_t<sql_item_t>,except_t> resolve( const string_t& cmd ) const { 
           queue_t<sql_item_t> arr; auto self = type::bind( this );
    return promise_t<array_t<sql_item_t>,except_t>([=]( 
        res_t<array_t<sql_item_t>> res, 
        rej_t<except_t> /*------*/ rej 
    ){

        function_t<void,sql_item_t> cb ([=]( sql_item_t args ){ arr.push(args); });

        if( cmd.empty() || self->is_closed() || self->obj->fd==nullptr )
          { rej(except_t("MariaDB Error: Closed")); return; }

        if( mysql_real_query( self->obj->fd, cmd.data(), cmd.size() ) != 0 )
          { rej(except_t( mysql_error( self->obj->fd ) )); return; }

        MYSQL_RES *ctx = mysql_store_result( self->obj->fd );

        if( ctx == NULL ){ 
        if( mysql_field_count(self->obj->fd) == 0 ){
        /*----*/ res( nullptr ); /*------------------*/ return;
        } else { rej(except_t( mysql_error(obj->fd) )); return; }}

        auto task = type::bind( _mariadb_::cb() ); process::add([=](){
            while( (*task)( ctx, cb, self )>=0 ){ return 1; }
            res( arr.data() ); return -1; 
        }); 
    
    }); }

    /*─······································································─*/

    expected_t<array_t<sql_item_t>,except_t> 
    await( const string_t& cmd ) const { return resolve( cmd ).await(); }

    /*─······································································─*/

    optional_t<except_t> 
    emit( const string_t& cmd, function_t<void,sql_item_t> cb=nullptr ) const {
        if( cmd.empty() || is_closed() || self->obj->fd==nullptr )
          { return except_t("MariaDB Error: Closed"); }

        if( mysql_real_query( obj->fd, cmd.data(), cmd.size() ) != 0 )
          { return except_t( mysql_error( obj->fd ) ); }

        auto self = type::bind( this );
        MYSQL_RES *ctx = mysql_store_result( obj->fd );

        if( ctx == NULL ){ 
        if( mysql_field_count(self->obj->fd) == 0 ); }
        else { return except_t( mysql_error(obj->fd) ); }

        _mariadb_::cb task; 
        process::add( task, obj->fd, ctx, cb, self );

    return nullptr; }

    /*─······································································─*/

    bool is_closed()    const noexcept { return obj->state & STATE::SQL_STATE_CLOSE; }
    bool is_used()      const noexcept { return obj->state & STATE::SQL_STATE_USED ; }
    void close()        const noexcept { /*--*/ obj->state = STATE::SQL_STATE_CLOSE; }
    void use()          const noexcept { /*--*/ obj->state|= STATE::SQL_STATE_USED ; }
    void release()      const noexcept { /*--*/ obj->state&=~STATE::SQL_STATE_USED ; }
    bool is_available() const noexcept { return !is_closed(); }

    /*─······································································─*/

    void free() const noexcept {
        if( obj->fd == nullptr ){ return; }
        if( is_closed() ) /*-*/ { return; }
        close(); mysql_close( obj->fd );
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
