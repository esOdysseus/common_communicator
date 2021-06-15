/***
 * CAliasSearcher.h
 * Copyright [2019-] 
 * Written by EunSeok Kim <es.odysseus@gmail.com>
 * 
 * This file is part of the Common-Communicator framework.
 */
#ifndef _ALIAS_SEARCHER_INTERFACE_H_
#define _ALIAS_SEARCHER_INTERFACE_H_

#include <memory>
#include <string>

#include <IAliasPVD.h>


namespace alias {


    class IAliasSearcher {
    public:
        static std::shared_ptr<IAliasSearcher> get_searcher( const std::string& alias_file_path );

        virtual ~IAliasSearcher(void) = default;

        virtual std::shared_ptr<cf_alias::IAliasPVD> get_provider( const std::string& peer_app, const std::string& peer_pvd ) = 0;

    protected:
        IAliasSearcher(void) = default;

    private:
        IAliasSearcher( const IAliasSearcher& inst ) = delete;
        IAliasSearcher( IAliasSearcher&& inst ) = delete;

    };


}

#endif // _ALIAS_SEARCHER_INTERFACE_H_